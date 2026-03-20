// Fill out your copyright notice in the Description page of Project Settings.

#include "Save/BSPlayerSaveGame.h"

UBSPlayerSaveGame::UBSPlayerSaveGame()
{
	// 기본 저장 슬롯 설정
	SaveSlotName = TEXT("PlayerSaveSlot");
	UserIndex = 0;

	// 초기값 설정
	PlayerLevel = 1;
	PlayerExperience = 0;
	Credit = 0;
	RecruitmentTicket = 0;
}
