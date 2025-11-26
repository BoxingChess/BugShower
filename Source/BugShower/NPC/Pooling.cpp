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

		//PoolSys->RegisterPoolForClass(MonsterClass,PoolSize);
		//PoolSys->RegisterPoolForClass(BulletClass, PoolSize);
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

	UWorld* World = GetWorld();

	if (!World)
	{
		LOG_LOGIC_ERROR(TEXT("World is null"));
		return;
	}

	Super::Tick(DeltaTime);

	FVector SpawnLocation = GetActorLocation();

	FVector YAxis = GetActorRightVector();
	FVector ZAxis = GetActorForwardVector();

	DrawDebugCircle(World, SpawnLocation, SpawnRadius, 12, FColor::Yellow, false, -1,0,2.f, YAxis, ZAxis);

	SpawnTimer += DeltaTime;
	if (SpawnTimer >= SpawnInterval)
	{

		// Get pooling subsystem

		UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
		if (PoolSys)
		{
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
			if (!NavSys)
			{
				LOG_LOGIC_WARNING(TEXT("SpawnMonsters: NavSystem is null"));
				return;
			}

			FNavLocation RandomLocation;
			if (NavSys->GetRandomPointInNavigableRadius(SpawnLocation, SpawnRadius, RandomLocation))
			{
				TScriptInterface<ISpawnable> SpawnActor = PoolSys->SpawnFromClass(MonsterClass, RandomLocation);
			}

			LOG_LOGIC_INFO(TEXT("Monster : %s Spawned from pool"), *GetName());
		}

		SpawnTimer -= SpawnInterval;
	}

}

void APooling::ReturnAllPoolingActors()
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
		PoolSys->ReturnAllOfClass(MonsterClass);
		PoolSys->ReturnAllOfClass(BulletClass);


		LOG_LOGIC_INFO(TEXT("Monster : %s Spawned from pool"), *GetName());
	}
}
