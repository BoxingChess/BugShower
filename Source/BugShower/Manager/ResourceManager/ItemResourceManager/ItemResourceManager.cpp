// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemResourceManager.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UItemResourceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("ItemResourceManager::Initialize - Loading all item data assets..."));

	// Load all item data assets from the project
	// 프로젝트 내 모든 아이템 데이터 에셋 로드
	LoadAllItemDataAssets();

	UE_LOG(LogTemp, Log, TEXT("ItemResourceManager::Initialize - Loaded %d consumable items, %d equipment items"),
		ConsumableItems.Num(), EquipmentItems.Num());
}

void UItemResourceManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("ItemResourceManager::Deinitialize - Clearing item registries"));

	// Clear all registered items
	// 등록된 모든 아이템 정리
	AllItems.Empty();
	ConsumableItems.Empty();
	EquipmentItems.Empty();

	Super::Deinitialize();
}

void UItemResourceManager::LoadAllItemDataAssets()
{
	// Get the asset registry module
	// 에셋 레지스트리 모듈 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all UBSStaticItemDataAsset assets
	// 모든 UBSStaticItemDataAsset 에셋 검색
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBSStaticItemDataAsset::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogTemp, Log, TEXT("ItemResourceManager::LoadAllItemDataAssets - Found %d data assets"), AssetDataList.Num());

	// Load and register each asset
	// 각 에셋을 로드하고 등록
	for (const FAssetData& AssetData : AssetDataList)
	{
		UBSStaticItemDataAsset* ItemDataAsset = Cast<UBSStaticItemDataAsset>(AssetData.GetAsset());
		if (!ItemDataAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("ItemResourceManager::LoadAllItemDataAssets - Failed to load asset: %s"),
				*AssetData.AssetName.ToString());
			continue;
		}

		// ========================================
		// 통합 레지스트리에 등록 (AllItems)
		// ========================================
		if (AllItems.Contains(ItemDataAsset->ItemID))
		{
			UE_LOG(LogTemp, Warning, TEXT("  ⚠️ ItemID %d already registered! Overwriting with: %s"),
				ItemDataAsset->ItemID, *ItemDataAsset->DisplayName.ToString());
		}
		AllItems.Add(ItemDataAsset->ItemID, ItemDataAsset);

		// 타입별 로그 출력
		FString TypeName;
		switch (ItemDataAsset->ItemType)
		{
			case EItemType::Consumable: TypeName = TEXT("Consumable"); break;
			case EItemType::Equipment:  TypeName = TEXT("Equipment"); break;
			case EItemType::Material:   TypeName = TEXT("Material"); break;
			case EItemType::Quest:      TypeName = TEXT("Quest"); break;
			default:                    TypeName = TEXT("Unknown"); break;
		}

		UE_LOG(LogTemp, Log, TEXT("  Registered %s: %s (ID: %d)"),
			*TypeName, *ItemDataAsset->DisplayName.ToString(), ItemDataAsset->ItemID);

		// [DEPRECATED] 기존 타입별 레지스트리에도 등록 (호환성 유지)
		switch (ItemDataAsset->ItemType)
		{
			case EItemType::Consumable:
				RegisterConsumableItem(ItemDataAsset->ItemID, ItemDataAsset);
				break;

			case EItemType::Equipment:
				RegisterEquipmentItem(ItemDataAsset->ItemID, ItemDataAsset);
				break;

			default:
				break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ItemResourceManager - Total items registered in AllItems: %d"), AllItems.Num());
}

const UBSStaticItemDataAsset* UItemResourceManager::GetStaticItem(EItemType ItemType, int32 ItemID)
{
	// 통합 레지스트리에서 검색 (ItemType 무관하게 ItemID로 검색)
	if (TObjectPtr<UBSStaticItemDataAsset>* Found = AllItems.Find(static_cast<uint8>(ItemID)))
	{
		return Found->Get();
	}

	// AllItems에서 못 찾으면 기존 타입별 레지스트리에서 검색 (호환성)
	switch (ItemType)
	{
		case EItemType::Consumable:
		{
			if (TObjectPtr<UBSStaticItemDataAsset>* Found = ConsumableItems.Find(static_cast<EConsumableID>(ItemID)))
			{
				return Found->Get();
			}
			break;
		}

		case EItemType::Equipment:
		{
			if (TObjectPtr<UBSStaticItemDataAsset>* Found = EquipmentItems.Find(static_cast<EEquipmentID>(ItemID)))
			{
				return Found->Get();
			}
			break;
		}

		default:
			break;
	}

	UE_LOG(LogTemp, Warning, TEXT("GetStaticItem: Item not found - ItemType=%d, ItemID=%d"),
		static_cast<uint8>(ItemType), ItemID);
	return nullptr;
}

void UItemResourceManager::RegisterConsumableItem(uint8 ItemID, UBSStaticItemDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterConsumableItem: Null DataAsset provided"));
		return;
	}

	// Use the ItemID from the DataAsset itself
	// DataAsset 자체의 ItemID 사용
	EConsumableID ConsumableID = static_cast<EConsumableID>(DataAsset->ItemID);

	// Warn if overwriting existing item
	// 기존 아이템을 덮어쓰는 경우 경고
	if (ConsumableItems.Contains(ConsumableID))
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterConsumableItem: Item ID %d already registered, overwriting"), DataAsset->ItemID);
	}

	ConsumableItems.Add(ConsumableID, DataAsset);
}

void UItemResourceManager::RegisterEquipmentItem(uint8 ItemID, UBSStaticItemDataAsset* DataAsset)
{
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterEquipmentItem: Null DataAsset provided"));
		return;
	}

	// Use the ItemID from the DataAsset itself
	// DataAsset 자체의 ItemID 사용
	EEquipmentID EquipmentID = static_cast<EEquipmentID>(DataAsset->ItemID);

	// Warn if overwriting existing item
	// 기존 아이템을 덮어쓰는 경우 경고
	if (EquipmentItems.Contains(EquipmentID))
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterEquipmentItem: Item ID %d already registered, overwriting"), DataAsset->ItemID);
	}

	EquipmentItems.Add(EquipmentID, DataAsset);
}
