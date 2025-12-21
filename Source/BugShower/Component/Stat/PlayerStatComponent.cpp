// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Stat/PlayerStatComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerStatComponent::UPlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// 기본값 설정
	CurrentHP = 100.f;
	MaxHP = 100.f;

	CurrentStamina = 100.f;
	MaxStamina = 100.f;
	StaminaRecoveryRate = 10.f;
	StaminaRecoveryDelay = 2.f;

	WalkSpeed = 600.f;
	SprintSpeed = 900.f;
	CrouchSpeed = 300.f;

	MaxJumpCount = 2;
	JumpPower = 600.f;

	Damage = 10.f;
	Defense = 5.f;

	bIsArmed = false;

	Level = 1;
	Experience = 0;
}

void UPlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// HP와 스태미나를 최대치로 초기화
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;

	// Owner의 CharacterMovementComponent에 이동 속도 적용
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = WalkSpeed;
			MovementComp->JumpZVelocity = JumpPower;
			UE_LOG(LogTemp, Warning, TEXT("PlayerStatComponent - MaxWalkSpeed initialized to %.1f"), WalkSpeed);
		}
	}
}

void UPlayerStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 스태미나 자동 회복 (서버에서만)
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (CurrentStamina < MaxStamina)
		{
			TimeSinceLastStaminaUse += DeltaTime;

			// 일정 시간 후 스태미나 회복 시작
			if (TimeSinceLastStaminaUse >= StaminaRecoveryDelay)
			{
				RecoverStamina(StaminaRecoveryRate * DeltaTime);
			}
		}
	}
}

void UPlayerStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// HP 리플리케이션
	DOREPLIFETIME(UPlayerStatComponent, CurrentHP);
	DOREPLIFETIME(UPlayerStatComponent, MaxHP);

	// 스태미나 리플리케이션
	DOREPLIFETIME(UPlayerStatComponent, CurrentStamina);
	DOREPLIFETIME(UPlayerStatComponent, MaxStamina);

	// 이동 관련 스탯
	DOREPLIFETIME(UPlayerStatComponent, WalkSpeed);
	DOREPLIFETIME(UPlayerStatComponent, SprintSpeed);
	DOREPLIFETIME(UPlayerStatComponent, CrouchSpeed);

	// 점프 관련 스탯
	DOREPLIFETIME(UPlayerStatComponent, MaxJumpCount);
	DOREPLIFETIME(UPlayerStatComponent, JumpPower);

	// 전투 스탯
	DOREPLIFETIME(UPlayerStatComponent, Damage);
	DOREPLIFETIME(UPlayerStatComponent, Defense);
	DOREPLIFETIME(UPlayerStatComponent, bIsArmed);

	// 레벨 & 경험치
	DOREPLIFETIME(UPlayerStatComponent, Level);
	DOREPLIFETIME(UPlayerStatComponent, Experience);
}

// ========================================
// HP 시스템 구현
// ========================================

void UPlayerStatComponent::OnRep_HPChanged()
{
	// HP가 변경되면 델리게이트 브로드캐스트 (UI 업데이트용)
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UPlayerStatComponent::CheckDeath()
{
	if (CurrentHP <= 0.f)
	{
		CurrentHP = 0.f;

		// 서버에서만 사망 이벤트 브로드캐스트
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %s is Dead!"), *GetOwner()->GetName());
			OnPlayerDeath.Broadcast();
		}
	}
}

