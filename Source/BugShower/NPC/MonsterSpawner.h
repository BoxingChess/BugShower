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

	void Spawn();
	

protected:
	class AMonsterBase* FindInActiveMonster();
	void InActiveAll();

	//소환될 몬스터 클래스
	UPROPERTY(EditAnywhere)
    TSubclassOf<class AMonsterBase> MonsterClass;

	TArray<class AMonsterBase*> MonsterPool;
	UPROPERTY(EditAnywhere)
	int32 PoolSize;

	// Spawn Parameters
	UPROPERTY(EditAnywhere)
	float SpawnRadius;
	UPROPERTY(EditAnywhere)
	float SpawnTime;
	UPROPERTY(EditAnywhere)
	float CheckTime;
};
