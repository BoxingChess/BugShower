// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NPC/PoolingType.h"
#include "PoolingSubsystem.generated.h"

/**
 * World Subsystem for managing object pooling
 * Handles spawning and recycling of Monsters, Items, and Projectiles
 */
UCLASS()
class BUGSHOWER_API UPoolingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// High-level API for spawning pooled objects
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	bool FireProjectileAt(
		AActor* Shooter,
		AActor* Target,
		const FVector& SpawnLocation,
		float ProjectileSpeed,
		float Damage
	);

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void SpawnItemDrop(const FVector& Location, int32 Count = 1);

	// Low-level pool management
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	TScriptInterface<class ISpawnable> SpawnFromPool(EPoolType Type, const FVector& Position);

	void ReturnToPool(TScriptInterface<class ISpawnable> Object);

	// Pool initialization (called from GameMode or Level Blueprint)
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void InitializePools(
		TSubclassOf<AActor> InMonsterClass,
		TSubclassOf<AActor> InItemClass,
		TSubclassOf<AActor> InBulletClass,
		int32 InPoolSize = 100
	);

	// Internal spawn method for specific type
	void SpawnMonsters(const FVector& SpawnLocation, float SpawnRadius);
protected:

private:
	// Pool storage
	TMap<EPoolType, TArray<TScriptInterface<class ISpawnable>>> PoolMap;
	TQueue<TScriptInterface<class ISpawnable>> AvailableMonsters;
	TQueue<TScriptInterface<class ISpawnable>> AvailableItems;
	TQueue<TScriptInterface<class ISpawnable>> AvailableBullets;

	// Pool configuration
	UPROPERTY()
	TSubclassOf<AActor> MonsterClass;

	UPROPERTY()
	TSubclassOf<AActor> ItemClass;

	UPROPERTY()
	TSubclassOf<AActor> BulletClass;

	int32 PoolSize;

	// Helper methods
	TScriptInterface<class ISpawnable> FindAvailableObject(EPoolType Type);
	void CreatePool(EPoolType Type, TSubclassOf<AActor> ActorClass, int32 Size);
	TQueue<TScriptInterface<class ISpawnable>>& GetQueueForType(EPoolType Type);

	// Monster spawning (for testing/debug)
	UPROPERTY(EditAnywhere)
	float SpawnRadius;

	UPROPERTY(EditAnywhere)
	float SpawnInterval;
	UPROPERTY(EditAnywhere)
	float SpawnTimer;
	UPROPERTY(EditAnywhere)
	bool bPoolsInitialized;
};