void UPlayerStatComponent::SetCurrentHP(float NewHP)
{
	// 서버 권한 체크
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	CurrentHP = FMath::Clamp(NewHP, 0.f, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	CheckDeath();
}

void UPlayerStatComponent::SetMaxHP(float NewMaxHP)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	MaxHP = FMath::Max(NewMaxHP, 1.f);
	CurrentHP = FMath::Min(CurrentHP, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UPlayerStatComponent::ApplyDamage(float DamageAmount)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	if (IsDead())
		return;

	// 방어력을 고려한 실제 데미지 계산
	float ActualDamage = FMath::Max(DamageAmount - Defense, 0.f);
	CurrentHP = FMath::Max(CurrentHP - ActualDamage, 0.f);

	UE_LOG(LogTemp, Log, TEXT("Player takes damage: %f (Actual: %f), CurrentHP: %f / %f"),
	       DamageAmount, ActualDamage, CurrentHP, MaxHP);

	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	CheckDeath();
}

void UPlayerStatComponent::Heal(float HealAmount)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	if (IsDead())
		return;

	CurrentHP = FMath::Min(CurrentHP + HealAmount, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	UE_LOG(LogTemp, Log, TEXT("Player healed: %f, CurrentHP: %f / %f"),
	       HealAmount, CurrentHP, MaxHP);
}

void UPlayerStatComponent::ResetHP()
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	CurrentHP = MaxHP;
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

// ========================================
// 스태미나 시스템 구현
// ========================================

void UPlayerStatComponent::OnRep_StaminaChanged()
{
	// 스태미나가 변경되면 델리게이트 브로드캐스트
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

bool UPlayerStatComponent::UseStamina(float Amount)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return false;

	if (CurrentStamina < Amount)
		return false; // 스태미나 부족

	CurrentStamina = FMath::Max(CurrentStamina - Amount, 0.f);
	TimeSinceLastStaminaUse = 0.f; // 회복 타이머 리셋

	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	return true;
}

void UPlayerStatComponent::RecoverStamina(float Amount)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	CurrentStamina = FMath::Min(CurrentStamina + Amount, MaxStamina);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

// ========================================
// 레벨 & 경험치 시스템
// ========================================

int32 UPlayerStatComponent::GetExperienceToNextLevel() const
{
	// 레벨업에 필요한 경험치 공식: Level * 100
	return Level * 100;
}

void UPlayerStatComponent::GainExperience(int32 Amount)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	Experience += Amount;

	UE_LOG(LogTemp, Log, TEXT("Player gained %d XP. Total: %d / %d"),
	       Amount, Experience, GetExperienceToNextLevel());

	// 레벨업 체크
	while (Experience >= GetExperienceToNextLevel())
	{
		Experience -= GetExperienceToNextLevel();
		LevelUp();
	}
}

void UPlayerStatComponent::LevelUp()
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	Level++;

	// 레벨업 시 스탯 증가
	MaxHP += 10.f;
	MaxStamina += 5.f;
	Damage += 2.f;
	Defense += 1.f;

	// HP와 스태미나 전체 회복
	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;

	UE_LOG(LogTemp, Warning, TEXT("Player leveled up to Level %d!"), Level);

	// 레벨업 이벤트 브로드캐스트 (스탯 포인트는 나중에 구현)
	OnLevelUp.Broadcast(Level, 0);

	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

// ========================================
// 초기화 함수
// ========================================

void UPlayerStatComponent::InitializeStats(float InMaxHP, float InMaxStamina, float InWalkSpeed,
                                            float InSprintSpeed, int32 InMaxJumpCount, float InJumpPower)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	// 스탯 설정
	MaxHP = InMaxHP;
	CurrentHP = InMaxHP;

	MaxStamina = InMaxStamina;
	CurrentStamina = InMaxStamina;

	WalkSpeed = InWalkSpeed;
	SprintSpeed = InSprintSpeed;

	MaxJumpCount = InMaxJumpCount;
	JumpPower = InJumpPower;

	// CharacterMovementComponent에 적용
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = WalkSpeed;
			MovementComp->JumpZVelocity = JumpPower;
		}
	}

	// UI 업데이트
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UPlayerStatComponent::ResetAllStats()
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	CurrentHP = MaxHP;
	CurrentStamina = MaxStamina;

	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

// ========================================
// 전투 스탯 구현
// ========================================

void UPlayerStatComponent::SetIsArmed(bool bNewIsArmed)
{
	// 서버 권한 체크
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	bIsArmed = bNewIsArmed;

	UE_LOG(LogTemp, Log, TEXT("PlayerStatComponent - SetIsArmed: %s"), bIsArmed ? TEXT("true") : TEXT("false"));
}
