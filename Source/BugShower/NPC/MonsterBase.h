// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "NPC/Spawnable.h"
#include "NPC/PoolingType.h"
#include "MonsterBase.generated.h"

UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
	NORMAL UMETA(DisplayName = "Normal"),
	ELITE UMETA(DisplayName = "Elite"),
	BOSS UMETA(DisplayName = "Boss")
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged"),
	Flying UMETA(DisplayName = "Flying"),
};




// Base class for all monsters
UCLASS()
class BUGSHOWER_API AMonsterBase : public ACharacter, public ISpawnable
{
	GENERATED_BODY()

	// Interface functions
public:
	virtual EPoolType GetPoolType() const override { return EPoolType::Monster; }
	virtual void Spawn(const FVector pos) override;
	virtual UPrimitiveComponent* GetPrimaryRenderComponent() override { return GetMesh(); }

	UFUNCTION()
	virtual void DeSpawn() override;

	virtual void ReturnPool() override;

public:
	// Sets default values for this character's properties
	AMonsterBase();

	//fixed value for monster stats
	UPROPERTY(EditAnywhere, Category = "MeleeStat")
	float DashSpeed;
	UPROPERTY(EditAnywhere, Category = "MeleeStat")
	float DashDistance;

	// Ranged attack settings
	UPROPERTY(EditAnywhere, Category = "RangedStat")
	float AttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RangedStat")
	float MinAttackRange;

	UPROPERTY(EditAnywhere, Category = "RangedStat")
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, Category = "RangedStat")
	TSubclassOf<AActor> BulletClass;

	// Drop configuration ID (references DataTable row)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DropStat")
	FName MonsterDropID;

	// Active gameplay tags for conditional drops (quest, event, etc.)
	UPROPERTY(BlueprintReadWrite, Category = "DropStat")
	FGameplayTagContainer ActiveDropConditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseStat")
	EMonsterGrade Grade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseStat")
	EAttackType Type;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	uint8 bIsDashing : 1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


	// Get MonsterStatComponent
	UFUNCTION(BlueprintCallable, Category = "Stat")
	class UMonsterStatComponent* GetMonsterStatComponent() const { return MonsterStatComp; }

	// Fire projectile towards target (called from BT Task)
	void FireProjectile(AActor* Target);

	void StartDash();
	void StopDash();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnDeath(AActor* DeadMonster);


protected:
	// Projectile class to spawn - set in Blueprint
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<class AMonsterProjectile> ProjectileClass;

	void DropItems();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UMonsterStatComponent* MonsterStatComp;
};