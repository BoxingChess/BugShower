// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterBaseAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UMonsterBaseAnimInstance::UMonsterBaseAnimInstance() : MovingThreshould(3), DashThreshould(5), GroundSpeed(0), Velocity(FVector{ 0,0,0 }), bIsIdle(true), bIsFalling(false), bIsDash(false)
{

}

void UMonsterBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UMonsterBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Movement)
	{
		Velocity = Movement->Velocity;
		GroundSpeed = Velocity.Size2D();
		bIsIdle = GroundSpeed < MovingThreshould;
		bIsFalling = Movement->IsFalling();
	}
}
