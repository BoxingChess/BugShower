// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/PoolingSubsystem.h"
#include "NPC/Spawnable.h"
#include "Projectile/MonsterProjectile.h"
#include "Item/ItemActor.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/BugShowerLog.h"
#include "Engine/DataTable.h"
#include "PoolingSetting.h"

void UPoolingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bPoolsInitialized = false;

	LOG_POOLING_INFO(TEXT("Start PoolingSubsystem initialized"));

	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		LOG_POOLING_INFO(TEXT("World is Null Or Client don't need to initialize"));
		return;
	}

	// Auto-initialize pools if enabled
	if (bAutoInitializePools)
	{
		const UPoolingSettings* Settings = GetDefault<UPoolingSettings>();

		// Settings에서 테이블 로드 시도
		bool bLoadedFromSettings = false;

		if (!Settings->PoolConfigTable.IsNull() && !Settings->MonsterDropTable.IsNull())
		{
			UDataTable* PoolTable = Settings->PoolConfigTable.LoadSynchronous();
			if (PoolTable)
			{
				UE_LOG(LogTemp, Log, TEXT("[PoolingSubsystem] Loaded PoolConfigTable from Settings: %s"), *PoolTable->GetName());
				InitializePoolsFromTable(PoolTable);
			}

			UDataTable* DropTable = Settings->MonsterDropTable.LoadSynchronous();
			if (DropTable)
			{
				UE_LOG(LogTemp, Log, TEXT("[PoolingSubsystem] Loaded MonsterDropTable from Settings: %s"), *DropTable->GetName());
				SetDropConfigTable(DropTable);
			}

			bLoadedFromSettings = true;
		}


#if WITH_EDITOR
		// Settings에서 로드 실패 시 기본 경로에서 로드
		if (!bLoadedFromSettings)
		{
			UDataTable* PoolTable = LoadObject<UDataTable>(
				nullptr,
				TEXT("/Game/Data/DT_PoolConfig.DT_PoolConfig")
			);

			if (PoolTable)
			{
				UE_LOG(LogTemp, Log, TEXT("[PoolingSubsystem] Loaded PoolConfigTable from default path"));
				InitializePoolsFromTable(PoolTable);
			}

			UDataTable* DropTable = LoadObject<UDataTable>(
				nullptr,
				TEXT("/Game/Data/DT_MonsterDrops.DT_MonsterDrops")
			);

			if (DropTable)
			{
				SetDropConfigTable(DropTable);
				UE_LOG(LogTemp, Log, TEXT("[PoolingSubsystem] Loaded MonsterDropTable from default path"));
			}
		}
#endif
	}
}

void UPoolingSubsystem::Deinitialize()
{
	// Clean up class-based pools
	ClassPools.Empty();
	LOG_POOLING_INFO(
		TEXT("PoolingSubsystem %p | World=%s"),
		this,
		*GetWorld()->GetName()
	);
	LOG_POOLING_INFO(TEXT("PoolingSubsystem deinitialized"));

	Super::Deinitialize();
}

bool UPoolingSubsystem::FireProjectileAt(
	AActor* Shooter,
	AActor* Target,
	TSubclassOf<AActor> ActorClass,
	const FVector& SpawnLocation,
	float ProjectileSpeed,
	float Damage)
{
	if (!Shooter || !Target)
	{
		LOG_POOLING_WARNING(TEXT("FireProjectileAt: Shooter or Target is null"));
		return false;
	}

	// Calculate arc trajectory
	FVector TargetLocation = Target->GetActorLocation();
	FVector LaunchVelocity;

	bool bHaveArc = UGameplayStatics::SuggestProjectileVelocity(
		Shooter,
		LaunchVelocity,
		SpawnLocation,
		TargetLocation,
		ProjectileSpeed,
		true,  // High arc
		0.0f,
		0.0f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if (!bHaveArc)
	{
		LOG_POOLING_WARNING(TEXT("FireProjectileAt: Could not calculate arc trajectory"));
		return false;
	}

	// Get projectile from pool
	TScriptInterface<ISpawnable> SpawnedObj = SpawnFromClass(ActorClass, SpawnLocation);
	if (!SpawnedObj)
	{
		LOG_POOLING_ERROR(TEXT("FireProjectileAt: Failed to get projectile from pool"));
		return false;
	}

	// Initialize projectile
	AMonsterProjectile* Projectile = Cast<AMonsterProjectile>(SpawnedObj.GetObject());
	if (Projectile)
	{
		Projectile->InitializeProjectileWithVelocity(LaunchVelocity, Damage, Shooter);
		LOG_POOLING_INFO(TEXT("Fired projectile from %s at %s"), *Shooter->GetName(), *Target->GetName());
		return true;
	}

	LOG_POOLING_ERROR(TEXT("FireProjectileAt: Failed to cast to MonsterProjectile"));
	return false;
}


bool UPoolingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		return false;
	}

	// InGame 맵에서만 생성
	FString MapName = World->GetName();
	if (!MapName.Contains(TEXT("InGame")))
	{
		return false;
	}

	// 디버그: World 정보 출력
	UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] ShouldCreateSubsystem - MapName: %s, WorldType: %d, NetMode: %d"),
		*MapName, (int32)World->WorldType, (int32)World->GetNetMode());

	// Game/PIE World에서만 생성 (Editor preview 등 제외)
	if (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE)
	{
		return false;
	}

	// 클라이언트에서는 생성하지 않음
	if (World->GetNetMode() == NM_Client)
	{
		return false;
	}

	return true;
}

