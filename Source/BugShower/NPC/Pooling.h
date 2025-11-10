// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPC/PoolingType.h"
#include "Pooling.generated.h"


//spawner that uses object pooling to manage monsters
//on off monster state in world(Activate,Rendering,Collision,RunBT...)


UCLASS()
class BUGSHOWER_API APooling : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APooling();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

	// Called when a object is returned to the pool
	UFUNCTION()
	void ReturnPool(TScriptInterface<ISpawnable> spawnable);
	// Spawn a specified type at pos
	TScriptInterface<ISpawnable> Spawn(const EPoolType type, FVector pos);


protected:
	TQueue<TScriptInterface<ISpawnable>>* GetAvailableQueue(const EPoolType type);
	void CreatePool(EPoolType InPoolType, int32 InPoolSize);

	void Spawn(const EPoolType type);	//only server spawning 

	TScriptInterface<ISpawnable>FindInActiveMonster(const EPoolType type);
	void InActiveAll();


	TMap<EPoolType, TArray<TScriptInterface<ISpawnable>>> PoolMap;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> MonsterClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ItemClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BulletClass;

	TQueue<TScriptInterface<ISpawnable>> AvailableMonsters;
	TQueue<TScriptInterface<ISpawnable>> AvailableItems;
	TQueue<TScriptInterface<ISpawnable>> AvailableBullets;

	UPROPERTY(EditAnywhere)
	int32 PoolSize;

	// Spawn Parameters
	UPROPERTY(EditAnywhere)
	float SpawnRadius;
	UPROPERTY(EditAnywhere)
	float SpawnInterval;
	float SpawnTimer;
};
