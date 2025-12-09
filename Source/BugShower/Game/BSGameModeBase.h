// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BSGameModeBase.generated.h"

class ABSGameStateBase;

/**
 * GameMode for BugShower multiplayer game
 * Handles game rules: win condition (survive for duration) and lose condition (all players dead)
 */
UCLASS()
class BUGSHOWER_API ABSGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABSGameModeBase();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual void StartPlay() override;

protected:
	// Cached reference to GameState
	UPROPERTY()
	TObjectPtr<ABSGameStateBase> BSGameState;

public:
	// Called when a player dies (to be called from player character)
	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	void OnPlayerDied(AController* DeadPlayerController);

	// Check if game end conditions are met
	void CheckGameEndConditions();

	// End the game with victory or defeat
	UFUNCTION(BlueprintCallable, Category = "Game Rules")
	void EndGame(bool bVictory);

	// Get alive player count
	int32 GetAlivePlayerCount() const;

	bool IsEnd() const;

	// Save all players' inventory to GameInstance
	void SaveAllPlayersInventory();

protected:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "InGame")
	FString GetNextGameMapName() const;

	/**
	 * Target map name for game start
	 * Default: "Lobby"
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InGame")
	FString NextMapName;
};