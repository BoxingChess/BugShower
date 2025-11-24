// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BSGameModeBase.h"

ABSGameModeBase::ABSGameModeBase()
{
	//TODO : 조만간 얼른 바꿀것. 보자마자 바꾸도록 한다.
	///DefaultPawnClass = AFL_CharacterPlayer::StaticClass();
	///PlayerControllerClass = AFL_PlayerController::StaticClass();

	//언리얼 엔진 내부에서 DefaultPawnClass와 PlayerControllerClass을 연결한다.

}

void ABSGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* ABSGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ABSGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ABSGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ABSGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABSGameModeBase::StartPlay()
{
	Super::StartPlay();
}
