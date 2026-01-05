// Fill out your copyright notice in the Description page of Project Settings.

#include "MovementInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/PickUp/PickUpDetectorComponent.h"
// PlayerStatComponent include 제거: Coupling 방지를 위해 BSCharacterPlayer의 Getter 함수 사용


// Sets default values for this component's properties
UMovementInputComponent::UMovementInputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Load IA_Move input action asset
	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionRef(TEXT("/Game/Input/IA_Move.IA_Move"));
	if (MoveActionRef.Succeeded())
	{
		// If successfully found, assign to MoveAction member variable
		MoveAction = MoveActionRef.Object;
	}

	// Load IA_Look_3P input action asset
	static ConstructorHelpers::FObjectFinder<UInputAction> LookAction_ThirdRef(TEXT("/Game/Input/IA_Look_3P.IA_Look_3P"));
	if (LookAction_ThirdRef.Succeeded())
	{
		LookAction_Third = LookAction_ThirdRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionRightClickRef(TEXT("/Game/Input/IA_Look_RightClick.IA_Look_RightClick"));
	if (LookActionRightClickRef.Succeeded())
	{
		LookAction_RightClick = LookActionRightClickRef.Object;
	}

	// Jump action
	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionRef(TEXT("/Game/Input/IA_Jump.IA_Jump"));
	if (JumpActionRef.Succeeded())
	{
		JumpAction_Space = JumpActionRef.Object;
	}

	// F Key action
	static ConstructorHelpers::FObjectFinder<UInputAction> FKeyActionRef(TEXT("/Game/Input/IA_FKey.IA_FKey"));
	if (FKeyActionRef.Succeeded())
	{
		FKeyAction = FKeyActionRef.Object;
	}


	static ConstructorHelpers::FObjectFinder<UInputAction> TapKeyActionRef(TEXT("/Game/Input/IA_Inventory_Tap.IA_Inventory_Tap"));
	if (TapKeyActionRef.Succeeded())
	{
		TapKeyAction = TapKeyActionRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ALTKeyActionRef(TEXT("/Game/Input/IA_ALTKey.IA_ALTKey"));
	if (ALTKeyActionRef.Succeeded())
	{
		ALTKeyAction = ALTKeyActionRef.Object;
	}

	//Sprint action은 블루프린트에서 설정하거나, 여기서 경로 지정 가능
	///하지만 혹시 몰라 세팅은 해둘께요 나중에 다 뺄 예정입니다. 이런식의 하드코딩은 별로라고 생각합니다. 밑에 주석해도 정상적으로 돌아가는거 확인함.
		static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionRef(TEXT("/Game/Input/IA_Sprint.IA_Sprint"));
	if (SprintActionRef.Succeeded())
	{
		SprintAction = SprintActionRef.Object;
	}
}


// Called when the game starts
void UMovementInputComponent::BeginPlay()
{

	Super::BeginPlay();
	OwnerPlayer = Cast<ABSCharacterPlayer>(GetOwner());

	if (!OwnerPlayer.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("MovementInputComponent_BeginePlay_Error : !OwnerCharacter"));
	}

	RefreshControllerCache();

	// 초기 이동 속도를 Walk 속도로 설정 (PlayerStatComponent에서 가져옴)
	// PlayerStatComponent가 먼저 초기화되므로, 그곳의 값을 사용
	// BeginPlay는 PlayerStatComponent가 이미 설정한 값을 유지하면 됨
}

void UMovementInputComponent::RefreshControllerCache()
{
	if (OwnerPlayer.IsValid())
	{
		if (AController* NewController = OwnerPlayer.Get()->GetController())
		{
			OwnerController = NewController;   // TWeakObjectPtr<AController> raw pointer assignment OK
		}
		else
		{
			OwnerController.Reset();
		}
	}
	else
	{
		OwnerController.Reset();
	}
}


