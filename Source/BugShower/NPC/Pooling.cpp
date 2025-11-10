// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Pooling.h"
#include "NPC/Spawnable.h"
#include "NavigationSystem.h"

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

	CreatePool(EPoolType::Monster, PoolSize);
	CreatePool(EPoolType::Item, PoolSize);
	CreatePool(EPoolType::Bullet, PoolSize);

}

void APooling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugSphere(GetWorld(), GetActorLocation(), SpawnRadius, 12, FColor::Yellow, false, 2.f);

	SpawnTimer += DeltaTime;
	if (SpawnTimer >= SpawnInterval)
	{
		Spawn(EPoolType::Monster);
		SpawnTimer -= SpawnInterval;
	}

}

TScriptInterface<ISpawnable> APooling::FindInActiveMonster(EPoolType type)
{
	switch (type)
	{
		case EPoolType::Monster:
		{
			TScriptInterface<ISpawnable> Monster = nullptr;
			if (AvailableMonsters.Dequeue(Monster))
			{
				return Monster;
			}
		}
		break;

		case EPoolType::Item:
		{
			TScriptInterface<ISpawnable> Item = nullptr;
			if (AvailableItems.Dequeue(Item))
			{
				return Item;
			}
		}
		break;

		case EPoolType::Bullet:
		{
			TScriptInterface<ISpawnable> Bullet = nullptr;
			if (AvailableBullets.Dequeue(Bullet))
			{
				return Bullet;
			}
		}
		break;

		default:
			break;
	}

	LOG_LOGIC_INFO("Return null if no spawnable %d object is available",type);
	// Return null if no spawnable object is available
	return TScriptInterface<ISpawnable>();
}

void APooling::InActiveAll()
{
	if (!HasAuthority())
		return;

	for (auto& Monster : PoolMap[EPoolType::Monster])
	{
		AActor* Actor = Cast<AActor>(Monster.GetObject());

		if (Actor && !Actor->IsHidden())
		{
			Monster->DeSpawn();
		}
	}

	for (auto& Item: PoolMap[EPoolType::Item])
	{
		AActor* Actor = Cast<AActor>(Item.GetObject());

		if (Actor && !Actor->IsHidden())
		{
			Item->DeSpawn();
		}
	}

	for (auto& Bullet: PoolMap[EPoolType::Bullet])
	{
		AActor* Actor = Cast<AActor>(Bullet.GetObject());

		if (Actor && !Actor->IsHidden())
		{
			Bullet->DeSpawn();
		}
	}
}

void APooling::CreatePool(EPoolType InPoolType, int32 InPoolSize)
{
	FString PoolingClassName;
	TSubclassOf<AActor> PoolingClass;
	switch (InPoolType)
	{
		case EPoolType::Monster:
			PoolingClassName = MonsterClass ? *MonsterClass->GetName() : TEXT("NULL");
			PoolingClass = MonsterClass;
			break;
		case EPoolType::Item:
			PoolingClassName = ItemClass ? *ItemClass->GetName() : TEXT("NULL");
			PoolingClass = ItemClass;
			break;
		case EPoolType::Bullet:
			PoolingClassName = BulletClass ? *BulletClass->GetName() : TEXT("NULL");
			PoolingClass = BulletClass;
			break;
		default:
			break;
	}

	LOG_LOGIC_INFO(TEXT("Creating Item Pool with size %d, PoolingClass: %s"), PoolSize, PoolingClassName);

	for (int i = 0; i < InPoolSize; i++)
	{
		if (GetWorld())
		{

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			AActor* Spawnable = GetWorld()->SpawnActor<AActor>(PoolingClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (Spawnable && Spawnable->Implements<USpawnable>())
			{
				TScriptInterface<ISpawnable> SpawnActor;
				SpawnActor.SetObject(Spawnable);
				SpawnActor.SetInterface(Cast<ISpawnable>(Spawnable));
				SpawnActor->InitState(this);

				if (!PoolMap.Contains(InPoolType))
				{
					PoolMap.Add(InPoolType, {SpawnActor});

				}
				else
				{
					PoolMap[InPoolType].Add(SpawnActor);
				}

				// Add to available queue
				switch (InPoolType)
				{
					case EPoolType::Monster:
						AvailableMonsters.Enqueue(SpawnActor);
						break;
					case EPoolType::Item:
						AvailableItems.Enqueue(SpawnActor);
						break;
					case EPoolType::Bullet:
						AvailableBullets.Enqueue(SpawnActor);
						break;
					default:
						break;
				}


				LOG_LOGIC_INFO(TEXT("Create Pooling Actor %d"), i);
			}
			else
			{
				LOG_LOGIC_WARNING(TEXT("Pooling Actor is not Create"));
			}

		}
	}
}

void APooling::Spawn(const EPoolType type)
{
	if (!HasAuthority())
		return ;

	TScriptInterface<ISpawnable> Spawnable = FindInActiveMonster(type);

	if (Spawnable)
	{
		// Find random spawn location within radius
		FNavLocation SpawnPos;
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSys && NavSys->GetRandomPointInNavigableRadius(GetActorLocation(), SpawnRadius, SpawnPos))
		{
			Spawnable->Spawn(SpawnPos.Location);
			LOG_LOGIC_INFO(TEXT("Spawned %d at location: %s"), (int)type, *SpawnPos.Location.ToString());
		}
		else
		{
			LOG_LOGIC_WARNING(TEXT("Failed to find spawn location for %d"), (int)type);
		}
	}
	else
	{
		LOG_LOGIC_WARNING(TEXT("No available %d to spawn"), (int)type);
	}
}

TScriptInterface<ISpawnable> APooling::Spawn(const EPoolType type, FVector pos)
{
	if (!HasAuthority())
		return TScriptInterface<ISpawnable>();

	LOG_LOGIC_INFO(TEXT("Trying to spawn type %d at %s"), (int)type, *pos.ToString());

	TScriptInterface<ISpawnable> Spawnable = FindInActiveMonster(type);

	if (Spawnable)
	{
		Spawnable->Spawn(pos);
		LOG_LOGIC_INFO(TEXT("Successfully spawned %d at location: %s"), (int)type, *pos.ToString());
	}
	else
	{
		LOG_LOGIC_ERROR(TEXT("No available %d in pool! Pool may be empty or not initialized."), (int)type);
	}

	return Spawnable;
}

void APooling::ReturnPool(TScriptInterface<ISpawnable> spawnable)
{
	switch (spawnable->GetPoolType())
	{
		case EPoolType::Monster:
		{
			AvailableMonsters.Enqueue(spawnable);
			break;
		}
		case EPoolType::Item:
		{
			AvailableItems.Enqueue(spawnable);
			break;
		}
		case EPoolType::Bullet:
		{
			AvailableBullets.Enqueue(spawnable);
			break;
		}
		default:
			break;
	}
}