// ========== Class-Based Pooling Implementation ==========

void UPoolingSubsystem::RegisterPoolForClass(TSubclassOf<AActor> ActorClass, int32 Size)
{
	if (!ActorClass)
	{
		LOG_POOLING_ERROR(TEXT("RegisterPoolForClass: ActorClass is null"));
		return;
	}

	if (ClassPools.Contains(ActorClass))
	{
		LOG_POOLING_WARNING(TEXT("RegisterPoolForClass: Pool for class %s already exists"), *ActorClass->GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_POOLING_ERROR(TEXT("RegisterPoolForClass: World is null"));
		return;
	}

	// DEBUG: Log pool registration start
	UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] ?�� RegisterPoolForClass: Creating pool for %s with size %d"), *ActorClass->GetName(), Size);

	// Create new pool data
	FPoolData NewPool;



	// Spawn and initialize pool objects
	for (int32 i = 0; i < Size; i++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (SpawnedActor && SpawnedActor->Implements<USpawnable>())
		{
			// Properly initialize TScriptInterface
			TScriptInterface<ISpawnable> Spawnable;
			Spawnable.SetObject(SpawnedActor);

			// Get interface pointer using GetInterfaceAddress
			// This is the correct way to get interface pointer for TScriptInterface
			ISpawnable* InterfacePtr = Cast<ISpawnable>(SpawnedActor);
			if (!InterfacePtr)
			{
				LOG_POOLING_ERROR(TEXT("Failed to get ISpawnable interface for %s"), *ActorClass->GetName());
				SpawnedActor->Destroy();
				continue;
			}

			Spawnable.SetInterface(InterfacePtr);

			// Add to pool first
			NewPool.All.Add(Spawnable);
			NewPool.Available.Add(Spawnable);

			InterfacePtr->Deactivate(SpawnedActor);
			

			// DEBUG: Log actor pointer added (only for first 10 items to avoid spam)
			if (i < 10)
			{
				UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] ?�� Added Actor %p to pool (index %d)"), SpawnedActor, i);
			}

			LOG_POOLING_INFO(TEXT("Created %s object %d/%d"), *ActorClass->GetName(), i + 1, Size);
		}
		else
		{
			LOG_POOLING_WARNING(TEXT("Failed to create %s object %d - does not implement ISpawnable"), *ActorClass->GetName(), i);
			if (SpawnedActor)
			{
				SpawnedActor->Destroy();
			}
		}
	}

	// DEBUG: Log final pool state BEFORE MoveTemp
	UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] ?�� Pool registered: All.Num()=%d, Available.Num()=%d"), NewPool.All.Num(), NewPool.Available.Num());

	// Store the pool
	ClassPools.Add(ActorClass, MoveTemp(NewPool));

	LOG_POOLING_INFO(TEXT("Registered pool for class %s with %d objects"), *ActorClass->GetName(), Size);
}

