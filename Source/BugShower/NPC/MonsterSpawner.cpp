// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterSpawner.h"
#include "NPC/MonsterBase.h"
#include "NPC/MonsterAIController.h"
#include "NavigationSystem.h"


// Sets default values
ASpawnMonster::ASpawnMonster()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	PoolSize = 1000;
	SpawnRadius = 1500.f;
	MonsterClass = AMonsterBase::StaticClass();
	SpawnTime = 3.0f;
	CheckTime = 0.0f;
}

// Called when the game starts or when spawned
void ASpawnMonster::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < PoolSize; i++)
	{
		if (GetWorld())
		{

			AMonsterBase* Monster = GetWorld()->SpawnActor<AMonsterBase>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator);
			if (Monster)
			{
				Monster->SetActorHiddenInGame(true);
				Monster->SetActorEnableCollision(false);
				Monster->SetActorTickEnabled(false);
				MonsterPool.Add(Monster);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Monster is Not Spawn"));
			}

		}
	}
}

// Called every frame
void ASpawnMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugSphere(GetWorld(), GetActorLocation(), SpawnRadius, 12, FColor::Yellow, false, 2.f);

	CheckTime += DeltaTime;
	if (CheckTime > SpawnTime)
	{
		for (int j = 0; j < 3; j++)
		{
			SpawnMonster();
			CheckTime -= SpawnTime;
		}
	}
}

void ASpawnMonster::SpawnMonster()
{
	AMonsterBase* Monster = FindInActiveMonster();
	if (Monster)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
		if (NavSystem == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("NavSys is Null"));
			return;
		}

		FNavLocation SpawnPos;
		if (NavSystem->GetRandomPointInNavigableRadius(GetActorLocation(), SpawnRadius, SpawnPos))
		{
			if (SpawnPos.Location.Z <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Name : %s, X: %f, Y: %f,Z: %f"), *Monster->GetName(), SpawnPos.Location.X, SpawnPos.Location.Y, SpawnPos.Location.Z);
			}

			SpawnPos.Location.Z = 90;


			Monster->Spawn(SpawnPos.Location);
		}


	}
}

AMonsterBase* ASpawnMonster::FindInActiveMonster()
{
	for (auto& Monster : MonsterPool)
	{
		if (Monster->IsHidden())
		{
			return Monster;
		}
	}

	return nullptr;
}

void ASpawnMonster::InActiveAll()
{
	for (auto& Monster : MonsterPool)
	{
		if (!Monster->IsHidden())
		{
			Monster->DeSpawn();
		}
	}
}

