// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BSLobbyGameMode.generated.h"

class ABSGameStateBase;

/**
 * GameMode for BugShower multiplayer game
 * Handles game rules: win condition (survive for duration) and lose condition (all players dead)
 */
UCLASS()
class BUGSHOWER_API ABSLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABSLobbyGameMode();

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
	/**
	 * Start the game and transition to the InGame map
	 * Called from UI when players are ready
	 * Only executes on server (uses ServerTravel for multiplayer)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartGame();

	/**
	 * 게임 시작전 가지온 아이템에 대한 정보를 저장
	 * 모든 클라이언트(+서버)에게 저장 명령 전송
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSaveBeforeGameStart();

	/**
	 * Get the map name to transition to
	 * Can be overridden in Blueprint to customize the target map
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Lobby")
	FString GetNextGameMapName() const;

protected:
	/**
	 * Target map name for game start
	 * Default: "InGame"
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Lobby")
	FString NextMapName;
};