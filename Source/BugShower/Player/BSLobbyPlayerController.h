// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BSLobbyPlayerController.generated.h"

class UBSItemInstance;

/**
 * PlayerController for Lobby
 * Handles lobby-specific functionality (UI interaction, game start request)
 * No character movement or gameplay input
 */
UCLASS()
class BUGSHOWER_API ABSLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABSLobbyPlayerController();

	virtual void BeginPlay() override;

	/**
	 * Request server to start the game (transition from lobby to game map)
	 * Called from lobby UI on client, executed on server
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Lobby")
	void ServerRequestStartGame();

	/**
	 * Get player's saved items from GameInstance
	 * 로비에서 플레이어가 보유한 아이템 목록을 가져옴
	 *
	 * @return 저장된 아이템 목록
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<UBSItemInstance*> GetSavedPlayerItems() const;

	/**
	 * Display saved items in UI
	 * 보유 아이템을 UI에 표시 (Blueprint에서 호출)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ShowSavedItems();
};
