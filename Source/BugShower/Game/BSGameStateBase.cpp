// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BSGameStateBase.h"

ABSGameStateBase::ABSGameStateBase()
{
	RemainingTime = 0.0f;
	AlivePlayerCount = 0;
	TotalPlayerCount = 0;
	bGameEnded = false;
	bVictory = false;
}

void ABSGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABSGameStateBase, RemainingTime);
	DOREPLIFETIME(ABSGameStateBase, AlivePlayerCount);
	DOREPLIFETIME(ABSGameStateBase, TotalPlayerCount);
	DOREPLIFETIME(ABSGameStateBase, bGameEnded);
	DOREPLIFETIME(ABSGameStateBase, bVictory);
}

void ABSGameStateBase::SetAlivePlayerCount(int32 NewCount)
{
	AlivePlayerCount = NewCount;
}

void ABSGameStateBase::SetGameEndState(bool bInVictory)
{
	bGameEnded = true;
	bVictory = bInVictory;
}
