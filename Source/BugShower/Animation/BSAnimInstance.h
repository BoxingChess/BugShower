// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BSAnimInstance.generated.h"

/**
 * 
 */

UCLASS()
class BUGSHOWER_API UBSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UBSAnimInstance();

protected:
	//AnimInstance�� ó�� �����ɶ� �ѹ��� ȣ���� �ȴ�.
	virtual void NativeInitializeAnimation() override;

	//�� �����Ӹ��� ȣ��ȴ�.
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;	// �� �ΰ��� �Լ��� ����Ͽ� �츮�� �ִ� �׷������� ������ ���������� �����ϵ��� �ؾ��Ѵ�.

	///�ִ� �׷����� �����ؾ��� ������--------------------------------

	//�� �ν��Ͻ��� �����ϰ� �ִ� ĳ������ ������ ��� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class ACharacter> Owner;
	
	//ĳ���� Movement ������Ʈ�� ���� ��ü �����͸� ��� ���� 
	//�� �����Ӹ��� Owner->GetCharacterMovement()�̷��� ȣ���ϴ� �ͺ��� �ּҸ� �����صΰ� ���� ������ �������°� �� ȿ�����̴ϱ�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UCharacterMovementComponent> Movement;

	//ĳ���� �ӵ� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FVector Velocity;

	//�������� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float GroundSpeed;

	//���� idle �����ΰ�?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsIdle : 1;

	//���� �����̰��ִ��� �����ִ���?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float MovingThreshould;

	//���� falling �����ΰ�?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsFalling : 1;

	//���� Jumping �����ΰ�?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsJumping : 1;

	//���� �������ΰ�?�� �Ǵ��Ҷ� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float JumpingThreshould;

	//총기를 들고 있는지 여부 (StatComponent에서 읽어옴)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	uint8 bIsArmed : 1;

	//발사 중인지 여부 (WeaponComponent에서 읽어옴)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	uint8 bIsFiring : 1;

	//현재 전력질주 중인지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsSprinting : 1;

	//전력질주 판단 임계값 (이 속도 이상이면 Sprint)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float SprintingThreshold;

	// ========================================
	// 발사 애니메이션
	// ========================================
public:
	// 발사 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
	TObjectPtr<class UAnimMontage> FireMontage;

	// 재장전 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
	TObjectPtr<class UAnimMontage> ReloadMontage;

	// 발사 애니메이션 재생 (WeaponComponent에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
	void PlayFireMontage();

	// 재장전 애니메이션 재생 (WeaponComponent에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Animation")
	void PlayReloadMontage();
};

