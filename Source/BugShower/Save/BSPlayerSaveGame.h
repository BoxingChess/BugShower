// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Item/BSItem.h"
#include "BSPlayerSaveGame.generated.h"




/**
 * 플레이어의 영구 데이터를 저장하는 SaveGame 클래스
 * 게임을 껐다 켜도 유지되는 데이터를 관리
 */
UCLASS()
class BUGSHOWER_API UBSPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UBSPlayerSaveGame();

	// ========================================
	// 저장 슬롯 정보
	// ========================================

	/**
	 * 기본 저장 슬롯 이름
	 * 향후 멀티플 세이브 슬롯 지원 시 확장 가능
	 */
	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SaveSlotName;

	/**
	 * 저장 슬롯 인덱스 (0 = 기본 슬롯)
	 */
	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	uint32 UserIndex;

	// ========================================
	// 플레이어 아이템 데이터
	// ========================================

	/**
	 * 플레이어가 영구적으로 보유한 아이템 목록
	 * 인게임에서 획득한 아이템들이 여기에 저장됨
	 */
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<FBS_Item> SavedItems;

	/**
	 * 보유 재화
	 */
	UPROPERTY(VisibleAnywhere, Category = "Player")
	int32 Credit;

	UPROPERTY(VisibleAnywhere, Category = "Player")
	int32 RecruitmentTicket;

	// ========================================
	// 추가 플레이어 데이터 (향후 확장)
	// ========================================

	/**
	 * 플레이어 레벨 (예시)
	 */
	UPROPERTY(VisibleAnywhere, Category = "Player")
	int32 PlayerLevel;

	/**
	 * 플레이어 경험치 (예시)
	 */
	UPROPERTY(VisibleAnywhere, Category = "Player")
	int32 PlayerExperience;

};