void UMovementInputComponent::FL_SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInput Casting in UMovementInputComponent"));

		// Input binding: when MoveAction is Triggered (input is active),
		// call UMovementInputComponent::Move() function
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UMovementInputComponent::Move);


		EnhancedInput->BindAction(LookAction_Third, ETriggerEvent::Triggered, this, &UMovementInputComponent::Look);

		EnhancedInput->BindAction(LookAction_RightClick, ETriggerEvent::Started, this, &UMovementInputComponent::OnRightClickStarted);
		EnhancedInput->BindAction(LookAction_RightClick, ETriggerEvent::Triggered, this, &UMovementInputComponent::OnRightClickHeld);
		EnhancedInput->BindAction(LookAction_RightClick, ETriggerEvent::Completed, this, &UMovementInputComponent::OnRightClickReleased);

		EnhancedInput->BindAction(JumpAction_Space, ETriggerEvent::Triggered, this, &UMovementInputComponent::HandleJump);
		EnhancedInput->BindAction(FKeyAction, ETriggerEvent::Triggered, this, &UMovementInputComponent::OnFKeyPressed);
		EnhancedInput->BindAction(TapKeyAction, ETriggerEvent::Triggered, this, &UMovementInputComponent::ToggleInventory);
		EnhancedInput->BindAction(ALTKeyAction, ETriggerEvent::Triggered, this, &UMovementInputComponent::OnALTKeyPressed);

		// Sprint binding
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &UMovementInputComponent::OnSprintStarted);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &UMovementInputComponent::OnSprintStopped);
			UE_LOG(LogTemp, Log, TEXT("MovementInputComponent - SprintAction bound"));
		}
	}
}

void UMovementInputComponent::Move(const struct FInputActionValue& Value)
{
	// 인벤토리 열려도 이동은 가능하게 할것임.

	//UE_LOG(LogTemp, Warning, TEXT("Move called!"));
	
	if (!OwnerPlayer.IsValid()) return;

	// 점프 중일 때는 WASD 입력으로 힘을 받지 못하게 함
	// Disable WASD movement input while jumping (in air)
	if (!OwnerPlayer->GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	// Get input movement vector from keyboard (usually WASD keys)
	FVector2D MovementVector = Value.Get<FVector2D>();

	// Check if player controller is valid
	if (OwnerController != nullptr)
	{
		// Get controller's rotation direction (Yaw only)
		const FRotator ControlRotation = OwnerController->GetControlRotation();
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		// Get forward and right direction vectors from rotation
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Apply movement input based on camera direction
		OwnerPlayer->AddMovementInput(ForwardDirection, MovementVector.Y);
		OwnerPlayer->AddMovementInput(RightDirection, MovementVector.X);
		//UE_LOG(LogTemp, Warning, TEXT("Location: %s"), *GetActorLocation().ToString());
	}
}

void UMovementInputComponent::Look(const struct FInputActionValue& Value)
{
	// 수정: OwnerController null 체크 추가 (166번 라인에서 역참조하기 전에 검증)
	if (!OwnerPlayer.IsValid() || !OwnerController.IsValid()) return;

	static int CallCount = 0;
	if (CallCount < 5)
	{
		UE_LOG(LogTemp, Error, TEXT("!!! Look() CALLED !!! (count: %d)"), CallCount++);
	}

	FVector2D LookAxisVector = Value.Get<FVector2D>();

	constexpr float LookSensitivity = 0.3f; // Mouse sensitivity (should be exposed to player settings)
	OwnerPlayer->AddControllerYawInput(LookAxisVector.X * LookSensitivity);    // Mouse horizontal -> Yaw

	//UE_LOG(LogTemp, Warning, TEXT("LookAxisVector.x: %f"), LookAxisVector.X);
	//UE_LOG(LogTemp, Warning, TEXT("Rotation: %s"), *GetActorRotation().ToString());

	// Get current controller rotation
	FRotator ControlRotation = OwnerController->GetControlRotation();

	float NewPitch = ControlRotation.Pitch + LookAxisVector.Y;
	//UE_LOG(LogTemp, Warning, TEXT("NewPitch: %f"), NewPitch);
	NewPitch = FMath::UnwindDegrees(NewPitch);

	// Clamp Pitch range: -90 to +90 degrees
	NewPitch = FMath::Clamp(NewPitch, -90.0f, 90.0f);

	OwnerController->SetControlRotation(FRotator(NewPitch, ControlRotation.Yaw, 0.0f));
}

void UMovementInputComponent::HandleJump(const struct FInputActionValue& Value)
{
	// 지상에서 점프 (첫 번째 점프)
	if (OwnerPlayer->GetCharacterMovement()->IsMovingOnGround())
	{
		JumpCount = 1; // First jump
		OwnerPlayer->Jump();
	}
	// 공중에서 추가 점프 (더블 점프 등)
	else
	{
		// BSCharacterPlayer의 Getter 함수 사용 (Loose coupling)
		// MovementComponent는 StatComponent를 직접 알 필요 없음
		int32 MaxJumps = OwnerPlayer->GetMaxJumpCount();
		float JumpPower = OwnerPlayer->GetJumpPower();

		// 최대 점프 횟수 체크
		if (JumpCount < MaxJumps)
		{
			JumpCount++;
			// 점프력을 BSCharacterPlayer에서 가져온 값으로 사용
			OwnerPlayer->LaunchCharacter(FVector(0.f, 0.f, JumpPower), false, true);
		}
	}
}

void UMovementInputComponent::OnRightClickStarted(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("OnRightClickStarted"));
	RightClickHoldTime = 0.0f;
	bRightClickTriggered = false;
}

