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

public:	
	// Sets default values for this actor's properties
	APooling();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> MonsterClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ItemClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BulletClass;


	UPROPERTY(EditAnywhere)
	uint32 PoolSize;

	// Spawn Parameters
	UPROPERTY(EditAnywhere)
	float SpawnRadius;
	UPROPERTY(EditAnywhere)
	float SpawnInterval;
	float SpawnTimer;
};
