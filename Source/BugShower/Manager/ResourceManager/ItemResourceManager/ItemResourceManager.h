// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Item/ItemEnum.h"
#include "Item/BSStaticItemDataAsset.h"

#include "ItemResourceManager.generated.h"

/**
 * Item Resource Manager Subsystem
 * 아이템 리소스 관리자 서브시스템
 *
 * Manages all static item data assets (Consumables, Equipment, Quest items, etc.)
 * 모든 정적 아이템 데이터 에셋을 관리 (소모품, 장비, 퀘스트 아이템 등)
 *
 * Prevents memory duplication by caching Mesh and Texture resources
 * Mesh와 Texture 리소스를 캐싱하여 메모리 중복을 방지
 *
 * Automatically loads and registers all BSStaticItemDataAsset on initialization
 * 초기화 시 모든 BSStaticItemDataAsset을 자동으로 로드 및 등록
 */
UCLASS()
class BUGSHOWER_API UItemResourceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	// 서브시스템 초기화/해제
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

protected:
	// Consumable item data registry
	// 소모품 아이템 데이터 등록소
	UPROPERTY()
	TMap<EConsumableID, TObjectPtr<UBSStaticItemDataAsset>> ConsumableItems;

	// Equipment item data registry
	// 장비 아이템 데이터 등록소
	UPROPERTY()
	TMap<EEquipmentID, TObjectPtr<UBSStaticItemDataAsset>> EquipmentItems;

public:
	/**
	 * Get static item data by type and ID
	 * 아이템 타입과 ID로 정적 아이템 데이터 조회
	 *
	 * @param ItemType Type of item (Consumable, Equipment, etc.) - 아이템 타입 (소모품, 장비 등)
	 * @param ItemID Unique identifier within the type - 해당 타입 내 고유 식별자
	 * @return Static item data, or nullptr if not found - 정적 아이템 데이터 (없으면 nullptr)
	 */
	const UBSStaticItemDataAsset* GetStaticItem(EItemType ItemType, int32 ItemID);

	/**
	 * Register a consumable item manually (for dynamic loading)
	 * 소모품 아이템을 수동으로 등록 (동적 로딩용)
	 *
	 * @param ItemID Consumable item ID - 소모품 아이템 ID
	 * @param DataAsset Data asset to register - 등록할 데이터 에셋
	 */
	void RegisterConsumableItem(uint8 ItemID, UBSStaticItemDataAsset* DataAsset);

	/**
	 * Register an equipment item manually (for dynamic loading)
	 * 장비 아이템을 수동으로 등록 (동적 로딩용)
	 *
	 * @param ItemID Equipment item ID - 장비 아이템 ID
	 * @param DataAsset Data asset to register - 등록할 데이터 에셋
	 */
	void RegisterEquipmentItem(uint8 ItemID, UBSStaticItemDataAsset* DataAsset);

private:
	/**
	 * Load all BSStaticItemDataAsset from the project
	 * 프로젝트 내 모든 BSStaticItemDataAsset 로드
	 *
	 * Called during Initialize()
	 * Initialize() 시 자동으로 호출됨
	 */
	void LoadAllItemDataAssets();

	/* TODO: 퀘스트 아이템 지원이 필요할 때 추가
	// Quest item data registry
	// 퀘스트 아이템 데이터 등록소
	UPROPERTY()
	TMap<EQuestItemID, TObjectPtr<UBSStaticItemDataAsset>> QuestItems;
	*/
};
