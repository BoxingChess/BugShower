// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"


#include "ItemEntryWidget.generated.h"



class UImage;
class UTextBlock;

/*
* ListView�� �� ItemEntry. ������ ��������Ʈ�θ� ���������� entry�� �巡�׾ص���� ���� ȿ�������� ����� �;� C++Ŭ������ �ٲ۴�.
 */
UCLASS()
class BUGSHOWER_API UItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	

public:
    //�������� �����ϴ� �̹��� ����
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UImage* Img_Icon;

    //������ �̸��� �����ϴ� �ؽ�Ʈ ����
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* Txt_Name;

    //�������� ������ �����ϴ� �ؽ�Ʈ ����
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* Txt_Qty;

    UPROPERTY(EditDefaultsOnly, Category="DragDrop")
	TSubclassOf<UUserWidget> DragVisualClass;

public:
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    // 드래그 감지를 위한 마우스 이벤트 오버라이드
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnDragDetected(const FGeometry& Geo, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation) override;


private:
    UPROPERTY()
    TObjectPtr<class UItemListEntryObject> CachedData;
};
