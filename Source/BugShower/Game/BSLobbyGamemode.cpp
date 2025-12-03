// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BSLobbyGameMode.h"
#include "Game/BSGameStateBase.h"
#include "Player/BSLobbyPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NPC/Pooling.h"
#include "Subsystems/PoolingSubSystem.h"
#include "Logging/BugShowerLog.h"


ABSLobbyGameMode::ABSLobbyGameMode()
{
	// Set GameState class
	GameStateClass = ABSGameStateBase::StaticClass();

	// Set PlayerController class for lobby
	PlayerControllerClass = ABSLobbyPlayerController::StaticClass();

	// Default game map name
	NextMapName = TEXT("InGame");
}

void ABSLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* ABSLobbyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ABSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

}

void ABSLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	
}

void ABSLobbyGameMode::InitGameState()
{
	Super::InitGameState();

	// Cache GameState reference (called right after GameState is created)
	BSGameState = Cast<ABSGameStateBase>(GameState);

	if (BSGameState)
	{
		LOG_NETWORK_INFO(TEXT("[Lobby] GameState initialized and cached successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Lobby] Failed to cache GameState in InitGameState!"));
	}
}

void ABSLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Double-check GameState reference (backup)
	if (!BSGameState)
	{
		BSGameState = Cast<ABSGameStateBase>(GameState);

		if (BSGameState)
		{
			LOG_NETWORK_INFO(TEXT("[Lobby] GameState cached in BeginPlay (backup)"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Lobby] Failed to cache GameState!"));
		}
	}
}

void ABSLobbyGameMode::StartPlay()
{
	Super::StartPlay();
}

void ABSLobbyGameMode::StartGame()
{
	// Only allow server to start the game
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartGame called on client - only server can start the game"));
		return;
	}

	FString MapName = GetNextGameMapName();

	if (MapName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("StartGame failed: GameMapName is empty"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting game - Traveling to map: %s"), *MapName);

	// Use ServerTravel to move all connected clients to the game map
	// bAbsolute = false: Use relative path (map name only)
	// bShouldSkipGameNotify = false: Call proper game mode transitions
	GetWorld()->ServerTravel(MapName, false, false);
}

FString ABSLobbyGameMode::GetNextGameMapName_Implementation() const
{
	return NextMapName;
}