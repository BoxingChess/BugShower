// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Item/ItemActor.h"
#include "Game/BSGameInstance.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"
#include "Player/BSCharacterPlayer.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Logging/BugShowerLog.h"


void AItemActor::Spawn(const FVector pos)
{
	if (StaticItemInfo)
	{
		Activate(this, pos);
		SetLifeSpan(StaticItemInfo->ItemLifeSpan);
	}
}

void AItemActor::ReturnPool()
{
	Deactivate(this);

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPoolByClass(this);
		}
	}
}

void AItemActor::LifeSpanExpired()
{
	DeSpawn();
}

void AItemActor::DeSpawn()
{
	Deactivate(this);

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPoolByClass(this);
		}
	}
}



// Sets default values
AItemActor::AItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Enable replication(pos,life span, type)
	bReplicates = true;
	SetReplicateMovement(true);

	// �޽� ������Ʈ ����
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetIsReplicated(true);

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(50.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);


	// Ʈ������ ������ �����ϵ��� ��Ʈ ������Ʈ�� ����
	RootComponent = MeshComponent;

}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnOverlapBegin);
	}
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (StaticItemInfo && StaticItemInfo->WorldMesh)
	{
		MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Loading StaticItemInfo"));

	}
}

// Initialize item with dynamic data
// 동적 아이템 데이터로 초기화
void AItemActor::InitializeItemBS_Item(FBS_Item& _item)
{
	// Set dynamic item data (quantity, durability, etc.)
	// 동적 아이템 데이터 설정 (수량, 내구도 등)
	ItemInformation = _item;

	// Load static item data from ItemResourceManager Subsystem
	// ItemResourceManager 서브시스템에서 정적 아이템 데이터 로드
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		UItemResourceManager* ResourceManager = GI->GetSubsystem<UItemResourceManager>();
		if (ResourceManager)
		{
			// Get static data (mesh, icon, description, etc.)
			// 정적 데이터 가져오기 (메시, 아이콘, 설명 등)
			StaticItemInfo = ResourceManager->GetStaticItem(_item.ItemType, _item.ItemID);

			if (StaticItemInfo)
			{
				// Update mesh if available
				// 메시가 있으면 적용
				if (StaticItemInfo->WorldMesh && MeshComponent)
				{
					MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ItemActor::InitializeItemBS_Item - Failed to find static data for item type %d, ID %d"),
					static_cast<uint8>(_item.ItemType), _item.ItemID);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ItemActor::InitializeItemBS_Item - ItemResourceManager Subsystem not found!"));
		}
	}
}

void AItemActor::OnPickup(AActor* PickupActor)
{
	if (!HasAuthority())
		return;

	LOG_LOGIC_INFO(TEXT("Item %s picked up by %s"), *(StaticItemInfo->DisplayName.ToString()), *PickupActor->GetName());

	// TODO: Add item to player inventory and 
	// TODO: don't runtime destroy change to item pooling
	DeSpawn();
}

void AItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 	if (!HasAuthority())
		return;

	// Check if overlapping actor is a player
	if (OtherActor && OtherActor->IsA(ABSCharacterPlayer::StaticClass()))
	{
		//test if it's player controller
		APawn* Pawn = Cast<APawn>(OtherActor);

		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{

			OnPickup(OtherActor);
		}
	}
}

