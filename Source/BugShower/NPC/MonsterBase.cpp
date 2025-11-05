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
	RangedAttackRange = 1500.f;
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

	// Calculate direction to target
	FVector TargetLocation = Target->GetActorLocation();
	FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMonsterProjectile* Projectile = GetWorld()->SpawnActor<AMonsterProjectile>(
		ProjectileClass,
		SpawnLocation,
		Direction.Rotation(),
		SpawnParams
	);

	if (Projectile)
	{
		// Get damage from MonsterStatComponent
		float Damage = MonsterStatComp ? MonsterStatComp->GetDamage() : 10.0f;

		// Initialize projectile
		Projectile->InitializeProjectile(Direction, Damage, this);

		LOG_LOGIC_INFO(TEXT("Monster %s fired projectile at %s"), *GetName(), *Target->GetName());
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

	if (DropTable.Num() == 0)
		return;

	// Check drop chance
	float RandomValue = FMath::FRand();
	if (RandomValue > DropChance)
		return;

	// Determine number of items to drop
	int32 DropCount = FMath::RandRange(MinDropCount, MaxDropCount);
	DropCount = FMath::Min(DropCount, DropTable.Num());

	FVector DropLocation = GetActorLocation();
	FRotator DropRotation = FRotator::ZeroRotator;

	for (int32 i = 0; i < DropCount; i++)
	{
		// Select random item from drop table
		int32 RandomIndex = FMath::RandRange(0, DropTable.Num() - 1);
		TSubclassOf<AItemBase> ItemClass = DropTable[RandomIndex];

		if (ItemClass)
		{
			// Add random offset to avoid items stacking
			FVector Offset = FVector(
				FMath::RandRange(-100.f, 100.f),
				FMath::RandRange(-100.f, 100.f),
				50.f
			);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AItemBase* DroppedItem = GetWorld()->SpawnActor<AItemBase>(ItemClass, DropLocation + Offset, DropRotation, SpawnParams);
			if (DroppedItem)
			{
				LOG_LOGIC_INFO(TEXT("Monster %s dropped item: %s"), *GetName(), *DroppedItem->ItemName);
			}
		}
	}
}

