// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
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

	UFUNCTION()
	virtual void DeSpawn() override;

	virtual void ReturnPool() override;

protected:
public:
	// Sets default values for this character's properties
	AMonsterBase();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|BaseStat")
	EMonsterGrade Grade;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|BaseStat")
	EAttackType Type;

	// Drop configuration ID (references DataTable row)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|DropStat")
	FName MonsterDropID;

	// Active gameplay tags for conditional drops (quest, event, etc.)
	UPROPERTY(BlueprintReadWrite, Category = "Monster|DropStat")
	FGameplayTagContainer ActiveDropConditions;

	//fixed value for monster stats
	UPROPERTY(EditAnywhere, Category = "Monster|MeleeStat", meta = (EditCondition = "Type == EAttackType::Melee", EditConditionHides))
	float DashSpeed;
	UPROPERTY(EditAnywhere, Category = "Monster|MeleeStat", meta = (EditCondition = "Type == EAttackType::Melee", EditConditionHides))
	float DashDistance;

	// Ranged attack settings
	UPROPERTY(EditAnywhere, Category = "Monster|RangedStat", meta = (EditCondition = "Type == EAttackType::Ranged", EditConditionHides))
	float AttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|RangedStat",meta = (EditCondition = "Type == EAttackType::Ranged", EditConditionHides))
	float MinAttackRange;

	UPROPERTY(EditAnywhere, Category = "Monster|RangedStat",meta = (EditCondition = "Type == EAttackType::Ranged", EditConditionHides))
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, Category = "Monster|RangedStat", meta = (EditCondition = "Type == EAttackType::Ranged", EditConditionHides))
	TSubclassOf<AActor> BulletClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


	// Get MonsterStatComponent
	UFUNCTION(BlueprintCallable, Category = "Stat")
	class UMonsterStatComponent* GetMonsterStatComponent() const { return MonsterStatComp; }

	// Fire projectile towards target (called from BT Task)
	void FireProjectile(AActor* Target);

	// Drop item on death
	UFUNCTION()
	void OnDeath(AActor* DeadMonster);


protected:
	// Drop items at monster location
	void DropItems();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UMonsterStatComponent* MonsterStatComp;

	//need animation comp
};