// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
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

	if (HasAuthority())
	{
		CurrentLifeTime += DeltaTime;
		if (CurrentLifeTime >= LifeSpan)
		{
			Destroy();
		}
	}

	// Add rotation animation
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += DeltaTime * 90.f;
	SetActorRotation(NewRotation);
}

void AItemBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	// Check if overlapping actor is a player
	if (OtherActor && OtherActor->IsA(APawn::StaticClass()))
	{
		OnPickup(OtherActor);
	}
}

void AItemBase::OnPickup(AActor* PickupActor)
{
	if (!HasAuthority())
		return;

	UE_LOG(LogTemp, Log, TEXT("Item %s picked up by %s"), *ItemName, *PickupActor->GetName());

	// TODO: Add item to player inventory
	// For now, just destroy the item
	Destroy();
}
