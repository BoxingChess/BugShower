// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Logging/BugShowerLog.h"
#include "NPC/Pooling.h"

void AItemBase::InitState(AActor* InOwningSpawnPool)
{
	//set inactive state
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	//set owning pool
	OwningPool = InOwningSpawnPool;
}

void AItemBase::Spawn(const FVector pos)
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	SetActorLocation(pos);

	// Reset lifetime when spawned from pool
	CurrentLifeTime = 0.f;
}

void AItemBase::ReturnPool()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (OwningPool.IsValid())
	{
		APooling* PoolActor = Cast<APooling>(OwningPool.Get());
		if (PoolActor)
		{
			PoolActor->ReturnPool(this);
		}
	}
}

void AItemBase::DeSpawn()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (OwningPool.IsValid())
	{
		APooling* PoolActor = Cast<APooling>(OwningPool.Get());
		if (PoolActor)
		{
			PoolActor->ReturnPool(this);
		}
	}
}

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication(pos,life span, type)
	bReplicates = true;
	SetReplicateMovement(true);

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(50.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetIsReplicated(true);

	// Default values
	ItemName = TEXT("Item");
	ItemType = EItemType::MATERIAL;
	ItemValue = 1;
	LifeSpan = 30.f;
	CurrentLifeTime = 0.f;
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnOverlapBegin);
	}
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Server
	if (HasAuthority())
	{
		CurrentLifeTime += DeltaTime;
		if (CurrentLifeTime >= LifeSpan)
		{
			DeSpawn();
		}
	}

	// Add rotation animation
	//FRotator NewRotation = GetActorRotation();
	//NewRotation.Yaw += DeltaTime * 90.f;
	//SetActorRotation(NewRotation);


	//Client
	{
		// Add rotation animation (mesh only, no network replication)
		if (MeshComponent)
		{
			FRotator NewRotation = MeshComponent->GetRelativeRotation();
			NewRotation.Yaw += DeltaTime * 90.f;
			MeshComponent->SetRelativeRotation(NewRotation);
		}
	}
}

void AItemBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	// Check if overlapping actor is a player
	if (OtherActor && OtherActor->IsA(APawn::StaticClass() ))
	{
		//test if it's player controller
		APawn* Pawn = Cast<APawn>(OtherActor);

		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{

			OnPickup(OtherActor);
		}
	}
}

void AItemBase::OnPickup(AActor* PickupActor)
{
	if (!HasAuthority())
		return;

	LOG_LOGIC_INFO(TEXT("Item %s picked up by %s"), *ItemName, *PickupActor->GetName());

	// TODO: Add item to player inventory and 
	// TODO: don't runtime destroy change to item pooling
	DeSpawn();
}
