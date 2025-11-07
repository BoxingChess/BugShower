// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BSAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBSAnimInstance::UBSAnimInstance()
{
	MovingThreshould = 3.0f;
	JumpingThreshould = 0.f;
}

void UBSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//현재 AnimInstance를 소유하고 이는 액터 정보를 얻어올수 있다 하지만 이것은 액터타입으로 변환되기에 캐릭터인지 알수 X -> 따라서 형변환 시켜준다.
	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UBSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//프레임마다 업데이트된다.
	if (Movement)
	{
		Velocity = Movement->Velocity;
		//땅에서 얼마나 빨리 움직이는지? Velocity는 (x,y,z)로 구성되어있는데 x-앞,뒤, y-좌,우, z-위,아래 /size2D - 이름에서 알수있듯 xy평면에서의 속도를 반환하는 함수
		GroundSpeed = Velocity.Size2D();

		bIsIdle = GroundSpeed < MovingThreshould;


		bIsJumping = (Velocity.Z > JumpingThreshould);

		bIsFalling = Movement->IsFalling();
		//UE_LOG(LogTemp, Warning, TEXT("bIsJumping(%d), bIsFalling(%d)"), bIsJumping, bIsFalling);

	}
}
