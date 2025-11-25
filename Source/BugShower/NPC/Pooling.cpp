// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Pooling.h"
#include "NPC/Spawnable.h"
#include "NavigationSystem.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Game/BSGameModeBase.h"
#include "Logging/BugShowerLog.h"


APooling::APooling()
{
	PrimaryActorTick.bCanEverTick = true;

	PoolSize = 1;
	SpawnRadius = 1500.f;
	SpawnInterval = 3.0f;
	SpawnTimer = 0.0f;

}

void APooling::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	UWorld* World = GetWorld();
	if (World)
	{
		UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
		if (!PoolSys)
		{
			LOG_LOGIC_ERROR(TEXT("Pooling: PoolingSubsystem not found"));
			return;
		}

		PoolSys->InitializePools(MonsterClass,ItemClass,BulletClass,PoolSize);
	}

}

void APooling::Tick(float DeltaTime)
{
	if (!HasAuthority())
		return;

	ABSGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ABSGameModeBase>();
	if (!GameMode)
		return;

	if (GameMode->IsEnd())
		return;

	Super::Tick(DeltaTime);

	DrawDebugSphere(GetWorld(), GetActorLocation(), SpawnRadius, 12, FColor::Yellow, false, 2.f);

	SpawnTimer += DeltaTime;
	if (SpawnTimer >= SpawnInterval)
	{

		// Get pooling subsystem
		UWorld* World = GetWorld();
		if (!World)
		{
			LOG_LOGIC_ERROR(TEXT("DropItems: World is null"));
			return;
		}

		UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
		if (PoolSys)
		{
			PoolSys->SpawnMonsters(GetActorLocation(),SpawnRadius);
			LOG_LOGIC_INFO(TEXT("Monster : %s Spawned from pool"), *GetName());
		}

		SpawnTimer -= SpawnInterval;
	}

}
