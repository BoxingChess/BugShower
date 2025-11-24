// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "BSGameStateBase.generated.h"

/**
 * GameState for BugShower multiplayer game
 * Replicates game information to all clients
 */
UCLASS()
class BUGSHOWER_API ABSGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABSGameStateBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Game timer - replicates remaining time to clients
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game Info")
	float RemainingTime;

	// Number of alive players
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game Info")
	int32 AlivePlayerCount;

	// Total number of players in the game
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game Info")
	int32 TotalPlayerCount;

	// Has the game ended?
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game Info")
	bool bGameEnded;

	// Did players win? (true = victory, false = defeat)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game Info")
	bool bVictory;

	// Update remaining time
	void SetRemainingTime(float NewTime);

	// Update alive player count
	void SetAlivePlayerCount(int32 NewCount);

	// Set game end state
	void SetGameEndState(bool bInVictory);
};
