// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

//spawner that uses object pooling to manage monsters
//on off monster state in world(Activate,Rendering,Collision,RunBT...)
UCLASS()
class BUGSHOWER_API ASpawnMonster : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnMonster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

protected:
	class AMonsterBase* FindInActiveMonster();
	void InActiveAll();
	void Spawn();

	// Called when a monster dies
	UFUNCTION()
	void OnMonsterDied(AActor* DeadMonster);

	//monster class to spawn
	UPROPERTY(EditAnywhere)
    TSubclassOf<class AMonsterBase> MonsterClass;

	TArray<class AMonsterBase*> MonsterPool;
	TQueue<class AMonsterBase*> AvailableMonsters;

	UPROPERTY(EditAnywhere)
	int32 PoolSize;

	// Spawn Parameters
	UPROPERTY(EditAnywhere)
	float SpawnRadius;
	UPROPERTY(EditAnywhere)
	float SpawnInterval;
	float SpawnTimer;
};
