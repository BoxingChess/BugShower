// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/PoolingSubsystem.h"
#include "NPC/Spawnable.h"
#include "Projectile/MonsterProjectile.h"
#include "Item/ItemBase.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/BugShowerLog.h"
#include "Engine/DataTable.h"

void UPoolingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bPoolsInitialized = false;

	LOG_LOGIC_INFO(TEXT("PoolingSubsystem initialized"));
}

void UPoolingSubsystem::Deinitialize()
{
	// Clean up class-based pools
	ClassPools.Empty();

	LOG_LOGIC_INFO(TEXT("PoolingSubsystem deinitialized"));

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
		LOG_LOGIC_WARNING(TEXT("FireProjectileAt: Shooter or Target is null"));
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
		LOG_LOGIC_WARNING(TEXT("FireProjectileAt: Could not calculate arc trajectory"));
		return false;
	}

	// Get projectile from pool
	TScriptInterface<ISpawnable> SpawnedObj = SpawnFromClass(ActorClass, SpawnLocation);
	if (!SpawnedObj)
	{
		LOG_LOGIC_ERROR(TEXT("FireProjectileAt: Failed to get projectile from pool"));
		return false;
	}

	// Initialize projectile
	AMonsterProjectile* Projectile = Cast<AMonsterProjectile>(SpawnedObj.GetObject());
	if (Projectile)
	{
		Projectile->InitializeProjectileWithVelocity(LaunchVelocity, Damage, Shooter);
		LOG_LOGIC_INFO(TEXT("Fired projectile from %s at %s"), *Shooter->GetName(), *Target->GetName());
		return true;
	}

	LOG_LOGIC_ERROR(TEXT("FireProjectileAt: Failed to cast to MonsterProjectile"));
	return false;
}


// ========== Class-Based Pooling Implementation ==========

void UPoolingSubsystem::RegisterPoolForClass(TSubclassOf<AActor> ActorClass, int32 Size)
{
	if (!ActorClass)
	{
		LOG_LOGIC_ERROR(TEXT("RegisterPoolForClass: ActorClass is null"));
		return;
	}

	if (ClassPools.Contains(ActorClass))
	{
		LOG_LOGIC_WARNING(TEXT("RegisterPoolForClass: Pool for class %s already exists"), *ActorClass->GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_LOGIC_ERROR(TEXT("RegisterPoolForClass: World is null"));
		return;
	}

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
			TScriptInterface<ISpawnable> Spawnable;
			Spawnable.SetObject(SpawnedActor);
			Spawnable.SetInterface(Cast<ISpawnable>(SpawnedActor));

			// Deactivate the object
			Spawnable->Deactivate(SpawnedActor);

			// Add to pool
			NewPool.All.Add(Spawnable);
			NewPool.Available.Add(Spawnable);

			LOG_LOGIC_INFO(TEXT("Created %s object %d/%d"), *ActorClass->GetName(), i + 1, Size);
		}
		else
		{
			LOG_LOGIC_WARNING(TEXT("Failed to create %s object %d - does not implement ISpawnable"), *ActorClass->GetName(), i);
		}
	}

	// Store the pool
	ClassPools.Add(ActorClass, MoveTemp(NewPool));

	LOG_LOGIC_INFO(TEXT("Registered pool for class %s with %d objects"), *ActorClass->GetName(), Size);
}

