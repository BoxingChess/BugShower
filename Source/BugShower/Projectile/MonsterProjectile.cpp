// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/MonsterProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/BugShowerLog.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Player/BSCharacterPlayer.h"

void AMonsterProjectile::Spawn(const FVector pos)
{
	Activate(this, pos);

	// Reset projectile state
	ProjectileOwner = nullptr;
	Damage = 0.0f;

	// Ensure collision is enabled
	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Reset velocity
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}

	SetLifeSpan(Life);	// Reset lifespan timer
}

void AMonsterProjectile::ReturnPool()
{
	Deactivate(this);

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPool(this);
		}
	}
}

void AMonsterProjectile::DeSpawn()
{
	// Disable collision before returning to pool
	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Reset state
	Deactivate(this);
	ProjectileOwner = nullptr;
	Damage = 0.0f;

	// Stop movement
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = FVector::ZeroVector;
		ProjectileMovement->StopMovementImmediately();
	}

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPool(this);
		}
	}
}

AMonsterProjectile::AMonsterProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Set up replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Create collision component
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);

	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AMonsterProjectile::OnProjectileOverlap);


	CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// pawn channel overlap for hitting characters
	CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	//CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	//CollisionComp->SetNotifyRigidBodyCollision(true); // Enable hit events
	//CollisionComp->OnComponentHit.AddDynamic(this, &AMonsterProjectile::OnProjectileHit);

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

	Life = 5.0f;
	// Auto-destroy after 5 seconds
	InitialLifeSpan = Life;
}

void AMonsterProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AMonsterProjectile::InitializeProjectile(const FVector& Direction, float InDamage, AActor* InOwner)
{
	if (!InOwner)
	{
		LOG_LOGIC_WARNING(TEXT("InitializeProjectile: Owner is null, returning to pool"));
		DeSpawn();
		return;
	}

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
	if (!InOwner)
	{
		LOG_LOGIC_WARNING(TEXT("InitializeProjectileWithVelocity: Owner is null, returning to pool"));
		return;
	}

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

void AMonsterProjectile::LifeSpanExpired()
{
	// On lifespan expiry, return to pool
	DeSpawn();
}

void AMonsterProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 디버그: 모든 충돌 출력
	UE_LOG(LogTemp, Warning, TEXT("MonsterProjectile Overlap! OtherActor: %s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"));

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterProjectile: No authority, ignoring"));
		return;
	}

	// Get valid owner reference
	AActor* AttackActor = ProjectileOwner.Get();

	// Don't hit the owner or self
	if (OtherActor == AttackActor || OtherActor == this)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterProjectile: Hit owner or self, ignoring"));
		return;
	}

	// Don't hit invalid actors
	if (!IsValid(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterProjectile: Invalid actor, ignoring"));
		return;
	}


	ABSCharacterPlayer* PlayerCharacter = Cast<ABSCharacterPlayer>(OtherActor);
	if (PlayerCharacter)
	{
		LOG_LOGIC_INFO(TEXT("Projectile hit a player character: %s"), *PlayerCharacter->GetName());

		// Apply damage to the hit actor
		if (Damage > 0.0f)
		{
			UGameplayStatics::ApplyDamage(
				OtherActor,
				Damage,
				(AttackActor && IsValid(AttackActor)) ? AttackActor->GetInstigatorController() : nullptr,
				this,
				UDamageType::StaticClass()
			);

			LOG_LOGIC_INFO(TEXT("Projectile dealt %.1f damage to %s"), Damage, *OtherActor->GetName());
		}
	}


	// Return to pool after hit
	DeSpawn();

}

void AMonsterProjectile::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only process on server
	if (!HasAuthority())
		return;

	// Get valid owner reference
	AActor* AttackActor = ProjectileOwner.Get();

	// Don't hit the owner or self
	if (OtherActor == AttackActor || OtherActor == this)
		return;

	// Don't hit invalid actors
	if (!IsValid(OtherActor))
		return;

	LOG_LOGIC_INFO(TEXT("Projectile hit: %s"), *OtherActor->GetName());

	// Apply damage to the hit actor
	if (Damage > 0.0f)
	{
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			(AttackActor && IsValid(AttackActor)) ? AttackActor->GetInstigatorController() : nullptr,
			this,
			UDamageType::StaticClass()
		);

		LOG_LOGIC_INFO(TEXT("Projectile dealt %.1f damage to %s"), Damage, *OtherActor->GetName());
	}

	// Return to pool after hit
	DeSpawn();
}
