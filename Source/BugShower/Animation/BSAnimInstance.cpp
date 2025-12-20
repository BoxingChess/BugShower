// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BSAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/BSCharacterPlayer.h"
#include "Weapon/WeaponActor.h"
#include "Weapon/Component/WeaponComponent.h"

UBSAnimInstance::UBSAnimInstance()
{
	MovingThreshould = 3.0f;
	JumpingThreshould = 0.f;
	SprintingThreshold = 600.f; // Walk(300) 이상의 속도면 Sprint로 판단
	bIsArmed = false;
	bIsFiring = false;
	bIsSprinting = false;
}

void UBSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//���� AnimInstance�� �����ϰ� �̴� ���� ������ ���ü� �ִ� ������ �̰��� ����Ÿ������ ��ȯ�Ǳ⿡ ĳ�������� �˼� X -> ���� ����ȯ �����ش�.
	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UBSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//�����Ӹ��� ������Ʈ�ȴ�.
	if (Movement)
	{
		Velocity = Movement->Velocity;
		//������ �󸶳� ���� �����̴���? Velocity�� (x,y,z)�� �����Ǿ��ִµ� x-��,��, y-��,��, z-��,�Ʒ� /size2D - �̸����� �˼��ֵ� xy��鿡���� �ӵ��� ��ȯ�ϴ� �Լ�
		GroundSpeed = Velocity.Size2D();

		bIsIdle = GroundSpeed < MovingThreshould;
		bIsSprinting = GroundSpeed > SprintingThreshold;

		bool bPreviousJumping = bIsJumping;
		bool bPreviousFalling = bIsFalling;

		bIsJumping = (Velocity.Z > JumpingThreshould);

		bIsFalling = Movement->IsFalling();

		// 점프/낙하 상태가 변경되었을 때만 로그 출력
		if (bPreviousJumping != bIsJumping || bPreviousFalling != bIsFalling)
		{
			UE_LOG(LogTemp, Error, TEXT("[ANIM] *** JUMP STATE CHANGE *** bIsJumping: %d, bIsFalling: %d, VelocityZ: %.2f"),
				bIsJumping, bIsFalling, Velocity.Z);
		}
	}

	// 무기 장착 상태를 Owner(BSCharacterPlayer)로부터 가져오기
	if (Owner)
	{
		if (ABSCharacterPlayer* PlayerOwner = Cast<ABSCharacterPlayer>(Owner))
		{
			bIsArmed = PlayerOwner->GetIsArmed();

			// 발사 중 여부를 WeaponComponent로부터 가져오기
			if (AWeaponActor* CurrentWeapon = PlayerOwner->GetCurrentWeapon())
			{
				if (UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent())
				{
					bIsFiring = WeaponComp->IsFiring();
				}
				else
				{
					bIsFiring = false;
				}
			}
			else
			{
				bIsFiring = false;
			}
		}
	}
}

// ========================================
// 발사 애니메이션 재생
// ========================================

void UBSAnimInstance::PlayFireMontage()
{
	if (FireMontage)
	{
		Montage_Play(FireMontage);
		UE_LOG(LogTemp, Log, TEXT("BSAnimInstance - Playing Fire Montage"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BSAnimInstance - FireMontage is not set!"));
	}
}

void UBSAnimInstance::PlayReloadMontage()
{
	if (ReloadMontage)
	{
		Montage_Play(ReloadMontage);
		UE_LOG(LogTemp, Log, TEXT("BSAnimInstance - Playing Reload Montage"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BSAnimInstance - ReloadMontage is not set!"));
	}
}
