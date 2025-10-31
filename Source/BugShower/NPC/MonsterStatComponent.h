// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterStatComponent.generated.h"

UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
	NORMAL UMETA(DisplayName = "Normal"),
	ELITE UMETA(DisplayName = "Elite"),
	BOSS UMETA(DisplayName = "Boss")
};

// Delegate for death event (passes the monster actor as parameter)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterDeath, AActor*, DeadMonster);

// Delegate for HP changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);



//management of monster stats(hp,death...)
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUGSHOWER_API UMonsterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMonsterStatComponent();

	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Stat accessors
	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetCurrentHP() const { return CurHP; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetHPRatio() const { return MaxHP > 0 ? CurHP / MaxHP : 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetDamage() const { return Damage; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetDefense() const { return Defense; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	FString GetMonsterName() const { return Name; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	EMonsterGrade GetGrade() const { return Grade; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool IsDead() const { return CurHP <= 0.f; }

	// Stat modifiers (server only)
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void SetCurrentHP(float NewHP);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void SetMaxHP(float NewMaxHP);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ResetHP();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void InitStats(FString InName, EMonsterGrade InGrade, float InMaxHP, float InDamage, float InDefense);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnMonsterDeath OnMonsterDeath;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_ChangeHP();

	void CheckDeath();

	//stat
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat")
	FString Name;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat")
	EMonsterGrade Grade;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeHP, EditAnywhere, Category = "Stat")
	float CurHP;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat")
	float MaxHP;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat")
	float Damage;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat")
	float Defense;
};
