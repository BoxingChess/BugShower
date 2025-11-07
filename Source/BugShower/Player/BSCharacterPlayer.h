// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/BSCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "BSCharacterTypes.h"

#include "BSCharacterPlayer.generated.h"

class UMovementInputComponent;
class UUI_InGameComponent;
class UPickUpDetectorComponent;
class UInventoryComponent;

/*
 �� Ŭ������ ABSCharacterBase�� ��ӹ޴� �÷��̾� ĳ���� Ŭ�����̴�.
 */
UCLASS()
class BUGSHOWER_API ABSCharacterPlayer : public ABSCharacterBase
{
	GENERATED_BODY()
	
public:
	ABSCharacterPlayer();

/*
�𸮾� �������� �÷��̾��� �Է��� ó���ϱ� ���� �������̵� �ؾ��ϴ� �Լ�
Ű���峪 �е��� �Է��� ���� �����ڵ忡 �����ϴ� �ٽ� �Լ�
*/
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

///�ش� �÷��̾ ������ �ִ� Ŀ���� ������Ʈ��
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UMovementInputComponent> MovementComponentOnGround;

	//UI�� �����ϴ� ���� ���� �÷��̾ �ƴ� �ܺ��� UIComponent���� �����ϰԲ� �� ���̴�.
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	//TObjectPtr<UUI_InGameComponent> UIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPickUpDetectorComponent> PickUpDetectorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

protected:
	// Camera boom that follows the character (spring arm component)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	// Third person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> ThirdPersonCamera;

	// First person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FirstPersonCamera;

public:
	// Camera system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	ECameraViewMode CurrentViewMode;

	ECharacterState CharacterState;

	void SetCurrentViewMode(ECameraViewMode NewMode);
	void SetCharacterState(ECharacterState _newState);

	ECameraViewMode GetCurrentViewMode();
	ECharacterState GetCharacterState();

	void ToggleViewMode();
	void SetCameraViewMode(ECameraViewMode NewMode);

	void StartThirdPersonZoom();
	void EndThirdPersonZoom();

public:
	// Called when this character is possessed/unpossessed by a controller
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

public:
	// Fire action - will be moved to weapon component later
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> FireAction;

	// Weapon firing
public:
	void Fire();

};
