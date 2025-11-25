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
