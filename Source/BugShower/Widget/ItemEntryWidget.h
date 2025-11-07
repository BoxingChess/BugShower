// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"


#include "ItemEntryWidget.generated.h"



class UImage;
class UTextBlock;

/*
* ListView에 쓸 ItemEntry. 원래는 블루프린트로만 구현했으나 entry의 드래그앤드롭을 좀더 효율적으로 만들고 싶어 C++클래스로 바꾼다.
 */
UCLASS()
class BUGSHOWER_API UItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	

public:
    //아이콘을 저장하는 이미지 위젯
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UImage* Img_Icon;

    //아이템 이름을 저장하는 텍스트 블록
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* Txt_Name;

    //아이템의 수량을 저장하는 텍스트 블록
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* Txt_Qty;

    UPROPERTY(EditDefaultsOnly, Category="DragDrop")
	TSubclassOf<UUserWidget> DragVisualClass;

public:
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    virtual void NativeOnDragDetected(const FGeometry& Geo, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation) override;


private:
    UPROPERTY()
    TObjectPtr<class UItemListEntryObject> CachedData;
};
