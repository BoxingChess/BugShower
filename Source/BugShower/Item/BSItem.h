#pragma once

#include "CoreMinimal.h"
#include "ItemEnum.h"
#include "BSItem.generated.h"

class AItemActor;

USTRUCT(BlueprintType)
struct FBS_Item
{
	GENERATED_BODY();

    //아이템의 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType = EItemType::None;

    //아이템의 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 ItemID = 0;

    //Item의 수량, 하나의 InventoryStack에 담기 위해서는 해당 아이템 개수만 담을 수가 있다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 0;

};
