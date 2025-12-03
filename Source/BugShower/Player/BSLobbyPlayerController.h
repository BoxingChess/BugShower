// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BSLobbyPlayerController.generated.h"

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
};
