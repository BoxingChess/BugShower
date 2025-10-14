// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

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

	void SpawnMonster();

protected:
	class AMonsterBase* FindInActiveMonster();
	void InActiveAll();

	UPROPERTY(EditAnywhere)
    TSubclassOf<class AMonsterBase> MonsterClass;

	TArray<class AMonsterBase*> MonsterPool;
	int32 PoolSize;
	float SpawnRadius;
	float SpawnTime;
	float CheckTime;

	TObjectPtr<class AChaosDungeonGameMode> GameMode;
};
