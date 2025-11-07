// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemEnum.h"

#include "BSStaticItemDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class BUGSHOWER_API UBSStaticItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    //아이템의 종류
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    EItemType ItemType;

    //아이템의 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    uint8 ItemID;

	// 아이템 이름 (UI, 인벤토리 등에서 플레이어에게 보여지는 이름)
    // 예: "응급처치키트", "M416" 등
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    // 아이템 아이콘 (UI에서 시각적으로 보여줄 때 사용되는 이미지)
    // 예: 인벤토리 슬롯에 들어갈 Texture2D 이미지
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon;

    // 아이템 설명 (툴팁이나 상세 정보 등에서 사용)
    // 예: "사용 시 체력을 75 회복합니다."
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText Description;

    // 해당 아이템이 스택 가능한지 여부 (true면 수량 누적 가능)
    // 예: 탄약, 포션 등은 true / 무기, 방어구 등은 false
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    bool bStackable = true;

    //Item 하나당 무게, 이후 InventoryStack에 쌓기 위해서는 해당 변수와 갯수를 곱한 값을 넣는다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 Weight = 1;

    // 하나의 인벤토리 슬롯에 쌓을 수 있는 최대 수량
    // 예: 9mm 탄약은 999개까지, 회복약은 10개까지 등 설정 가능
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MaxStackSize = 999;

    // 추가된 메시 정보
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UStaticMesh* WorldMesh = nullptr;

    // 아이템을 월드에 드랍할 때 스폰할 액터 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TSubclassOf<class AItemActor> ItemActorClass;

public:
    TSubclassOf<AItemActor> GetItemActorClass() const { return ItemActorClass; }
	
};
