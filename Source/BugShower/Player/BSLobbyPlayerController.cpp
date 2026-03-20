// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BSLobbyPlayerController.h"
#include "Game/BSLobbyGameMode.h"
#include "Game/BSGameInstance.h"
#include "Manager/UIManager/BSUIManager.h"
#include "Item/BSItemInstance.h"

ABSLobbyPlayerController::ABSLobbyPlayerController()
{
	// Lobby only needs UI input, no game input
	bShowMouseCursor = true;
}

void ABSLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set input mode to UI only for lobby
	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	// Initialize UI Manager for this player
	UBSGameInstance* GameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSLobbyPlayerController::BeginPlay - GameInstance is NULL!"));
		return;
	}

	// GameInstance::Init()에서 이미 로드했으므로 여기서는 로드하지 않음
	// ServerTravel 후에도 GameInstance의 RuntimeItemInventory는 유지됨
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("BSLobbyPlayerController::BeginPlay - Using existing RuntimeItemInventory"));
	UE_LOG(LogTemp, Warning, TEXT("Current items in inventory: %d"), GameInstance->GetPlayerItems().Num());
	UE_LOG(LogTemp, Warning, TEXT("========================================"));

	UBSUIManager* UIManager = GameInstance->GetSubsystem<UBSUIManager>();
	if (UIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BSLobbyPlayerController::BeginPlay - Initializing Lobby UI"));
		UIManager->SwitchUIConfig(BSUIConfigNames::Lobby);
		UIManager->InitializePlayerUI(this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSLobbyPlayerController::BeginPlay - Failed to get UIManager Subsystem"));
	}
}

void ABSLobbyPlayerController::ServerRequestStartGame_Implementation()
{
	// This function runs on the server
	// Get the current game mode and start the game
	if (ABSLobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ABSLobbyGameMode>())
	{
		UE_LOG(LogTemp, Log, TEXT("ServerRequestStartGame - Player %s requested game start"), *GetName());
		LobbyGameMode->StartGame();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerRequestStartGame - Current GameMode is not BSLobbyGameMode"));
	}
}

TArray<UBSItemInstance*> ABSLobbyPlayerController::GetSavedPlayerItems() const
{
	// GameInstance에서 저장된 아이템 가져오기
	UBSGameInstance* GameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSLobbyPlayerController::GetSavedPlayerItems - GameInstance is NULL!"));
		return TArray<UBSItemInstance*>();
	}

	const TArray<UBSItemInstance*>& SavedItems = GameInstance->GetPlayerItems();

	UE_LOG(LogTemp, Log, TEXT("BSLobbyPlayerController::GetSavedPlayerItems - Retrieved %d items from GameInstance"),
		SavedItems.Num());

	return SavedItems;
}

void ABSLobbyPlayerController::ShowSavedItems()
{
	UBSGameInstance* GameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSLobbyPlayerController::ShowSavedItems - GameInstance is NULL!"));
		return;
	}

	const TArray<UBSItemInstance*>& SavedItems = GameInstance->GetPlayerItems();

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("       PLAYER SAVE DATA SUMMARY"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Save Slot: %s"), *GameInstance->GetSaveSlotName());
	UE_LOG(LogTemp, Log, TEXT("Player ID: %s"), GameInstance->GetPlayerID().IsEmpty() ? TEXT("(Default)") : *GameInstance->GetPlayerID());
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Total Items: %d"), SavedItems.Num());
	UE_LOG(LogTemp, Log, TEXT("----------------------------------------"));

	if (SavedItems.Num() > 0)
	{
		for (int32 i = 0; i < SavedItems.Num(); ++i)
		{
			const UBSItemInstance* Item = SavedItems[i];
			if (Item && Item->StaticData)
			{
				UE_LOG(LogTemp, Log, TEXT("[%d] %s (ID: %d) - Quantity: %d"),
					i + 1,
					*Item->StaticData->DisplayName.ToString(),
					Item->Dynamic.ItemID,
					Item->Dynamic.Quantity);
			}
			else if (Item)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%d] Unknown Item (ID: %d) - Quantity: %d (StaticData missing)"),
					i + 1,
					Item->Dynamic.ItemID,
					Item->Dynamic.Quantity);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No items found in save data."));
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));

	// TODO: UI 위젯에 아이템 목록 표시
	// Blueprint에서 InventoryWidget을 생성하고 아이템 목록을 전달할 수 있음
}
