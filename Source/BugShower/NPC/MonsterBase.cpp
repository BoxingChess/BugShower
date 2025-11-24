// Fill out your copyright notice in the Description page of Project Settings.

#include "NPC/MonsterBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/MonsterAIController.h"
#include "NPC/MonsterStatComponent.h"
#include "Logging/BugShowerLog.h"
#include "Subsystems/PoolingSubsystem.h"


void AMonsterBase::Spawn(const FVector pos)
{
	Activate(this, pos);

	MonsterStatComp->ResetHP();

	AAIController* AI = Cast<AAIController>(GetController());
	if (AI)
	{
		AMonsterAIController* MonsterAI = Cast<AMonsterAIController>(AI);
		if (MonsterAI)
		{
			MonsterAI->RunAI();
			return;
		}
	}
}

void AMonsterBase::DeSpawn()
{
	Deactivate(this);

	// Stop AI
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI)
	{
		AMonsterAIController* MonsterAI = Cast<AMonsterAIController>(AI);
		if (MonsterAI)
		{
			MonsterAI->StopAI();
		}
	}

	// Drop items before returning to pool
	DropItems();

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPool(this);
		}
	}
}

void AMonsterBase::ReturnPool()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// Stop AI
	AAIController* AI = Cast<AAIController>(GetController());
	if (AI)
	{
		AMonsterAIController* MonsterAI = Cast<AMonsterAIController>(AI);
		if (MonsterAI)
		{
			MonsterAI->StopAI();
		}
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

// Sets default values
AMonsterBase::AMonsterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;	//Replicate setting
	SetReplicateMovement(true);	//sync with position

	AIControllerClass = AMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Spawned;

	// Create MonsterStatComponent
	MonsterStatComp = CreateDefaultSubobject<UMonsterStatComponent>(TEXT("MonsterStatComponent"));

	// Default fixed value settings
	DashSpeed = 600.f;
	DashDistance = 600.f;
	AttackRange = 1500.f;
	ProjectileSpeed = 1000.f;
	DropChance = 0.5f;
	MinDropCount = 1;
	MaxDropCount = 3;



	GetCharacterMovement()->MaxWalkSpeed = MonsterStatComp->GetMoveSpeed();

}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	// Subscribe to death event (only on server)
	if (HasAuthority() && MonsterStatComp)
	{
		MonsterStatComp->OnDeath.BindDynamic(this, &AMonsterBase::DeSpawn);
	}
}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	MonsterStatComp->ApplyDamage(DamageAmount);

	return DamageAmount;
}

void AMonsterBase::FireProjectile(AActor* Target)
{
	// Only execute on server
	if (!HasAuthority())
	{
		LOG_LOGIC_WARNING(TEXT("FireProjectile: No authority"));
		return;
	}

	// Validate target
	if (!Target)
	{
		LOG_LOGIC_WARNING(TEXT("FireProjectile: Target is null"));
		return;
	}

	// Get pooling subsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_LOGIC_ERROR(TEXT("FireProjectile: World is null"));
		return;
	}

	UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
	if (!PoolSys)
	{
		LOG_LOGIC_ERROR(TEXT("FireProjectile: PoolingSubsystem not found"));
		return;
	}

	// Calculate spawn location (slightly in front of monster)
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
	SpawnLocation.Z += 50.0f;  // Spawn at chest height

	// Get damage from MonsterStatComponent
	float Damage = MonsterStatComp ? MonsterStatComp->GetDamage() : 10.0f;

	// Delegate projectile firing to subsystem
	PoolSys->FireProjectileAt(this, Target, SpawnLocation, ProjectileSpeed, Damage);
}

void AMonsterBase::OnDeath(AActor* DeadMonster)
{
	if (!HasAuthority())
		return;

	// Drop items before the monster is deactivated
	DropItems();
}

void AMonsterBase::DropItems()
{
	if (!HasAuthority())
		return;

	// Check drop chance
	float RandomValue = FMath::FRand();
	if (RandomValue > DropChance)
		return;

	// Determine number of items to drop
	int32 DropCount = FMath::RandRange(MinDropCount, MaxDropCount);

	FVector DropLocation = GetActorLocation();
	DropLocation.Z += 50.f; // Slightly above ground

	// Get pooling subsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_LOGIC_ERROR(TEXT("DropItems: World is null"));
		return;
	}

	UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
	if (PoolSys)
	{
		PoolSys->SpawnItemDrop(DropLocation, DropCount);
		LOG_LOGIC_INFO(TEXT("Monster %s dropped %d items from pool"), *GetName(), DropCount);
	}
	else
	{
		LOG_LOGIC_ERROR(TEXT("DropItems: PoolingSubsystem not found"));
	}
}

