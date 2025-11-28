// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatComponent.generated.h"

// Delegate for HP changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHPChanged, float, CurrentHP, float, MaxHP);

// Delegate for Stamina changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStaminaChanged, float, CurrentStamina, float, MaxStamina);

// Delegate for player death
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

// Delegate for level up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelUp, int32, NewLevel, int32, StatPoints);


/**
 * 플레이어 캐릭터의 스탯을 관리하는 컴포넌트
 * HP, 스태미나, 이동 속도, 점프 등 플레이어의 모든 수치를 관리합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUGSHOWER_API UPlayerStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// ========================================
	// HP 시스템
	// ========================================
public:
	// HP Getters
	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	float GetHPRatio() const { return MaxHP > 0 ? CurrentHP / MaxHP : 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	bool IsDead() const { return CurrentHP <= 0.f; }

	// HP Modifiers (Server Only)
	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	void SetCurrentHP(float NewHP);

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	void SetMaxHP(float NewMaxHP);

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Stat|HP")
	void ResetHP();

private:
	UPROPERTY(ReplicatedUsing = OnRep_HPChanged, EditAnywhere, Category = "Stat|HP")
	float CurrentHP = 100.f;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|HP")
	float MaxHP = 100.f;

	UFUNCTION()
	void OnRep_HPChanged();

	void CheckDeath();

	// ========================================
	// 스태미나 시스템
	// ========================================
public:
	// Stamina Getters
	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	float GetStaminaRatio() const { return MaxStamina > 0 ? CurrentStamina / MaxStamina : 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	bool HasEnoughStamina(float Amount) const { return CurrentStamina >= Amount; }

	// Stamina Modifiers
	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	bool UseStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	void RecoverStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat|Stamina")
	void SetStaminaRecoveryRate(float Rate) { StaminaRecoveryRate = Rate; }

private:
	UPROPERTY(ReplicatedUsing = OnRep_StaminaChanged, EditAnywhere, Category = "Stat|Stamina")
	float CurrentStamina = 100.f;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Stamina")
	float MaxStamina = 100.f;

	// 초당 회복량
	UPROPERTY(EditAnywhere, Category = "Stat|Stamina")
	float StaminaRecoveryRate = 10.f;

	// 스태미나 회복 대기 시간 (사용 후 이 시간 뒤에 회복 시작)
	UPROPERTY(EditAnywhere, Category = "Stat|Stamina")
	float StaminaRecoveryDelay = 2.f;

	float TimeSinceLastStaminaUse = 0.f;

	UFUNCTION()
	void OnRep_StaminaChanged();

	// ========================================
	// 이동 관련 스탯
	// ========================================
public:
	UFUNCTION(BlueprintCallable, Category = "Stat|Movement")
	float GetWalkSpeed() const { return WalkSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Movement")
	float GetSprintSpeed() const { return SprintSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Movement")
	float GetCrouchSpeed() const { return CrouchSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Movement")
	void SetWalkSpeed(float NewSpeed) { WalkSpeed = NewSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Movement")
	void SetSprintSpeed(float NewSpeed) { SprintSpeed = NewSpeed; }

private:
	// 기본 걷기 속도
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Movement")
	float WalkSpeed = 600.f;

	// 달리기 속도
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Movement")
	float SprintSpeed = 900.f;

	// 앉은 상태 속도
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Movement")
	float CrouchSpeed = 300.f;

	// ========================================
	// 점프 관련 스탯
	// ========================================
public:
	UFUNCTION(BlueprintCallable, Category = "Stat|Jump")
	int32 GetMaxJumpCount() const { return MaxJumpCount; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Jump")
	float GetJumpPower() const { return JumpPower; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Jump")
	void SetMaxJumpCount(int32 NewCount) { MaxJumpCount = FMath::Max(1, NewCount); }

	UFUNCTION(BlueprintCallable, Category = "Stat|Jump")
	void SetJumpPower(float NewPower) { JumpPower = NewPower; }

private:
	// 최대 점프 횟수 (1 = 일반 점프, 2 = 더블 점프)
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Jump")
	int32 MaxJumpCount = 2;

	// 점프력 (CharacterMovementComponent의 JumpZVelocity)
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Jump")
	float JumpPower = 600.f;

	// ========================================
	// 전투 스탯
	// ========================================
public:
	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	float GetDamage() const { return Damage; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	float GetDefense() const { return Defense; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	void SetDamage(float NewDamage) { Damage = NewDamage; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	void SetDefense(float NewDefense) { Defense = NewDefense; }

private:
	// 공격력
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Combat")
	float Damage = 10.f;

	// 방어력 (받는 데미지 감소)
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Combat")
	float Defense = 5.f;

	// ========================================
	// 레벨 & 경험치 시스템 (향후 확장용)
	// ========================================
public:
	UFUNCTION(BlueprintCallable, Category = "Stat|Level")
	int32 GetLevel() const { return Level; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Level")
	int32 GetExperience() const { return Experience; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Level")
	int32 GetExperienceToNextLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Stat|Level")
	void GainExperience(int32 Amount);

private:
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Level")
	int32 Level = 1;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Level")
	int32 Experience = 0;

	void LevelUp();

	// ========================================
	// 델리게이트 (이벤트)
	// ========================================
public:
	// HP 변경 시 (UI 업데이트용)
	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnPlayerHPChanged OnHPChanged;

	// 스태미나 변경 시 (UI 업데이트용)
	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnPlayerStaminaChanged OnStaminaChanged;

	// 플레이어 사망 시
	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnPlayerDeath OnPlayerDeath;

	// 레벨업 시
	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnPlayerLevelUp OnLevelUp;

	// ========================================
	// 초기화
	// ========================================
public:
	// 모든 스탯 초기화
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void InitializeStats(float InMaxHP, float InMaxStamina, float InWalkSpeed,
	                     float InSprintSpeed, int32 InMaxJumpCount, float InJumpPower);

	// 스탯 리셋 (사망 후 부활 시)
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ResetAllStats();
};
