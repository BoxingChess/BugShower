// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemResourceManager.h"

const UBSStaticItemDataAsset* UItemResourceManager::GetStaticItem(EItemType ItemType, int32 ItemID)
{
	switch (ItemType)
	{
		case EItemType::Consumable:
		{
			//Find함수는 value의 포인너를 반환하므로
			UBSStaticItemDataAsset** Found = ConsumableItems.Find(static_cast<EConsumableID>(ItemID));
			return Found ? *Found : nullptr;
		}

		case EItemType::Equipment:
		{
			UBSStaticItemDataAsset** Found = EquipmentItems.Find(static_cast<EEquipmentID>(ItemID));
			return Found ? *Found : nullptr;
		}

		case EItemType::Quest:
		{
			// return QuestItems.Find(ItemID); // TODO : 이건 후에 퀘스트 아이템이 생긴다면 추가하도록 하자.
		}

		default: 
		{
			UE_LOG(LogTemp, Warning, TEXT("GetStaticItem: 잘못된 ItemType (%d)이 전달."), static_cast<uint8>(ItemType));
			return nullptr;
		}
	}
}

void UItemResourceManager::RegisterConsumableItem(uint8 ItemID, UBSStaticItemDataAsset* DataAsset)
{
	if (!DataAsset) return;

	// 예: EConsumableID를 키로 등록
	ConsumableItems.Add(static_cast<EConsumableID>(DataAsset->ItemID), DataAsset);
}

void UItemResourceManager::RegisterEquipmentItem(uint8 ItemID, UBSStaticItemDataAsset* DataAsset)
{
	if (!DataAsset) return;

	// 예: EEquipmentID를 키로 등록
	EquipmentItems.Add(static_cast<EEquipmentID>(DataAsset->ItemID), DataAsset);
}
