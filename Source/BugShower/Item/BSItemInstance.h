// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Item/BSitem.h"
#include "Item/BSStaticItemDataAsset.h"

#include "BSItemInstance.generated.h"

/*
 인벤토리에서 아이템의 정보를 저장하기 위한 얕은 데이터.
 */
UCLASS()
class BUGSHOWER_API UBSItemInstance : public UObject
{
	GENERATED_BODY()
public:
	// UI용 정적 정보
	UPROPERTY(BlueprintReadOnly, Category="Item")
	TObjectPtr<const UBSStaticItemDataAsset> StaticData = nullptr;

	// 수량 등 동적 상태
	UPROPERTY(BlueprintReadOnly, Category="Item")
	FBS_Item Dynamic;

public:
	 //원본으로 역참조하고 싶을 때 -> 아무래도 필요할꺼 같다.
    UPROPERTY() 
    TWeakObjectPtr<class AItemActor> SourceActor;
	
public:
	FBS_Item& GetItemData() { return Dynamic; }
	const UBSStaticItemDataAsset* GetItemStaticData() const { return StaticData; }
};
