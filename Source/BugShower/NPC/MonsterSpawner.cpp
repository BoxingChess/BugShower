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


	PoolSize = 100;
	SpawnRadius = 1500.f;
	MonsterClass = AMonsterBase::StaticClass();
	SpawnInterval	= 3.0f;
	SpawnTimer = 0.0f;
}

// Called when the game starts or when spawned
void ASpawnMonster::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
		return;

	for (int i = 0; i < PoolSize; i++)
	{
		if (GetWorld())
		{

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			AMonsterBase* Monster = GetWorld()->SpawnActor<AMonsterBase>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator,SpawnParams);
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

	SpawnTimer += DeltaTime;
	if (SpawnTimer >= SpawnInterval)
	{
		Spawn();
		SpawnTimer -= SpawnInterval;
	}
}

void ASpawnMonster::Spawn()
{
	if (!HasAuthority())
		return;

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

			SpawnPos.Location.Z = 0;

			Monster->SetActorLocation(SpawnPos.Location);
			Monster->SetActorHiddenInGame(false);
			Monster->SetActorEnableCollision(true);
			Monster->SetActorTickEnabled(true);
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
	if (!HasAuthority())
		return;

	for (auto& Monster : MonsterPool)
	{
		if (!Monster->IsHidden())
		{
			Monster->SetActorHiddenInGame(true);
			Monster->SetActorEnableCollision(false);
			Monster->SetActorTickEnabled(false);
		}
	}
}

