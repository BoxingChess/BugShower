// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Item/BSItem.h"
#include "BSGameInstance.generated.h"

class UBSPlayerSaveGame;
class UBSItemInstance;

// 인벤토리 변경 델리게이트
// 아이템이 추가/제거/사용될 때마다 브로드캐스트됨
// UI들은 이 델리게이트를 구독하여 자동으로 갱신
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyInventoryChanged, const TArray<UBSItemInstance*>&, UpdateItems);

/**
 * BugShower Game Instance
 * BugShower 게임 인스턴스
 *
 * Main game instance class for BugShower project
 * BugShower 프로젝트의 메인 게임 인스턴스 클래스
 *
 * Uses Subsystem pattern for managers (UIManager, ItemResourceManager)
 * 매니저들은 Subsystem 패턴을 사용 (UIManager, ItemResourceManager)
 * - UIManager: UI 관리 서브시스템
 * - ItemResourceManager: 아이템 리소스 관리 서브시스템
 */
UCLASS()
class BUGSHOWER_API UBSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Game Instance initialization
	// 게임 인스턴스 초기화 (서브시스템 확인)
	virtual void Init() override;

	// ========================================
	// 인벤토리 변경 이벤트
	// ========================================

	/**
	 * 인벤토리 변경 델리게이트
	 * 아이템 추가/제거/사용 시 자동으로 브로드캐스트됨
	 * UI들은 이 델리게이트를 구독하여 자동으로 갱신할 수 있음
	 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnLobbyInventoryChanged OnStorageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnLobbyInventoryChanged OnSelectedItemsChanged;

private:
	// ========================================
	// 플레이어 영구 저장 시스템
	// ========================================

	/**
	 * 현재 로드된 세이브 데이터
	 * 게임 시작 시 자동으로 로드됨
	 */
	UPROPERTY()
	TObjectPtr<UBSPlayerSaveGame> CurrentSaveGame;

	/**
	 * 현재 플레이어의 고유 ID
	 * 멀티플레이어에서 각 클라이언트를 구분하기 위해 사용
	 * 예: "Player_12345"
	 */
	UPROPERTY()
	FString CurrentPlayerID;

	/**
	 * 유저 인덱스 (기본값 = 0)
	 */
	UPROPERTY()
	int32 UserIndex = 0;

	/**
	 * 런타임에 임시로 저장되는 아이템 목록
	 * 인게임에서 획득한 아이템들이 여기 저장되고, SaveGame 호출 시 디스크에 저장됨
	 */
	UPROPERTY()
	TArray<TObjectPtr<UBSItemInstance>> RuntimeItemInventory;

	/**
	 * 로비에서 선택한 아이템 목록 (인게임 전달용)
	 * ServerTravel 후에도 유지됨
	 */
	UPROPERTY()
	TArray<TObjectPtr<UBSItemInstance>> SelectedItemsForGame;

public:
	/**
	 * 플레이어 ID 설정
	 * 멀티플레이어에서 각 클라이언트가 자신의 고유 ID를 설정
	 * 이 ID로 개별 세이브 파일 관리 (예: "PlayerSave_12345")
	 *
	 * @param InPlayerID - 플레이어 고유 식별자
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SetPlayerID(const FString& InPlayerID);

	/**
	 * 현재 설정된 플레이어 ID 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame")
	FString GetPlayerID() const { return CurrentPlayerID; }

	/**
	 * 플레이어별 세이브 슬롯 이름 생성
	 * PlayerID가 설정되어 있으면 "PlayerSave_{ID}" 형식 사용
	 * 설정 안 되어 있으면 "PlayerSaveSlot" (기본값)
	 */
	FString GetSaveSlotName() const;

	/**
	 * 세이브 데이터 로드
	 * 게임 시작 시 자동으로 호출되며, 저장된 아이템을 불러옴
	 *
	 * @return 로드 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	bool LoadPlayerSaveData();

	/**
	 * 세이브 데이터 저장
	 * 인게임에서 획득한 아이템들을 디스크에 영구 저장
	 *
	 * @return 저장 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	bool SavePlayerData();

	/**
	 * 인게임에서 획득한 아이템을 런타임 인벤토리에 추가
	 * 레벨 클리어 시 또는 실시간으로 호출 가능
	 *
	 * @param ItemsToAdd - 추가할 아이템 목록
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItemsToRuntimeInventory(const TArray<UBSItemInstance*>& ItemsToAdd);

	
	/**
	 * 저장된 아이템 목록 가져오기 (로비 창고에서 사용)
	 * RuntimeItemInventory를 반환하여 현재 세션에서 획득한 아이템까지 포함
	 *
	 * @return 플레이어가 보유한 모든 아이템
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<UBSItemInstance*>& GetPlayerItems() const;

	/**
	 * 세이브 데이터 초기화 (개발/테스트 용도)
	 * 모든 저장된 아이템과 데이터를 삭제
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void ResetSaveData();


	/**
	 * 로비에서 개별 아이템을 선택하여 인게임으로 전달할 목록에 추가
	 * RuntimeItemInventory에서 차감하고 SelectedItemsForGame에 추가
	 * @param AddData - 추가할 아이템 정보
	 * @param Amount - 추가할 수량
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItemsToInventoryForGame(UBSItemInstance* AddData, int32 Amount);

	//창고로 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItemsToStarage(UBSItemInstance* AddData, int32 Amount);

	/**
	 * 선택한 아이템 목록 가져오기 (창고에서 선택된 아이템)
	 * SelectedItemsForGame를 반환하여 인게임에 전달할 목록
	 *
	 * @return 플레이어가 창고에서 선택한 모든 아이템
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<UBSItemInstance*>& GetSelecedItems() const;

	/**
	 * 선택한 아이템 목록 비우기
	 * 인게임 전달 후 호출하여 SelectedItemsForGame 초기화
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSelectedItems();
};