TScriptInterface<ISpawnable> UPoolingSubsystem::SpawnFromClass(
	TSubclassOf<AActor> ActorClass,
	const FVector& Position)
{
	if (!ActorClass)
	{
		LOG_POOLING_ERROR(TEXT("SpawnFromClass: ActorClass is null"));
		return TScriptInterface<ISpawnable>();
	}

	FPoolData* PoolData = ClassPools.Find(ActorClass);
	if (!PoolData)
	{
		LOG_POOLING_ERROR(TEXT("SpawnFromClass: No pool registered for class %s"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}


	if (PoolData->Available.Num() == 0)
	{
		LOG_POOLING_WARNING(TEXT("SpawnFromClass: Pool for class %s is exhausted"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}

	TScriptInterface<ISpawnable> Spawnable = PoolData->Available.Pop();

	// DEBUG: Log Available array size after Pop and actor pointer
	AActor* DebugActor = Cast<AActor>(Spawnable.GetObject());

	// Validate the spawnable object
	AActor* SpawnedActor = Cast<AActor>(Spawnable.GetObject());
	if (!SpawnedActor)
	{
		LOG_POOLING_ERROR(TEXT("SpawnFromClass: Failed to cast to AActor for class %s"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}

	// Get interface and call Spawn
	ISpawnable* SpawnableInterface = Spawnable.GetInterface();
	if (!SpawnableInterface)
	{
		LOG_POOLING_ERROR(TEXT("SpawnFromClass: Failed to get interface for class %s"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}

	SpawnableInterface->Spawn(Position);

	LOG_POOLING_INFO(TEXT("Spawned %s at location: %s"), *ActorClass->GetName(), *Position.ToString());
	return Spawnable;
}

void UPoolingSubsystem::ReturnToPoolByClass(TScriptInterface<ISpawnable> Object)
{
	if (!Object)
	{
		LOG_POOLING_WARNING(TEXT("ReturnToPoolByClass: Object is null"));
		return;
	}

	AActor* Actor = Cast<AActor>(Object.GetObject());
	if (!Actor)
	{
		LOG_POOLING_WARNING(TEXT("ReturnToPoolByClass: Cannot cast object to AActor"));
		return;
	}

	// DEBUG: Log actor being returned

	// UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] ReturnToPoolByClass called for Actor: %p"), Actor);


	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	FPoolData* PoolData = ClassPools.Find(ActorClass);

	if (!PoolData)
	{
		LOG_POOLING_WARNING(TEXT("ReturnToPoolByClass: No pool found for class %s"), *ActorClass->GetName());
		return;
	}

	// DEBUG: Log Available size before Add

	// UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] Available.Num() BEFORE Add: %d"), PoolData->Available.Num());

	// Deactivate before returning
	Object->Deactivate(Actor);

	// Return to available queue
	PoolData->Available.Add(Object);

	// DEBUG: Log Available size after Add
	UE_LOG(LogTemp, Warning, TEXT("[PoolingSubsystem] ?�� Available.Num() AFTER Add: %d"), PoolData->Available.Num());

	LOG_POOLING_INFO(TEXT("Returned %s to class pool"), *ActorClass->GetName());
}

void UPoolingSubsystem::ReturnAllOfClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		LOG_POOLING_WARNING(TEXT("ReturnAllOfClass: ActorClass is null"));
		return;
	}

	FPoolData* PoolData = ClassPools.Find(ActorClass);
	if (!PoolData)
	{
		LOG_POOLING_WARNING(TEXT("ReturnAllOfClass: No pool exists for class %s"), *ActorClass->GetName());
		return;
	}

	int32 ReturnedCount = 0;
	for (TScriptInterface<ISpawnable>& Spawnable : PoolData->All)
	{
		if (Spawnable)
		{
			Spawnable->ReturnPool();
			ReturnedCount++;
		}
	}

	LOG_POOLING_INFO(TEXT("Returned %d objects of class %s to pool"), ReturnedCount, *ActorClass->GetName());
}

// ========== Pool Initialization Implementation ==========

void UPoolingSubsystem::InitializePoolsFromConfig()
{
	if (bPoolsInitialized)
	{
		LOG_POOLING_WARNING(TEXT("InitializePoolsFromConfig: Pools already initialized"));
		return;
	}

	LOG_POOLING_INFO(TEXT("InitializePoolsFromConfig: Starting pool initialization from config array"));

	int32 TotalPools = 0;
	for (const FPoolConfig& Config : PoolConfigs)
	{
		if (!Config.ActorClass)
		{
			LOG_POOLING_WARNING(TEXT("InitializePoolsFromConfig: Skipping null ActorClass"));
			continue;
		}

		RegisterPoolForClass(Config.ActorClass, Config.PoolSize);
		TotalPools++;
	}

	bPoolsInitialized = true;
	LOG_POOLING_INFO(TEXT("InitializePoolsFromConfig: Initialized %d pools"), TotalPools);
}

void UPoolingSubsystem::InitializePoolsFromTable(UDataTable* PoolConfigTableParam)
{
	if (bPoolsInitialized)
	{
		LOG_POOLING_WARNING(TEXT("InitializePoolsFromTable: Pools already initialized"));
		return;
	}

	if (!PoolConfigTableParam)
	{
		LOG_POOLING_ERROR(TEXT("InitializePoolsFromTable: PoolConfigTable is null"));
		return;
	}

	LOG_POOLING_INFO(TEXT("InitializePoolsFromTable: Starting pool initialization from DataTable"));

	TArray<FPoolConfigTableRow*> AllRows;
	PoolConfigTableParam->GetAllRows<FPoolConfigTableRow>(TEXT("InitializePoolsFromTable"), AllRows);

	int32 TotalPools = 0;
	for (FPoolConfigTableRow* Row : AllRows)
	{
		if (!Row || !Row->ActorClass)
		{
			LOG_POOLING_WARNING(TEXT("InitializePoolsFromTable: Skipping invalid row"));
			continue;
		}

		RegisterPoolForClass(Row->ActorClass, Row->PoolSize);
		TotalPools++;
	}

	bPoolsInitialized = true;
	LOG_POOLING_INFO(TEXT("InitializePoolsFromTable: Initialized %d pools from DataTable"), TotalPools);
}

// ========== Drop Management Implementation ==========

void UPoolingSubsystem::SetDropConfigTable(UDataTable* DropTable)
{
	if (!DropTable)
	{
		LOG_POOLING_WARNING(TEXT("SetDropConfigTable: DropTable is null"));
		return;
	}

	MonsterDropTable = DropTable;
	LOG_POOLING_INFO(TEXT("Drop configuration table set: %s"), *DropTable->GetName());
}

void UPoolingSubsystem::ProcessMonsterDrop(
	FName MonsterDropID,
	const FVector& DropLocation,
	const FGameplayTagContainer& ActiveConditions)
{
	if (!MonsterDropTable)
	{
		LOG_POOLING_WARNING(TEXT("ProcessMonsterDrop: MonsterDropTable is not set. Call SetDropConfigTable first."));
		return;
	}

	// Find drop configuration in DataTable
	FMonsterDropConfig* DropConfig = MonsterDropTable->FindRow<FMonsterDropConfig>(MonsterDropID, TEXT("ProcessMonsterDrop"));
	if (!DropConfig)
	{
		LOG_POOLING_WARNING(TEXT("ProcessMonsterDrop: No drop config found for ID '%s'"), *MonsterDropID.ToString());
		return;
	}

	// Calculate drops from configuration
	TArray<TSubclassOf<AItemActor>> ItemsToDrop = CalculateDropsFromConfig(*DropConfig, ActiveConditions);

	if (ItemsToDrop.Num() == 0)
	{
		LOG_POOLING_INFO(TEXT("ProcessMonsterDrop: No items to drop for '%s'"), *MonsterDropID.ToString());
		return;
	}

	// Spawn the items
	SpawnDroppedItems(ItemsToDrop, DropLocation, DropConfig->DropSpreadRadius);

	LOG_POOLING_INFO(TEXT("ProcessMonsterDrop: Dropped %d items for '%s' at %s"),
		ItemsToDrop.Num(), *MonsterDropID.ToString(), *DropLocation.ToString());
}

TArray<TSubclassOf<AItemActor>> UPoolingSubsystem::CalculateDropsFromConfig(
	const FMonsterDropConfig& Config,
	const FGameplayTagContainer& ActiveConditions) const
{
	TArray<TSubclassOf<AItemActor>> Result;

	// Layer 1: Guaranteed drops (always drop, ignore drop chance)
	for (const FItemDropEntry& Entry : Config.GuaranteedDrops)
	{
		// Check conditions
		if (!Entry.RequiredTags.IsEmpty() && !ActiveConditions.HasAll(Entry.RequiredTags))
			continue;

		if (Entry.ItemClass)
		{
			int32 DropCount = FMath::RandRange(Entry.MinCount, Entry.MaxCount);
			for (int32 i = 0; i < DropCount; i++)
			{
				Result.Add(Entry.ItemClass);
			}
		}
	}

	// Check base drop chance
	float RollValue = FMath::FRand();
	if (RollValue > Config.BaseDropChance)
	{
		// Failed drop chance, only return guaranteed drops
		return Result;
	}

	// Layer 2: Common drops
	int32 CommonDropCount = FMath::RandRange(Config.MinTotalDropCount, Config.MaxTotalDropCount);
	TArray<TSubclassOf<AItemActor>> CommonDrops = SelectDropsByWeight(
		Config.CommonDrops,
		CommonDropCount,
		ActiveConditions
	);
	Result.Append(CommonDrops);

	// Layer 3: Unique/Rare drops (usually 0-1 item)
	TArray<TSubclassOf<AItemActor>> UniqueDrops = SelectDropsByWeight(
		Config.UniqueDrops,
		1, // Usually only 1 unique drop
		ActiveConditions
	);
	Result.Append(UniqueDrops);

	return Result;
}

TArray<TSubclassOf<AItemActor>> UPoolingSubsystem::SelectDropsByWeight(
	const TArray<FItemDropEntry>& Entries,
	int32 MaxSelections,
	const FGameplayTagContainer& ActiveConditions) const
{
	TArray<TSubclassOf<AItemActor>> Result;

	if (Entries.Num() == 0 || MaxSelections <= 0)
		return Result;

	// Build weighted list
	TArray<FItemDropEntry> ValidEntries;
	float TotalWeight = 0.0f;

	for (const FItemDropEntry& Entry : Entries)
	{
		// Check required tags
		if (!Entry.RequiredTags.IsEmpty() && !ActiveConditions.HasAll(Entry.RequiredTags))
			continue;

		if (Entry.ItemClass && Entry.DropWeight > 0.0f)
		{
			ValidEntries.Add(Entry);
			TotalWeight += Entry.DropWeight;
		}
	}

	if (ValidEntries.Num() == 0 || TotalWeight <= 0.0f)
		return Result;

	// Select items by weight
	for (int32 i = 0; i < MaxSelections; i++)
	{
		float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
		float CurrentWeight = 0.0f;

		for (const FItemDropEntry& Entry : ValidEntries)
		{
			CurrentWeight += Entry.DropWeight;
			if (RandomValue <= CurrentWeight)
			{
				// Selected this entry
				int32 DropCount = FMath::RandRange(Entry.MinCount, Entry.MaxCount);
				for (int32 j = 0; j < DropCount; j++)
				{
					Result.Add(Entry.ItemClass);
				}
				break;
			}
		}
	}

	return Result;
}

void UPoolingSubsystem::SpawnDroppedItems(
	const TArray<TSubclassOf<AItemActor>>& ItemClasses,
	const FVector& CenterLocation,
	float SpreadRadius)
{
	if (ItemClasses.Num() == 0)
		return;

	for (int32 i = 0; i < ItemClasses.Num(); i++)
	{
		TSubclassOf<AItemActor> ItemClass = ItemClasses[i];
		if (!ItemClass)
			continue;

		// Calculate spread position
		FVector SpawnLocation = CenterLocation;
		if (SpreadRadius > 0.0f)
		{
			// Random position in circle
			float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
			float Distance = FMath::FRandRange(0.0f, SpreadRadius);
			SpawnLocation.X += FMath::Cos(Angle) * Distance;
			SpawnLocation.Y += FMath::Sin(Angle) * Distance;
		}

		// Spawn from pool
		TScriptInterface<ISpawnable> SpawnedItem = SpawnFromClass(ItemClass, SpawnLocation);
		if (SpawnedItem)
		{
			LOG_POOLING_INFO(TEXT("Spawned drop item %s at %s"), *ItemClass->GetName(), *SpawnLocation.ToString());
		}
		else
		{
			LOG_POOLING_WARNING(TEXT("Failed to spawn drop item %s - pool may be empty"), *ItemClass->GetName());
		}
	}
}
