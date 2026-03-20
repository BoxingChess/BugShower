// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatComponent.generated.h"

// Delegate for HP changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHPChanged, float, CurrentHP, float, MaxHP);

// Delegate for Abla Particle changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerAblaParticleChanged, float, CurrentAblaParticle, float, MaxAblaParticle);

// Delegate for player death
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

// Delegate for level up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelUp, int32, NewLevel, int32, StatPoints);


/**
 * 플레이어 캐릭터의 스탯을 관리하는 컴포넌트
 * HP, 에블라 입자, 이동 속도, 점프 등 플레이어의 모든 수치를 관리합니다.
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
	// 에블라 입자 시스템
	// ========================================
public:
	// Abla Particle Getters
	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	float GetCurrentAblaParticle() const { return CurrentAblaParticle; }

	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	float GetMaxAblaParticle() const { return MaxAblaParticle; }

	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	float GetAblaParticleRatio() const { return MaxAblaParticle > 0 ? CurrentAblaParticle / MaxAblaParticle : 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	bool HasAblaParticle(float Amount) const { return CurrentAblaParticle >= Amount; }

	// Abla Particle Modifiers
	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	bool ConsumeAblaParticle(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	void GainAblaParticle(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat|AblaParticle")
	void SetAblaParticleGainRate(float Rate) { AblaParticleGainRate = Rate; }

private:
	UPROPERTY(ReplicatedUsing = OnRep_AblaParticleChanged, EditAnywhere, Category = "Stat|AblaParticle")
	float CurrentAblaParticle = 0.f;

	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|AblaParticle")
	float MaxAblaParticle = 100.f;

	// 초당 증가량 (체내에 쌓이는 속도)
	UPROPERTY(EditAnywhere, Category = "Stat|AblaParticle")
	float AblaParticleGainRate = 0.1f;

	UFUNCTION()
	void OnRep_AblaParticleChanged();

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
	float WalkSpeed = 50.f;  // Walk 최대 속도 (Shift 누르지 않았을 때)

	// 달리기 속도
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Movement")
	float SprintSpeed = 300.f;  // Sprint 최대 속도 (Shift 눌렀을 때)

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

	// 무기 장착 상태
	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	bool IsArmed() const { return bIsArmed; }

	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	void SetIsArmed(bool bNewIsArmed);

private:
	// 공격력
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Combat")
	float Damage = 10.f;

	// 방어력 (받는 데미지 감소)
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Combat")
	float Defense = 5.f;

	// 무기 장착 여부
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat|Combat")
	uint8 bIsArmed : 1;

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

	// 에블라 입자 변경 시 (UI 업데이트용)
	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnPlayerAblaParticleChanged OnAblaParticleChanged;

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
	void InitializeStats(float InMaxHP, float InMaxAblaParticle, float InWalkSpeed,
	                     float InSprintSpeed, int32 InMaxJumpCount, float InJumpPower);

	// 스탯 리셋 (사망 후 부활 시)
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ResetAllStats();
};
