// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BSLobbyPlayerController.h"
#include "Game/BSLobbyGameMode.h"
#include "Manager/UIManager/BSUIManager.h"

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
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("BSLobbyPlayerController::BeginPlay - GameInstance is NULL!"));
		return;
	}

	UBSUIManager* UIManager = GameInstance->GetSubsystem<UBSUIManager>();
	if (UIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BSLobbyPlayerController::BeginPlay - Initializing Lobby UI"));
		UIManager->InitializeLobbyUI(this);
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
