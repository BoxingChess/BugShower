// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BSPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Manager/UIManager/BSUIManager.h"

ABSPlayerController::ABSPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Game/Input/BS_PlayerIMC"));
	if (InputMappingContextRef.Succeeded())
	{
		InputMappingContext = InputMappingContextRef.Object;
	}
}

void ABSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input mode to Game Only for gameplay
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	// Setup Enhanced Input System
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	// Initialize UI Manager for this player
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSPlayerController::BeginPlay - GameInstance is NULL!"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BSPlayerController::BeginPlay - GameInstance found: %s"), *GameInstance->GetClass()->GetName());

		UBSUIManager* UIManager = GameInstance->GetSubsystem<UBSUIManager>();
		if (UIManager)
		{
			UE_LOG(LogTemp, Log, TEXT("BSPlayerController::BeginPlay - UIManager Subsystem found, initializing UI..."));
			UIManager->InitializePlayerUI(this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BSPlayerController::BeginPlay - Failed to get UIManager Subsystem from GameInstance: %s"),
				*GameInstance->GetClass()->GetName());
		}
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is possessing: %s"), *ControlledPawn->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is not possessing any pawn"));
	}
}

void ABSPlayerController::EnableGameInput()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext && !Subsystem->HasMappingContext(InputMappingContext))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
			UE_LOG(LogTemp, Warning, TEXT("BSPlayerController::EnableGameInput - Added InputMappingContext"));
		}
	}
}

void ABSPlayerController::DisableGameInput()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext && Subsystem->HasMappingContext(InputMappingContext))
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
			UE_LOG(LogTemp, Warning, TEXT("BSPlayerController::DisableGameInput - Removed InputMappingContext"));
		}
	}
}
