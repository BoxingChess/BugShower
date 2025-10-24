// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterSpawner.h"
#include "NPC/MonsterBase.h"
#include "NPC/MonsterStatComponent.h"
#include "NavigationSystem.h"
#include "NPC/MonsterAIController.h"

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

				// Subscribe to monster death event
				UMonsterStatComponent* StatComp = Monster->GetMonsterStatComponent();
				if (StatComp)
				{
					StatComp->OnMonsterDeath.AddDynamic(this, &ASpawnMonster::OnMonsterDied);
				}

				// Add to available queue
				AvailableMonsters.Enqueue(Monster);

				UE_LOG(LogTemp, Warning, TEXT("Create Monster %d"),i);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Monster is not Create"));
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
		UMonsterStatComponent* stat = Monster->GetMonsterStatComponent();
		if (stat)
		{
			stat->ResetHP();
		}


		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
		if (NavSystem == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("NavSys is Null"));
			return;
		}

		FNavLocation SpawnPos;
		if (NavSystem->GetRandomPointInNavigableRadius(GetActorLocation(), SpawnRadius, SpawnPos))
		{
			SpawnPos.Location.Z = 0;
			UE_LOG(LogTemp, Warning, TEXT("Name : %s, X: %f, Y: %f,Z: %f"), *Monster->GetName(), SpawnPos.Location.X, SpawnPos.Location.Y, SpawnPos.Location.Z);

			

			Monster->SetActorLocation(SpawnPos.Location);
			Monster->SetActorHiddenInGame(false);
			Monster->SetActorEnableCollision(true);
			Monster->SetActorTickEnabled(true);

			AAIController* AI = Cast<AAIController>(Monster->GetController());
			if (AI)
			{
				AMonsterAIController* MonsterAI = Cast<AMonsterAIController>(AI);
				MonsterAI->RunAI();
			}
		}


	}
}


AMonsterBase* ASpawnMonster::FindInActiveMonster()
{
	AMonsterBase* Monster = nullptr;
	if (AvailableMonsters.Dequeue(Monster))
	{
		return Monster;
	}

	return nullptr;
}

void ASpawnMonster::OnMonsterDied(AActor* DeadMonster)
{
	if (!HasAuthority())
		return;

	AMonsterBase* Monster = Cast<AMonsterBase>(DeadMonster);
	if (!Monster)
		return;

	// Deactivate monster (return to pool)
	Monster->SetActorHiddenInGame(true);
	Monster->SetActorEnableCollision(false);
	Monster->SetActorTickEnabled(false);

	// Stop AI
	AAIController* AI = Cast<AAIController>(Monster->GetController());
	if (AI)
	{
		AMonsterAIController* MonsterAI = Cast<AMonsterAIController>(AI);
		if (MonsterAI)
		{
			MonsterAI->StopAI();
		}
	}

	// Add back to available queue
	AvailableMonsters.Enqueue(Monster);

	UE_LOG(LogTemp, Log, TEXT("Monster died and returned to pool: %s"), *Monster->GetName());
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

			AAIController* AI = Cast<AAIController>(Monster->GetController());
			if (AI)
			{
				AMonsterAIController* MonsterAI = Cast<AMonsterAIController>(AI);
				MonsterAI->StopAI();
			}
		}
	}
}

