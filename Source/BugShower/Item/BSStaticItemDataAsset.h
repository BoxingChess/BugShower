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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "아이템의 카테고리\nConsumable: 소비 아이템\nEquipment: 장비 (무기 등)\nQuest: 퀘스트 아이템\nMaterial: 기타 재료"))
    EItemType ItemType;

    //아이템의 ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "아이템 고유 번호\n0~100: 힐/에블라 아이템\n101~200: 투척류\n201~255: 탄약"))
    uint8 ItemID;

	// 아이템 이름 (UI, 인벤토리 등에서 플레이어에게 보여지는 이름)
    // 예: "응급처치키트", "M416" 등
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "플레이어에게 보여지는 아이템 이름\n예) 응급처치키트, M416, 9mm 탄약 등"))
    FText DisplayName;

    // 아이템 아이콘 (UI에서 시각적으로 보여줄 때 사용되는 이미지)
    // 예: 인벤토리 슬롯에 들어갈 Texture2D 이미지
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "인벤토리 UI에 표시될 아이콘 이미지 (Texture2D)"))
    UTexture2D* Icon;

    // 아이템 설명 (툴팁이나 상세 정보 등에서 사용)
    // 예: "사용 시 체력을 75 회복합니다."
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "아이템 설명 텍스트\n예) 사용 시 체력을 75 회복합니다"))
    FText Description;

    // 해당 아이템이 스택 가능한지 여부 (true면 수량 누적 가능)
    // 예: 탄약, 포션 등은 true / 무기, 방어구 등은 false
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "여러 개를 한 슬롯에 쌓을 수 있는지 여부\ntrue: 탄약, 힐템 등 (쌓을 수 있음)\nfalse: 무기 등 (각각 개별 슬롯)"))
    bool bStackable = true;

    //Item 하나당 무게, 이후 InventoryStack에 쌓기 위해서는 해당 변수와 갯수를 곱한 값을 넣는다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "아이템 1개당 무게\n인벤토리 무게 제한 계산에 사용됨"))
    int32 Weight = 1;

    //아이템이 스폰될때 기본적으로 갖추고 스폰되는 수량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "월드에 스폰될 때 기본 수량\n예) 탄약은 30발 묶음으로 스폰"))
    int32 SpawnQuntity = 1;

    // 하나의 인벤토리 슬롯에 쌓을 수 있는 최대 수량
    // 예: 9mm 탄약은 999개까지, 회복약은 10개까지 등 설정 가능
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
        meta = (ToolTip = "한 슬롯에 쌓을 수 있는 최대 개수\n예) 탄약: 999, 힐템: 10"))
    int32 MaxStackSize = 999;

    // 추가된 메시 정보
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UStaticMesh* WorldMesh = nullptr;

    // 월드에 스폰될 때 적용할 스케일 (기본값 1,1,1)
    // Scale to apply when spawned in world (default 1,1,1)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FVector WorldMeshScale = FVector(1.0f, 1.0f, 1.0f);

    // 아이템을 월드에 드랍할 때 스폰할 액터 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TSubclassOf<class AItemActor> ItemActorClass;

    UPROPERTY(EditAnywhere, Category = "Item")
	float ItemLifeSpan;

    // ========================================
    // 소비 아이템 효과 (Consumable Item Effects)
    // ========================================

    // HP 회복량 (0이면 힐 아이템 아님)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Heal",
        meta = (ClampMin = "0.0",
                ToolTip = "아이템 사용 시 회복되는 HP 양\n예) 응급처치키트: 75, 구급상자: 50, 붕대: 25\n0이면 힐 아이템이 아님"))
    float HealAmount = 0.f;

    // 에블라 입자 감소량 (0이면 정화제 아님)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Abla",
        meta = (ClampMin = "0.0",
                ToolTip = "아이템 사용 시 감소되는 에블라 입자 양\n예) 에블라 정화제: 30 (현재 에블라를 30만큼 감소)\n0이면 정화제가 아님"))
    float AblaReductionAmount = 0.f;

    // 에블라 증가율 배율 (1.0 = 변화 없음, 0.5 = 50% 감소, 기본 0.1에 곱함)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Abla",
        meta = (ClampMin = "0.0", ClampMax = "2.0",
                ToolTip = "에블라 입자 증가 속도 배율 (기본 0.1에 곱함)\n1.0 = 변화 없음 (0.1 유지)\n0.5 = 50% 감소 (0.1 → 0.05)\n0.3 = 70% 감소 (0.1 → 0.03)\n예) 에블라 억제제: 0.5"))
    float AblaGainRateMultiplier = 1.0f;

    // 에블라 증가율 효과 지속 시간(초) (0이면 영구, 억제제는 60초 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Abla",
        meta = (ClampMin = "0.0",
                ToolTip = "에블라 증가율 변경 효과가 지속되는 시간(초)\n0 = 영구 지속 (복구 안 됨)\n60 = 60초 후 원래대로 복구\n예) 에블라 억제제: 60 (1분간 증가율 감소)"))
    float AblaEffectDuration = 0.f;
    //Item 판매 가치
    UPROPERTY(EditAnywhere, Category = "Item")
    int32 SellPrice;


public:
    TSubclassOf<AItemActor> GetItemActorClass() const { return ItemActorClass; }

    // 사용 가능한 소비 아이템인지 체크 (우클릭으로 사용 가능한지)
    UFUNCTION(BlueprintCallable, Category = "Item")
    bool IsUsableItem() const
    {
        return (HealAmount > 0.f ||
                AblaReductionAmount > 0.f ||
                AblaGainRateMultiplier < 1.0f);
    }

};
