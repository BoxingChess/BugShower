// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/PoolingSubsystem.h"
#include "NPC/Spawnable.h"
#include "Projectile/MonsterProjectile.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/BugShowerLog.h"

void UPoolingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PoolSize = 100;
	SpawnRadius = 1500.f;
	SpawnInterval = 3.0f;
	SpawnTimer = 0.0f;
	bPoolsInitialized = false;

	LOG_LOGIC_INFO(TEXT("PoolingSubsystem initialized"));
}

void UPoolingSubsystem::Deinitialize()
{
	// Clean up pools
	PoolMap.Empty();
	AvailableMonsters.Empty();
	AvailableItems.Empty();
	AvailableBullets.Empty();
	

	LOG_LOGIC_INFO(TEXT("PoolingSubsystem deinitialized"));

	Super::Deinitialize();
}

void UPoolingSubsystem::InitializePools(
	TSubclassOf<AActor> InMonsterClass,
	TSubclassOf<AActor> InItemClass,
	TSubclassOf<AActor> InBulletClass,
	int32 InPoolSize)
{
	if (bPoolsInitialized)
	{
		LOG_LOGIC_WARNING(TEXT("Pools already initialized"));
		return;
	}

	MonsterClass = InMonsterClass;
	ItemClass = InItemClass;
	BulletClass = InBulletClass;
	PoolSize = InPoolSize;

	// Create pools for each type
	if (MonsterClass)
	{
		CreatePool(EPoolType::Monster, MonsterClass, PoolSize);
	}

	if (ItemClass)
	{
		CreatePool(EPoolType::Item, ItemClass, PoolSize);
	}

	if (BulletClass)
	{
		CreatePool(EPoolType::Bullet, BulletClass, PoolSize);
	}

	bPoolsInitialized = true;
	LOG_LOGIC_INFO(TEXT("All pools initialized with size: %d"), PoolSize);
}

void UPoolingSubsystem::CreatePool(EPoolType Type, TSubclassOf<AActor> ActorClass, int32 Size)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_LOGIC_ERROR(TEXT("CreatePool: World is null"));
		return;
	}

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

			// Initialize without OwningPool parameter
			Spawnable->Deactivate(SpawnedActor);

			// Add to pool map
			if (!PoolMap.Contains(Type))
			{
				PoolMap.Add(Type, {Spawnable});
			}
			else
			{
				PoolMap[Type].Add(Spawnable);
			}

			// Add to available queue
			GetQueueForType(Type).Enqueue(Spawnable);

			LOG_LOGIC_INFO(TEXT("Created %s object %d"), *UEnum::GetValueAsString(Type), i);
		}
		else
		{
			LOG_LOGIC_WARNING(TEXT("Failed to create %s object %d"), *UEnum::GetValueAsString(Type), i);
		}
	}
}

TQueue<TScriptInterface<ISpawnable>>& UPoolingSubsystem::GetQueueForType(EPoolType Type)
{
	switch (Type)
	{
	case EPoolType::Monster:
		return AvailableMonsters;
	case EPoolType::Item:
		return AvailableItems;
	case EPoolType::Bullet:
		return AvailableBullets;
	default:
		return AvailableMonsters; // Fallback
	}
}

TScriptInterface<ISpawnable> UPoolingSubsystem::FindAvailableObject(EPoolType Type)
{
	TScriptInterface<ISpawnable> Result;

	switch (Type)
	{
	case EPoolType::Monster:
		if (AvailableMonsters.Dequeue(Result))
		{
			return Result;
		}
		break;

	case EPoolType::Item:
		if (AvailableItems.Dequeue(Result))
		{
			return Result;
		}
		break;

	case EPoolType::Bullet:
		if (AvailableBullets.Dequeue(Result))
		{
			return Result;
		}
		break;

	default:
		break;
	}

	LOG_LOGIC_WARNING(TEXT("No available %s object in pool"), *UEnum::GetValueAsString(Type));
	return TScriptInterface<ISpawnable>();
}

TScriptInterface<ISpawnable> UPoolingSubsystem::SpawnFromPool(EPoolType Type, const FVector& Position)
{
	if (!bPoolsInitialized)
	{
		LOG_LOGIC_ERROR(TEXT("SpawnFromPool: Pools not initialized"));
		return TScriptInterface<ISpawnable>();
	}

	TScriptInterface<ISpawnable> Spawnable = FindAvailableObject(Type);

	if (Spawnable)
	{
		Spawnable->Spawn(Position);
		LOG_LOGIC_INFO(TEXT("Spawned %s at location: %s"), *UEnum::GetValueAsString(Type), *Position.ToString());
		return Spawnable;
	}

	LOG_LOGIC_ERROR(TEXT("Failed to spawn %s - no available objects"), *UEnum::GetValueAsString(Type));
	return TScriptInterface<ISpawnable>();
}

void UPoolingSubsystem::ReturnToPool(TScriptInterface<ISpawnable> Object)
{
	if (!Object)
	{
		LOG_LOGIC_WARNING(TEXT("ReturnToPool: Object is null"));
		return;
	}

	EPoolType Type = Object->GetPoolType();

	switch (Type)
	{
	case EPoolType::Monster:
		AvailableMonsters.Enqueue(Object);
		break;
	case EPoolType::Item:
		AvailableItems.Enqueue(Object);
		break;
	case EPoolType::Bullet:
		AvailableBullets.Enqueue(Object);
		break;
	default:
		LOG_LOGIC_WARNING(TEXT("ReturnToPool: Unknown pool type"));
		break;
	}

	LOG_LOGIC_INFO(TEXT("Returned %s to pool"), *UEnum::GetValueAsString(Type));
}

bool UPoolingSubsystem::FireProjectileAt(
	AActor* Shooter,
	AActor* Target,
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
	TScriptInterface<ISpawnable> SpawnedObj = SpawnFromPool(EPoolType::Bullet, SpawnLocation);
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

void UPoolingSubsystem::SpawnItemDrop(const FVector& Location, int32 Count)
{
	for (int32 i = 0; i < Count; i++)
	{
		// Add random offset to avoid items stacking
		FVector Offset(
			FMath::RandRange(-100.f, 100.f),
			FMath::RandRange(-100.f, 100.f),
			0.f
		);

		FVector SpawnLocation = Location + Offset;
		SpawnLocation.Z += 50.f; // Slightly above ground

		TScriptInterface<ISpawnable> SpawnedItem = SpawnFromPool(EPoolType::Item, SpawnLocation);
		if (SpawnedItem)
		{
			LOG_LOGIC_INFO(TEXT("Spawned item %d/%d at %s"), i + 1, Count, *SpawnLocation.ToString());
		}
	}
}

void UPoolingSubsystem::SpawnMonsters(const FVector& SpawnLocation, float InSpawnRadius)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		LOG_LOGIC_WARNING(TEXT("SpawnMonsters: NavSystem is null"));
		return;
	}

	FNavLocation RandomLocation;
	if (NavSys->GetRandomPointInNavigableRadius(SpawnLocation, InSpawnRadius, RandomLocation))
	{
		SpawnFromPool(EPoolType::Monster, RandomLocation.Location);
	}
}