TScriptInterface<ISpawnable> UPoolingSubsystem::SpawnFromClass(
	TSubclassOf<AActor> ActorClass,
	const FVector& Position)
{
	if (!ActorClass)
	{
		LOG_LOGIC_ERROR(TEXT("SpawnFromClass: ActorClass is null"));
		return TScriptInterface<ISpawnable>();
	}

	FPoolData* PoolData = ClassPools.Find(ActorClass);
	if (!PoolData)
	{
		LOG_LOGIC_ERROR(TEXT("SpawnFromClass: No pool registered for class %s"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}


	if (PoolData->Available.Num() == 0)
	{
		LOG_LOGIC_WARNING(TEXT("SpawnFromClass: Pool for class %s is exhausted"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}

	TScriptInterface<ISpawnable> Spawnable = PoolData->Available.Pop();
	if (!Spawnable)
	{
		LOG_LOGIC_WARNING(TEXT("SpawnFromClass: No available objects in pool for class %s"), *ActorClass->GetName());
		return TScriptInterface<ISpawnable>();
	}

	if (Spawnable)
	{
		Spawnable->Spawn(Position);
		LOG_LOGIC_INFO(TEXT("Spawned %s at location: %s"), *ActorClass->GetName(), *Position.ToString());
		return Spawnable;
	}

	LOG_LOGIC_ERROR(TEXT("SpawnFromClass: Dequeued object is null for class %s"), *ActorClass->GetName());
	return TScriptInterface<ISpawnable>();
}

void UPoolingSubsystem::ReturnToPoolByClass(TScriptInterface<ISpawnable> Object)
{
	if (!Object)
	{
		LOG_LOGIC_WARNING(TEXT("ReturnToPoolByClass: Object is null"));
		return;
	}

	AActor* Actor = Cast<AActor>(Object.GetObject());
	if (!Actor)
	{
		LOG_LOGIC_WARNING(TEXT("ReturnToPoolByClass: Cannot cast object to AActor"));
		return;
	}

	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	FPoolData* PoolData = ClassPools.Find(ActorClass);

	if (!PoolData)
	{
		LOG_LOGIC_WARNING(TEXT("ReturnToPoolByClass: No pool found for class %s"), *ActorClass->GetName());
		return;
	}

	// Deactivate before returning
	Object->Deactivate(Actor);

	// Return to available queue
	PoolData->Available.Add(Object);
	LOG_LOGIC_INFO(TEXT("Returned %s to class pool"), *ActorClass->GetName());
}

void UPoolingSubsystem::ReturnAllOfClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		LOG_LOGIC_WARNING(TEXT("ReturnAllOfClass: ActorClass is null"));
		return;
	}

	FPoolData* PoolData = ClassPools.Find(ActorClass);
	if (!PoolData)
	{
		LOG_LOGIC_WARNING(TEXT("ReturnAllOfClass: No pool exists for class %s"), *ActorClass->GetName());
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

	LOG_LOGIC_INFO(TEXT("Returned %d objects of class %s to pool"), ReturnedCount, *ActorClass->GetName());
}

// ========== Drop Management Implementation ==========

void UPoolingSubsystem::SetDropConfigTable(UDataTable* DropTable)
{
	if (!DropTable)
	{
		LOG_LOGIC_WARNING(TEXT("SetDropConfigTable: DropTable is null"));
		return;
	}

	MonsterDropTable = DropTable;
	LOG_LOGIC_INFO(TEXT("Drop configuration table set: %s"), *DropTable->GetName());
}

void UPoolingSubsystem::ProcessMonsterDrop(
	FName MonsterDropID,
	const FVector& DropLocation,
	const FGameplayTagContainer& ActiveConditions)
{
	if (!MonsterDropTable)
	{
		LOG_LOGIC_WARNING(TEXT("ProcessMonsterDrop: MonsterDropTable is not set. Call SetDropConfigTable first."));
		return;
	}

	// Find drop configuration in DataTable
	FMonsterDropConfig* DropConfig = MonsterDropTable->FindRow<FMonsterDropConfig>(MonsterDropID, TEXT("ProcessMonsterDrop"));
	if (!DropConfig)
	{
		LOG_LOGIC_WARNING(TEXT("ProcessMonsterDrop: No drop config found for ID '%s'"), *MonsterDropID.ToString());
		return;
	}

	// Calculate drops from configuration
	TArray<TSubclassOf<AItemBase>> ItemsToDrop = CalculateDropsFromConfig(*DropConfig, ActiveConditions);

	if (ItemsToDrop.Num() == 0)
	{
		LOG_LOGIC_INFO(TEXT("ProcessMonsterDrop: No items to drop for '%s'"), *MonsterDropID.ToString());
		return;
	}

	// Spawn the items
	SpawnDroppedItems(ItemsToDrop, DropLocation, DropConfig->DropSpreadRadius);

	LOG_LOGIC_INFO(TEXT("ProcessMonsterDrop: Dropped %d items for '%s' at %s"),
		ItemsToDrop.Num(), *MonsterDropID.ToString(), *DropLocation.ToString());
}

TArray<TSubclassOf<AItemBase>> UPoolingSubsystem::CalculateDropsFromConfig(
	const FMonsterDropConfig& Config,
	const FGameplayTagContainer& ActiveConditions) const
{
	TArray<TSubclassOf<AItemBase>> Result;

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
	TArray<TSubclassOf<AItemBase>> CommonDrops = SelectDropsByWeight(
		Config.CommonDrops,
		CommonDropCount,
		ActiveConditions
	);
	Result.Append(CommonDrops);

	// Layer 3: Unique/Rare drops (usually 0-1 item)
	TArray<TSubclassOf<AItemBase>> UniqueDrops = SelectDropsByWeight(
		Config.UniqueDrops,
		1, // Usually only 1 unique drop
		ActiveConditions
	);
	Result.Append(UniqueDrops);

	return Result;
}

TArray<TSubclassOf<AItemBase>> UPoolingSubsystem::SelectDropsByWeight(
	const TArray<FItemDropEntry>& Entries,
	int32 MaxSelections,
	const FGameplayTagContainer& ActiveConditions) const
{
	TArray<TSubclassOf<AItemBase>> Result;

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
	const TArray<TSubclassOf<AItemBase>>& ItemClasses,
	const FVector& CenterLocation,
	float SpreadRadius)
{
	if (ItemClasses.Num() == 0)
		return;

	for (int32 i = 0; i < ItemClasses.Num(); i++)
	{
		TSubclassOf<AItemBase> ItemClass = ItemClasses[i];
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
			LOG_LOGIC_INFO(TEXT("Spawned drop item %s at %s"), *ItemClass->GetName(), *SpawnLocation.ToString());
		}
		else
		{
			LOG_LOGIC_WARNING(TEXT("Failed to spawn drop item %s - pool may be empty"), *ItemClass->GetName());
		}
	}
}
