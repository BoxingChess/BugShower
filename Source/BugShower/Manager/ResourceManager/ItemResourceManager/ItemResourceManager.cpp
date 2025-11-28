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

		// Register based on item type
		// 아이템 타입에 따라 등록
		switch (ItemDataAsset->ItemType)
		{
			case EItemType::Consumable:
				RegisterConsumableItem(ItemDataAsset->ItemID, ItemDataAsset);
				UE_LOG(LogTemp, Log, TEXT("  Registered Consumable: %s (ID: %d)"),
					*ItemDataAsset->DisplayName.ToString(), ItemDataAsset->ItemID);
				break;

			case EItemType::Equipment:
				RegisterEquipmentItem(ItemDataAsset->ItemID, ItemDataAsset);
				UE_LOG(LogTemp, Log, TEXT("  Registered Equipment: %s (ID: %d)"),
					*ItemDataAsset->DisplayName.ToString(), ItemDataAsset->ItemID);
				break;

			default:
				UE_LOG(LogTemp, Warning, TEXT("  Unknown item type for asset: %s"), *AssetData.AssetName.ToString());
				break;
		}
	}
}

const UBSStaticItemDataAsset* UItemResourceManager::GetStaticItem(EItemType ItemType, int32 ItemID)
{
	switch (ItemType)
	{
		case EItemType::Consumable:
		{
			// Find() returns a pointer to the value (TObjectPtr)
			// Find()는 값(TObjectPtr)에 대한 포인터를 반환
			if (TObjectPtr<UBSStaticItemDataAsset>* Found = ConsumableItems.Find(static_cast<EConsumableID>(ItemID)))
			{
				return Found->Get();
			}
			UE_LOG(LogTemp, Warning, TEXT("GetStaticItem: Consumable item with ID %d not found"), ItemID);
			return nullptr;
		}

		case EItemType::Equipment:
		{
			if (TObjectPtr<UBSStaticItemDataAsset>* Found = EquipmentItems.Find(static_cast<EEquipmentID>(ItemID)))
			{
				return Found->Get();
			}
			UE_LOG(LogTemp, Warning, TEXT("GetStaticItem: Equipment item with ID %d not found"), ItemID);
			return nullptr;
		}

		case EItemType::Quest:
		{
			// TODO: Implement quest item support when needed
			// TODO: 퀘스트 아이템 지원 추가 필요
			UE_LOG(LogTemp, Warning, TEXT("GetStaticItem: Quest items not yet implemented"));
			return nullptr;
		}

		default:
		{
			UE_LOG(LogTemp, Warning, TEXT("GetStaticItem: Invalid ItemType (%d)"), static_cast<uint8>(ItemType));
			return nullptr;
		}
	}
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
