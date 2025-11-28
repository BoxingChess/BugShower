// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Components/ActorComponent.h"
#include "Player/BSCharacterPlayer.h"
#include "Player/BSCharacterTypes.h"


#include "MovementInputComponent.generated.h"


//Minimize dependence through the declaration of independence
class UInputAction;
class ACharacter;
class AController;

// 인벤토리 토글 요청 이벤트
// Tap 키 입력 시 MovementInputComponent에서 브로드캐스트됨
// UI_InGameComponent 또는 InventoryComponent가 바인딩하여
// 인벤토리 창 열기/닫기 로직을 실행한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryToggleRequested);

//F키 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFKeyToggleRequested);

//ALT키 관련
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnALTKeyToggleRequested);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUGSHOWER_API UMovementInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMovementInputComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void RefreshControllerCache();

public:
	void FL_SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

private:
    // Owner Character/Controller Caching (GC Safe)
	UPROPERTY(Transient) 
	TWeakObjectPtr<ABSCharacterPlayer> OwnerPlayer = nullptr;
    UPROPERTY(Transient) 
	TWeakObjectPtr<AController> OwnerController = nullptr;

public:
	/*
	Input Action?
	"어떤 행동을 하겠다." ex - move, junp, dash
	*/
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> MoveAction;

	//Look Function InputAction.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> LookAction_Third;

	//Right mouse Click -> Can Change point of view
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> LookAction_RightClick;

	//Space for Can Jump and Double Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> JumpAction_Space;

	//left mouse push down For Can Player FastRun //TODO : 총발사 이후 구현.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> DashAction;


	// F Key for interaction/pickup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> FKeyAction;

	// Tab Key for inventory toggle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> TapKeyAction;

	// ALT Key for alternate actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> ALTKeyAction;

public:
	/*
	위에서 연결한 MoveAction이 발생했을 때 실행되는 함수
	방향값(입력된 벡터)를 파라미터로 받아서 실제 이동 로직을 구현하는 곳.
	*/
	UFUNCTION()
	void Move(const struct FInputActionValue& Value);

	/*
	*/
	void Look(const struct FInputActionValue& Value);

	void HandleJump(const struct FInputActionValue& Value);

	void OnRightClickStarted(const struct FInputActionValue& Value);
	void OnRightClickHeld(const struct FInputActionValue& Value);
	void OnRightClickReleased(const struct FInputActionValue& Value);

public:
	// Toggle inventory open/close
	void ToggleInventory(const struct FInputActionValue& Value);

	// Event broadcast when Tab key is pressed (for inventory toggle)
	UPROPERTY(BlueprintAssignable)
    FOnInventoryToggleRequested OnInventoryToggleRequested;

	// Event broadcast when F key is pressed (for interaction)
	UPROPERTY(BlueprintAssignable)
	FOnFKeyToggleRequested OnKeyToggleRequested;

	// Event broadcast when ALT key is pressed
	UPROPERTY(BlueprintAssignable)
	FOnALTKeyToggleRequested OnALTKeyToggleRequested;

	
protected:
	// 시점을 짧은 클릭으로 토글할 수 있도록 시간 추적용
	float RightClickHoldTime = 0.0f;
	bool bRightClickTriggered = false;


	//점프 관련
protected:
	// 현재 점프 횟수 (지상에 닿으면 0으로 리셋됨)
	// 최대 점프 횟수는 PlayerStatComponent에서 관리
	int32 JumpCount = 0;


public:
	// F Key handler (for interaction/pickup)
	void OnFKeyPressed();

	// ALT Key handler (for alternate actions)
	void OnALTKeyPressed();
};
