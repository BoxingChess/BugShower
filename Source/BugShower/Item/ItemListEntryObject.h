// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item/BSItemInstance.h"
#include "ItemListEntryObject.generated.h"
//#include "Blueprint/DragDropOperation.h"        // UDragDropOperation

class UTexture2D;            // ← 전방선언 권장
class AItemActor;

/*
 IventortListView의 ListEntry를 만들기 위한 클래스
 */
UCLASS(BlueprintType)   //BlueprintType이게 없으면 블루프린트에 노출이 안된다..! 주의하고쓰자 또 실수했네
class BUGSHOWER_API UItemListEntryObject : public UObject
{
	  GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* Icon = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Name;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Quantity; // "x2" 표시용

    // 원본으로 역참조하고 싶을 때, 월드에 존재하는 액터일 경우
    UPROPERTY() 
    TWeakObjectPtr<class AItemActor> SourceActor;

    //인벤토리에 저장된 UObject일 경우 
    UPROPERTY() TObjectPtr<UBSItemInstance> SourceInstance;



public:
};
