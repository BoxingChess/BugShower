// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Logging/BugShowerLog.h"
#include "Player/BSCharacterPlayer.h"
#include "Subsystems/PoolingSubsystem.h"

void AItemBase::Spawn(const FVector pos)
{
	Activate(this,pos);

	SetLifeSpan(ItemLifeSpan); //30 seconds life span
}

void AItemBase::ReturnPool()
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

void AItemBase::LifeSpanExpired()
{
	DeSpawn();
}

void AItemBase::DeSpawn()
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

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	MeshComponent->SetIsReplicated(true);


	// Default values
	ItemName = TEXT("Item");
	ItemType = EItemType::Material;
	ItemValue = 1;
	ItemLifeSpan = 5.f;
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
	if (OtherActor && OtherActor->IsA(ABSCharacterPlayer::StaticClass() ))
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
