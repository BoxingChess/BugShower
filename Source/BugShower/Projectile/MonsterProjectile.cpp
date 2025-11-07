// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/MonsterProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/BugShowerLog.h"

AMonsterProjectile::AMonsterProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Set up replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Create collision component
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AMonsterProjectile::OnProjectileHit);
	RootComponent = CollisionComp;

	// Create mesh component (visual)
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create projectile movement component (handles arc trajectory)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 5000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;  // Enable gravity for arc

	// Default values
	Damage = 10.0f;
	ProjectileOwner = nullptr;

	// Auto-destroy after 5 seconds
	InitialLifeSpan = 5.0f;
}

void AMonsterProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AMonsterProjectile::InitializeProjectile(const FVector& Direction, float InDamage, AActor* InOwner)
{
	Damage = InDamage;
	ProjectileOwner = InOwner;

	if (ProjectileMovement)
	{
		// Set velocity in the specified direction
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;

		LOG_LOGIC_INFO(TEXT("Projectile initialized: Direction=%s, Damage=%.1f, Speed=%.1f"),
			*Direction.ToString(), Damage, ProjectileMovement->InitialSpeed);
	}
}

void AMonsterProjectile::InitializeProjectileWithVelocity(const FVector& Velocity, float InDamage, AActor* InOwner)
{
	Damage = InDamage;
	ProjectileOwner = InOwner;

	if (ProjectileMovement)
	{
		// Set exact velocity (already calculated with arc trajectory)
		ProjectileMovement->Velocity = Velocity;

		LOG_LOGIC_INFO(TEXT("Projectile initialized with velocity: Velocity=%s, Damage=%.1f"),
			*Velocity.ToString(), Damage);
	}
}

void AMonsterProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only process on server
	if (!HasAuthority())
		return;

	// Don't hit the owner
	if (OtherActor == ProjectileOwner || OtherActor == this)
		return;

	LOG_LOGIC_INFO(TEXT("Projectile hit: %s"), *OtherActor->GetName());

	// Apply damage to the hit actor
	if (OtherActor && Damage > 0.0f)
	{
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			ProjectileOwner ? ProjectileOwner->GetInstigatorController() : nullptr,
			this,
			UDamageType::StaticClass()
		);

		LOG_LOGIC_INFO(TEXT("Projectile dealt %.1f damage to %s"), Damage, *OtherActor->GetName());
	}

	// Destroy projectile after hit
	Destroy();
}
