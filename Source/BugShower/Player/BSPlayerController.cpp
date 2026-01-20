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

	// Initialize UI Manager for this player (로컬 플레이어만!)
	// 멀티플레이어에서 다른 플레이어의 PlayerController가 UI를 생성하지 않도록 체크
	if (IsLocalController())
	{
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
				UE_LOG(LogTemp, Log, TEXT("BSPlayerController::BeginPlay - UIManager Subsystem found, initializing UI for LOCAL player..."));
				UIManager->InitializePlayerUI(this);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BSPlayerController::BeginPlay - Failed to get UIManager Subsystem from GameInstance: %s"),
					*GameInstance->GetClass()->GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BSPlayerController::BeginPlay - Skipping UI init for REMOTE player"));
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is possessing: %s"), *ControlledPawn->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is not possessing any pawn"));
	}

	// 세이브 데이터 로드 (클라이언트에서만)
	// 모든 클라이언트는 기본 슬롯 "PlayerSaveSlot" 사용
	// 각 클라이언트의 로컬 PC에 저장되므로 충돌하지 않음
	if (IsLocalController())
	{
		UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GetGameInstance());
		if (BSGameInstance)
		{
			// 세이브 데이터 로드
			BSGameInstance->LoadPlayerSaveData();
			UE_LOG(LogTemp, Log, TEXT("BSPlayerController::BeginPlay - Loaded save data from slot: %s"),
				*BSGameInstance->GetSaveSlotName());
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
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("BSPlayerController::ClientSaveInventory - Client saving inventory..."));

	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!BSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSPlayerController::ClientSaveInventory - GameInstance is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Save Slot Name: %s"), *BSGameInstance->GetSaveSlotName());
	UE_LOG(LogTemp, Warning, TEXT("Player ID: %s"), *BSGameInstance->GetPlayerID());

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

	UE_LOG(LogTemp, Warning, TEXT("Items in InventoryComponent: %d"), PlayerItems.Num());

	if (PlayerItems.Num() > 0)
	{
		// GameInstance에 아이템 추가
		BSGameInstance->AddItemsToRuntimeInventory(PlayerItems);

		UE_LOG(LogTemp, Warning, TEXT("Added %d items to GameInstance RuntimeInventory"), PlayerItems.Num());
	}

	UE_LOG(LogTemp, Warning, TEXT("Total items in GameInstance before save: %d"), BSGameInstance->GetPlayerItems().Num());

	// 클라이언트의 로컬 디스크에 저장
	bool bSaveSuccess = BSGameInstance->SavePlayerData();

	if (bSaveSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("✅ Successfully saved to local disk!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Failed to save to disk!"));
	}
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
}
