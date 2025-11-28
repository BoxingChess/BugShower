#pragma once

#include "CoreMinimal.h"
#include "ItemEnum.h"
#include "BSItem.generated.h"

class AItemActor;

USTRUCT(BlueprintType)
struct FBS_Item
{
	GENERATED_BODY();

    //아이템의 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType;

    //아이템의 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 ItemID;

    //Item의 수량, 이후 InventoryStack에 쌓기 위해서는 해당 변수와 무게를 곱한 값을 넣는다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

};
