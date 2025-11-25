// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Item/ItemDropStructs.h"
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
		TSubclassOf<AActor> ActorClass,
		const FVector& SpawnLocation,
		float ProjectileSpeed,
		float Damage
	);

	// ========== Class-Based Pooling API ==========
	// Register a new pool for a specific monster class
	UFUNCTION(BlueprintCallable, Category = "Pooling|ClassBased")
	void RegisterPoolForClass(TSubclassOf<AActor> ActorClass, int32 Size);

	// Spawn from a class-specific pool
	UFUNCTION(BlueprintCallable, Category = "Pooling|ClassBased")
	TScriptInterface<class ISpawnable> SpawnFromClass(
		TSubclassOf<AActor> ActorClass,
		const FVector& Position
	);



	// Return object to its class-specific pool
	void ReturnToPoolByClass(TScriptInterface<class ISpawnable> Object);

	// Return all monsters of a specific class to pool
	UFUNCTION(BlueprintCallable, Category = "Pooling|ClassBased")
	void ReturnAllOfClass(TSubclassOf<AActor> ActorClass);

	// ========== Drop Management API ==========
	// Process monster drop using DataTable configuration
	UFUNCTION(BlueprintCallable, Category = "Pooling|DropManagement")
	void ProcessMonsterDrop(
		FName MonsterDropID,
		const FVector& DropLocation,
		const FGameplayTagContainer& ActiveConditions = FGameplayTagContainer()
	);

	// Set the drop configuration DataTable
	UFUNCTION(BlueprintCallable, Category = "Pooling|DropManagement")
	void SetDropConfigTable(UDataTable* DropTable);

protected:
	// Calculate drops from configuration
	TArray<TSubclassOf<class AItemBase>> CalculateDropsFromConfig(
		const FMonsterDropConfig& Config,
		const FGameplayTagContainer& ActiveConditions
	) const;

	// Select items by weight from drop entries
	TArray<TSubclassOf<class AItemBase>> SelectDropsByWeight(
		const TArray<FItemDropEntry>& Entries,
		int32 MaxSelections,
		const FGameplayTagContainer& ActiveConditions
	) const;

	// Spawn dropped items in a spread pattern
	void SpawnDroppedItems(
		const TArray<TSubclassOf<class AItemBase>>& ItemClasses,
		const FVector& CenterLocation,
		float SpreadRadius
	);

private:
	// ========== Class-Based Pooling ==========
	// Pool data structure for each class
	struct FPoolData
	{
		TArray<TScriptInterface<class ISpawnable>> Available;
		TArray<TScriptInterface<class ISpawnable>> All;
	};

	// Map from actor class to its pool data
	TMap<TSubclassOf<AActor>, FPoolData> ClassPools;

	
	int32 PoolSize;

	UPROPERTY(EditAnywhere)
	bool bPoolsInitialized;

	// ========== Drop Management ==========
	// DataTable containing monster drop configurations
	UPROPERTY()
	UDataTable* MonsterDropTable;
};
