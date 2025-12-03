// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BSGameModeBase.h"
#include "Game/BSGameStateBase.h"
#include "Player/BSPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NPC/Pooling.h"
#include "Subsystems/PoolingSubSystem.h"
#include "Logging/BugShowerLog.h"


ABSGameModeBase::ABSGameModeBase()
{
	// Set GameState class
	GameStateClass = ABSGameStateBase::StaticClass();

	// Set PlayerController class for in-game
	PlayerControllerClass = ABSPlayerController::StaticClass();
}

void ABSGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* ABSGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ABSGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Update player count when a new player joins
	if (BSGameState)
	{
		BSGameState->TotalPlayerCount += 1;
		BSGameState->AlivePlayerCount = GetAlivePlayerCount();

		LOG_NETWORK_INFO(TEXT("Player joined. Total: %d, Alive: %d"), BSGameState->TotalPlayerCount, BSGameState->AlivePlayerCount);
	}
}

void ABSGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// Update player count when a player leaves
	if (BSGameState)
	{
		BSGameState->AlivePlayerCount = GetAlivePlayerCount();

		LOG_NETWORK_INFO(TEXT("Player left. Alive: %d"), BSGameState->AlivePlayerCount);

		// Check if game should end due to no players left
		CheckGameEndConditions();
	}
}

void ABSGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Cache GameState reference
	BSGameState = Cast<ABSGameStateBase>(GameState);

	if (BSGameState)
	{
		UE_LOG(LogTemp, Log, TEXT("GameState cached successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cache GameState!"));
	}


	UPoolingSubsystem* PoolSys = GetWorld()->GetSubsystem<UPoolingSubsystem>();
	if (PoolSys)
	{

		// Ǯ �ʱ�ȭ
		UDataTable* PoolTable = LoadObject<UDataTable>(
			nullptr,
			TEXT("/Game/Data/DT_PoolConfig.DT_PoolConfig")
		);

		PoolSys->InitializePoolsFromTable(PoolTable);

		UDataTable* DropTable = LoadObject<UDataTable>(
			nullptr,
			TEXT("/Game/Data/DT_MonsterDrops.DT_MonsterDrops")
		);

		if (DropTable)
		{
			PoolSys->SetDropConfigTable(DropTable);
			UE_LOG(LogTemp, Log, TEXT("Drop table loaded successfully"));
		}
	}
}

void ABSGameModeBase::StartPlay()
{
	Super::StartPlay();

	// Initialize game state
	if (BSGameState)
	{
		BSGameState->AlivePlayerCount = GetAlivePlayerCount();

		LOG_NETWORK_INFO(TEXT("Game Started! NumPlayer : %d"), BSGameState->AlivePlayerCount);
	}
}

void ABSGameModeBase::OnPlayerDied(AController* DeadPlayerController)
{
	// Only run on server
	if (!HasAuthority())
	{
		return;
	}

	if (!BSGameState || BSGameState->bGameEnded)
	{
		return;
	}

	// Update alive player count
	int32 AliveCount = GetAlivePlayerCount();
	BSGameState->SetAlivePlayerCount(AliveCount);

	if (DeadPlayerController)
	{
		FString PlayerName = DeadPlayerController->GetPlayerState<APlayerState>()
			? DeadPlayerController->GetPlayerState<APlayerState>()->GetPlayerName()
			: TEXT("Unknown");

		LOG_NETWORK_INFO(TEXT("Player died: %s. Alive players: %d"), *PlayerName, AliveCount);
	}

	// Check if all players are dead
	CheckGameEndConditions();
}

void ABSGameModeBase::CheckGameEndConditions()
{
	// Only run on server
	if (!HasAuthority())
	{
		return;
	}

	if (!BSGameState || BSGameState->bGameEnded)
	{
		return;
	}

	int32 AliveCount = GetAlivePlayerCount();

	// Defeat condition: all players are dead
	if (AliveCount <= 0 && BSGameState->TotalPlayerCount > 0)
	{
		LOG_NETWORK_INFO(TEXT("All players died! Game Over!"));
		EndGame(false); // Defeat
	}
}

void ABSGameModeBase::EndGame(bool bVictory)
{
	// Only run on server
	if (!HasAuthority())
	{
		return;
	}

	if (!BSGameState || BSGameState->bGameEnded)
	{
		return;
	}

	// Set game end state
	BSGameState->SetGameEndState(bVictory);

	if (bVictory)
	{
		LOG_NETWORK_INFO(TEXT("======== VICTORY! ========"));
	}
	else
	{
		LOG_NETWORK_INFO(TEXT("======== DEFEAT! ========"));
	}

	// TODO: Show game end UI, restart level, or return to menu
	// This can be implemented in Blueprint or with additional C++ code


	 // ��� Pooling ������ Tick ��Ȱ��ȭ
	TArray<AActor*> PoolingActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APooling::StaticClass(), PoolingActors);

	for (AActor* Actor : PoolingActors)
	{
		APooling* CurPoolingActor = Cast<APooling>(Actor);
		if (CurPoolingActor)
		{
			CurPoolingActor->ReturnAllPoolingActors();	//spawn�� ���� Ǯ�� ��ȯ
		}

		Actor->SetActorTickEnabled(false);  // Tick ������ ����
	}

}

int32 ABSGameModeBase::GetAlivePlayerCount() const
{
	// Count players with valid pawns
	// Note: When player HP system is implemented, this should check if player is actually alive
	int32 AliveCount = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			// TODO: Add check for player HP > 0 when health system is implemented
			// For now, just count players with valid pawns
			AliveCount++;
		}
	}

	return AliveCount;
}

bool ABSGameModeBase::IsEnd() const
{
	return BSGameState ? BSGameState->bGameEnded : false;
}

