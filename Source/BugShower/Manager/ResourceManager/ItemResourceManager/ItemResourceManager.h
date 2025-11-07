// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item/ItemEnum.h"
#include "Item/BSStaticItemDataAsset.h"

#include "ItemResourceManager.generated.h"

///TODO : 싱글톤으로 바꿀 생각을 하고있다. 또한 이후 Json이나 CSV를 파싱 시켜 툴 연동에 유리하게 만들것이다.
/*
  이 클래스는 장비아이템, 소비아이템, 퀘스트 아이템에 대한 Mesh나 Texture들을 가지고 있는 클래스
  메모리의 중복사용을 피하기 위해 제작하였다.
 */
UCLASS()
class BUGSHOWER_API UItemResourceManager : public UObject
{
	GENERATED_BODY()
	
protected:
	// 소비 아이템 정적 정보
	UPROPERTY()
	TMap<EConsumableID, UBSStaticItemDataAsset*> ConsumableItems;

	// 장비 아이템 정적 정보
	UPROPERTY()
	TMap<EEquipmentID, UBSStaticItemDataAsset*> EquipmentItems;

public:
	const UBSStaticItemDataAsset* GetStaticItem(EItemType ItemType, int32 ItemID);

	//소비 아이템을 등록한다.
	void RegisterConsumableItem(uint8 ItemID, UBSStaticItemDataAsset* Data);
	//장비 아이템을 등록한다.
	void RegisterEquipmentItem(uint8 ItemID, UBSStaticItemDataAsset* Data);

	/* TODO 이건 후에 추가할것. 지금은 퀘스트 아이템이 없기 때문이다.
	// 퀘스트 아이템 정적 정보
	UPROPERTY()
	TMap<"TODO", FFL_StaticItem> QuestItems; // 혹은 EQuestItemID
	*/

};
