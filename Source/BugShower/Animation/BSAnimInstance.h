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
	//AnimInstance가 처음 생성될때 한번만 호출이 된다.
	virtual void NativeInitializeAnimation() override;

	//매 프레임마다 호출된다.
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;	// 이 두가지 함수를 사용하여 우리가 애님 그래프에서 참조할 변수값들을 저장하도록 해야한다.

	///애님 그래프가 참조해야할 변수들--------------------------------

	//이 인스턴스를 소유하고 있는 캐릭터의 정보를 담는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class ACharacter> Owner;
	
	//캐릭터 Movement 컴포넌트에 대한 객체 포인터를 담는 변수 
	//매 프레임마다 Owner->GetCharacterMovement()이렇게 호출하는 것보다 주소를 저장해두고 값만 빠르게 가져오는게 더 효율적이니까.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UCharacterMovementComponent> Movement;

	//캐릭터 속도 보간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FVector Velocity;

	//땅에서의 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float GroundSpeed;

	//현재 idle 상태인가?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsIdle : 1;

	//현재 움직이고있는지 쉬고있는지?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float MovingThreshould;

	//현재 falling 상태인가?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsFalling : 1;

	//현재 Jumping 상태인가?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsJumping : 1;

	//현재 점프중인가?를 판단할때 쓰는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float JumpingThreshould;
};

