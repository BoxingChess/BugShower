// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterBaseAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UMonsterBaseAnimInstance::UMonsterBaseAnimInstance() : MovingThreshould(3), GroundSpeed(0), Velocity(FVector{ 0,0,0 }), bIsIdle(true), bIsWalk(false), bIsDash(false), bIsFalling(false)
{

}

void UMonsterBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ACharacter* Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		Movement = Character->GetCharacterMovement();
	}
}

void UMonsterBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Movement) return;

	AMonsterBase* Monster = Cast<AMonsterBase>(GetOwningActor());
	if (!Monster) return;

	Velocity = Movement->Velocity;
	GroundSpeed = Velocity.Size2D();
	bIsDash = Monster->bIsDashing;
	bIsWalk = !bIsDash && GroundSpeed > MovingThreshould;
	bIsIdle = !bIsDash && !bIsWalk;
	bIsFalling = Movement->IsFalling();
}