void UMovementInputComponent::OnRightClickHeld(const struct FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Held: %.4f"), RightClickHoldTime);

	// Only process if in third person mode
	if (OwnerPlayer->CurrentViewMode != ECameraViewMode::ThirdPerson)
		return;

	RightClickHoldTime += GetWorld()->GetDeltaSeconds();
	if (!bRightClickTriggered && RightClickHoldTime > 0.2f)
	{
		bRightClickTriggered = true;
		// Start third person zoom/aim mode
		OwnerPlayer->StartThirdPersonZoom(); // Reduce FOV
	}
}

void UMovementInputComponent::OnRightClickReleased(const struct FInputActionValue& Value)
{
	if (OwnerPlayer->GetCurrentViewMode() == ECameraViewMode::FirstPerson)
	{
		// If in first person, switch back to third person
		OwnerPlayer->SetCameraViewMode(ECameraViewMode::ThirdPerson);
		OwnerPlayer->EndThirdPersonZoom();
	}
	else
	{

		if (!bRightClickTriggered)
		{
			// Short click - toggle view mode
			OwnerPlayer->ToggleViewMode();
		}
		else
		{
			// Was zooming, now release zoom
			OwnerPlayer->EndThirdPersonZoom();
		}
	}

	bRightClickTriggered = false;
	RightClickHoldTime = 0.0f;
}

// Tab key pressed - toggle inventory
void UMovementInputComponent::ToggleInventory(const struct FInputActionValue& Value)
{
	// Only process if player is valid and locally controlled
	if (OwnerPlayer.IsValid() && OwnerPlayer->IsLocallyControlled())
	{
		OnInventoryToggleRequested.Broadcast();
	}
}

void UMovementInputComponent::OnFKeyPressed()
{
	// Only process if player is valid and locally controlled
	if (OwnerPlayer.IsValid() && OwnerPlayer->IsLocallyControlled())
	{
		OnKeyToggleRequested.Broadcast();
	}
}

void UMovementInputComponent::OnALTKeyPressed()
{
	// Only process if player is valid and locally controlled
	if (OwnerPlayer.IsValid() && OwnerPlayer->IsLocallyControlled())
	{
		OnALTKeyToggleRequested.Broadcast();
	}
}

void UMovementInputComponent::OnSprintStarted()
{
	if (!OwnerPlayer.IsValid()) return;

	// BSCharacterPlayer의 Getter를 통해 Sprint 속도 가져오기
	// StatComponent를 직접 알 필요 없음 (Loose Coupling)
	if (UCharacterMovementComponent* MovementComp = OwnerPlayer->GetCharacterMovement())
	{
		float SprintSpeed = OwnerPlayer->GetSprintSpeed();
		MovementComp->MaxWalkSpeed = SprintSpeed;
		UE_LOG(LogTemp, Log, TEXT("Sprint Started - MaxWalkSpeed: %.1f"), SprintSpeed);
	}
}

void UMovementInputComponent::OnSprintStopped()
{
	if (!OwnerPlayer.IsValid()) return;

	// BSCharacterPlayer의 Getter를 통해 Walk 속도 가져오기
	// StatComponent를 직접 알 필요 없음 (Loose Coupling)
	if (UCharacterMovementComponent* MovementComp = OwnerPlayer->GetCharacterMovement())
	{
		float WalkSpeed = OwnerPlayer->GetWalkSpeed();
		MovementComp->MaxWalkSpeed = WalkSpeed;
		UE_LOG(LogTemp, Log, TEXT("Sprint Stopped - MaxWalkSpeed: %.1f"), WalkSpeed);
	}
}
