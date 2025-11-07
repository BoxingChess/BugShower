// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterBase.h"
#include "NPC/MonsterAIController.h"
#include "Net/UnrealNetwork.h"
#include "NPC/MonsterStatComponent.h"
#include "Item/ItemBase.h"
#include "Logging/BugShowerLog.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Projectile/MonsterProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/Spawnable.h"

#include "NPC/Pooling.h"


void AMonsterBase::InitState(AActor* InOwningSpawnPool)
{
	//set inactive state
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	//set owning pool
	OwningPool = InOwningSpawnPool;

	// Subscribe to death event
	MonsterStatComp->OnDeath.BindDynamic(this, &AMonsterBase::DeSpawn);
}

void AMonsterBase::Spawn(const FVector pos)
{
	SetActorLocation(pos);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

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

	if (!OwningPool.IsValid())
	{
		return;
	}

	APooling* PoolActor = Cast<APooling>(OwningPool);
	if (PoolActor)
	{
		PoolActor->ReturnPool(this);
	}

	DropItems();

	return;
}

void AMonsterBase::ReturnPool()
{
	if (!OwningPool.IsValid())
	{
		return;
	}

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

	APooling* PoolActor = Cast<APooling>(OwningPool);
	if (PoolActor)
	{
		PoolActor->ReturnPool(this);
		return;
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
		MonsterStatComp->OnMonsterDeath.AddDynamic(this, &AMonsterBase::OnDeath);
	}
}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MonsterStatComp->ApplyDamage(DeltaTime);
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

	// Validate projectile class
	if (!ProjectileClass)
	{
		LOG_LOGIC_ERROR(TEXT("FireProjectile: ProjectileClass is not set"));
		return;
	}

	// Validate target
	if (!Target)
	{
		LOG_LOGIC_WARNING(TEXT("FireProjectile: Target is null"));
		return;
	}

	// Calculate spawn location (slightly in front of monster)
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
	SpawnLocation.Z += 50.0f;  // Spawn at chest height

	// Calculate target location
	FVector TargetLocation = Target->GetActorLocation();

	// Calculate arc velocity (grenade-like trajectory)
	FVector LaunchVelocity;

	// Use standard SuggestProjectileVelocity with high arc for grenade-like effect
	bool bHaveArc = UGameplayStatics::SuggestProjectileVelocity(
		this,
		LaunchVelocity,
		SpawnLocation,
		TargetLocation,
		ProjectileSpeed,  // Use projectile speed from MonsterBase
		true,  // use high arc  default is false
		0.0f,   // No collision radius
		0.0f,   // Override gravity Z (0 = use world gravity)
		ESuggestProjVelocityTraceOption::DoNotTrace  // Don't trace for obstacles
	);

	// If arc calculation succeeded, modify velocity for very high arc trajectory
	if (bHaveArc)
	{
		LOG_LOGIC_INFO(TEXT("FireProjectile: Calculated high-arc mortar trajectory"));
	}
	else
	{
		LOG_LOGIC_WARNING(TEXT("FireProjectile: Could not calculate arc trajectory, target may be unreachable"));
		return;  // Don't fire if we can't reach the target
	}

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMonsterProjectile* Projectile = GetWorld()->SpawnActor<AMonsterProjectile>(
		ProjectileClass,
		SpawnLocation,
		LaunchVelocity.Rotation(),
		SpawnParams
	);

	if (Projectile)
	{
		// Get damage from MonsterStatComponent
		float Damage = MonsterStatComp ? MonsterStatComp->GetDamage() : 10.0f;

		// Initialize projectile with calculated arc velocity
		Projectile->InitializeProjectileWithVelocity(LaunchVelocity, Damage, this);

		LOG_LOGIC_INFO(TEXT("Monster %s fired projectile at %s with arc trajectory"), *GetName(), *Target->GetName());
	}
	else
	{
		LOG_LOGIC_ERROR(TEXT("FireProjectile: Failed to spawn projectile"));
	}
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

	//if (DropTable.Num() == 0)
	//	return;

	//// Check drop chance
	//float RandomValue = FMath::FRand();
	//if (RandomValue > DropChance)
	//	return;

	//// Determine number of items to drop
	//int32 DropCount = FMath::RandRange(MinDropCount, MaxDropCount);
	//DropCount = FMath::Min(DropCount, DropTable.Num());

	FVector DropLocation = GetActorLocation();
	DropLocation.Z += 50.f; // Slightly above ground
	FRotator DropRotation = FRotator::ZeroRotator;


	APooling* PoolActor = Cast<APooling>(OwningPool);
	if (PoolActor)
	{
		PoolActor->Spawn(EPoolType::Item, DropLocation);
		LOG_LOGIC_INFO(TEXT("Monster %s dropped item from pool"), *GetName());
	}

	//for (int32 i = 0; i < DropCount; i++)
	//{
	//	// Select random item from drop table
	//	int32 RandomIndex = FMath::RandRange(0, DropTable.Num() - 1);
	//	TSubclassOf<AItemBase> ItemClass = DropTable[RandomIndex];

	//	if (ItemClass)
	//	{
	//		// Add random offset to avoid items stacking
	//		FVector Offset = FVector(
	//			FMath::RandRange(-100.f, 100.f),
	//			FMath::RandRange(-100.f, 100.f),
	//			50.f
	//		);

	//		FActorSpawnParameters SpawnParams;
	//		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	//		
	//		
	//	}
	//}
}

