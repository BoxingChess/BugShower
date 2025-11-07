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

	// Create Monster Pool
	for (int i = 0; i < PoolSize; i++)
	{
		if (GetWorld())
		{

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (SpawnedActor && SpawnedActor->Implements<USpawnable>())
			{
				TScriptInterface<ISpawnable> Monster;
				Monster.SetObject(SpawnedActor);
				Monster.SetInterface(Cast<ISpawnable>(SpawnedActor));

				Monster->InitState(this);

				if (!PoolMap.Contains(EPoolType::Monster))
				{
					PoolMap.Add(EPoolType::Monster, {Monster});
				}
				else
				{
					PoolMap[EPoolType::Monster].Add(Monster);
				}
				// Add to available queue
				AvailableMonsters.Enqueue(Monster);

				LOG_LOGIC_INFO(TEXT("Create Monster %d"), i);
			}
			else
			{
				LOG_LOGIC_WARNING(TEXT("Monster is not Create"));
			}

		}
	}

	// Create Item Pool
	LOG_LOGIC_INFO(TEXT("Creating Item Pool with size %d, ItemClass: %s"), PoolSize, ItemClass ? *ItemClass->GetName() : TEXT("NULL"));

	for (int i = 0; i < PoolSize; i++)
	{
		if (GetWorld())
		{

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (SpawnedActor && SpawnedActor->Implements<USpawnable>())
			{
				TScriptInterface<ISpawnable> Item;
				Item.SetObject(SpawnedActor);
				Item.SetInterface(Cast<ISpawnable>(SpawnedActor));

				Item->InitState(this);

				if (!PoolMap.Contains(EPoolType::Item))
				{
					PoolMap.Add(EPoolType::Item, {Item});

				}
				else
				{
					PoolMap[EPoolType::Item].Add(Item);
				}

				// Add to available queue
				AvailableItems.Enqueue(Item);

				LOG_LOGIC_INFO(TEXT("Create item %d"), i);
			}
			else
			{
				LOG_LOGIC_WARNING(TEXT("Item is not Create"));
			}

		}
	}

	// Create Bullet Pool
	for (int i = 0; i < PoolSize; i++)
	{
		if (GetWorld())
		{

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(BulletClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (SpawnedActor && SpawnedActor->Implements<USpawnable>())
			{
				TScriptInterface<ISpawnable> Bullet;
				Bullet.SetObject(SpawnedActor);
				Bullet.SetInterface(Cast<ISpawnable>(SpawnedActor));

				Bullet->InitState(this);

				if (!PoolMap.Contains(EPoolType::Item))
				{
					PoolMap.Add(EPoolType::Bullet, { Bullet });

				}
				else
				{
					PoolMap[EPoolType::Bullet].Add(Bullet);
				}

				// Add to available queue
				AvailableBullets.Enqueue(Bullet);

				LOG_LOGIC_INFO(TEXT("Create Bullet %d"), i);
			}
			else
			{
				LOG_LOGIC_WARNING(TEXT("Bullet is not Create"));
			}

		}
	}

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

void APooling::Spawn(const EPoolType type)
{
	if (!HasAuthority())
		return;

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

void APooling::Spawn(const EPoolType type, FVector pos)
{
	if (!HasAuthority())
		return;

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
