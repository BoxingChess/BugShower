// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BSPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Manager/UIManager/BSUIManager.h"
#include "Game/BSGameInstance.h"
#include "Player/BSCharacterPlayer.h"
#include "Component/Inventory/InventoryComponent.h"
#include "GameFramework/PlayerState.h"

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

	// 멀티플레이어: 플레이어 ID 설정 및 세이브 로드
	// 클라이언트에서만 실행 (IsLocalController)
	if (IsLocalController())
	{
		UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GetGameInstance());
		if (BSGameInstance)
		{
			// PlayerState에서 고유 ID 가져오기
			if (APlayerState* PS = GetPlayerState<APlayerState>())
			{
				// PlayerId를 문자열로 변환하여 사용
				FString PlayerID = FString::Printf(TEXT("%d"), PS->GetPlayerId());
				BSGameInstance->SetPlayerID(PlayerID);

				UE_LOG(LogTemp, Log, TEXT("BSPlayerController::BeginPlay - PlayerID set to: %s"), *PlayerID);

				// 세이브 데이터 로드
				BSGameInstance->LoadPlayerSaveData();
			}
		}
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

void ABSPlayerController::ClientSaveInventory_Implementation()
{
	// 클라이언트에서 실행됨!
	UE_LOG(LogTemp, Log, TEXT("BSPlayerController::ClientSaveInventory - Client saving inventory..."));

	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!BSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSPlayerController::ClientSaveInventory - GameInstance is NULL!"));
		return;
	}

	// 플레이어 캐릭터의 인벤토리 가져오기
	ABSCharacterPlayer* PlayerCharacter = Cast<ABSCharacterPlayer>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSPlayerController::ClientSaveInventory - Player has no pawn"));
		return;
	}

	UInventoryComponent* InventoryComp = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSPlayerController::ClientSaveInventory - Player has no InventoryComponent"));
		return;
	}

	// 인벤토리 아이템 가져오기
	TArray<UBSItemInstance*> PlayerItems = InventoryComp->GetItemInventory();

	if (PlayerItems.Num() > 0)
	{
		// GameInstance에 아이템 추가
		BSGameInstance->AddItemsToRuntimeInventory(PlayerItems);

		UE_LOG(LogTemp, Log, TEXT("BSPlayerController::ClientSaveInventory - Added %d items to GameInstance"),
			PlayerItems.Num());
	}

	// 클라이언트의 로컬 디스크에 저장
	bool bSaveSuccess = BSGameInstance->SavePlayerData();

	if (bSaveSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("BSPlayerController::ClientSaveInventory - Successfully saved to local disk!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSPlayerController::ClientSaveInventory - Failed to save to disk!"));
	}
}
