// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Collision component setup
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);

	// Set up collision properly for bouncing
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	// Enable physics simulation for bouncing
	CollisionComponent->SetSimulatePhysics(false); // ProjectileMovement handles physics
	CollisionComponent->SetNotifyRigidBodyCollision(true); // Enable hit events

	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	RootComponent = CollisionComponent;

	// Mesh component setup
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set default mesh to engine's basic sphere
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshRef(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshRef.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMeshRef.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f)); // Make it smaller (20% size)
	}

	// Projectile movement setup
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	// Set lifespan (auto destroy after 10 seconds)
	InitialLifeSpan = 10.0f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Don't hit ourselves or our owner
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// Apply damage (only if Damage is set)
		if (Damage > 0.0f)
		{
			UGameplayStatics::ApplyDamage(
				OtherActor,
				Damage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass()
			);

			UE_LOG(LogTemp, Warning, TEXT("Projectile hit: %s - Damage: %.1f"), *OtherActor->GetName(), Damage);

			// Destroy projectile on hit (only if it does damage)
			Destroy();
		}
		else
		{
			// No damage means this projectile should bounce or behave differently
			// Let child classes handle it (like Grenade)
			UE_LOG(LogTemp, Log, TEXT("Projectile hit: %s (no damage, bouncing)"), *OtherActor->GetName());
		}
	}
}

void AProjectileBase::InitVelocity(const FVector& ShootDirection)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = ShootDirection * InitialSpeed;
	}
}
