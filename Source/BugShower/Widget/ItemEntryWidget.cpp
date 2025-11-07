// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/ItemEntryWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Item/ItemListEntryObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Widget/DragVisual.h"                  // UDragVisual (드래그 비주얼 위젯)

void UItemEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    CachedData = Cast<UItemListEntryObject>(ListItemObject);

    if (CachedData)
    {
        if (Img_Icon)  Img_Icon->SetBrushFromTexture(CachedData->Icon);
        if (Txt_Name)  Txt_Name->SetText(CachedData->Name);
        if (Txt_Qty)   Txt_Qty->SetText(CachedData->Quantity);
        
    }

}


void UItemEntryWidget::NativeOnDragDetected(const FGeometry& Geo, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation)
{
    if (!CachedData) return;

    UDragDropOperation* DragOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
    DragOp->Payload = Cast<UObject>(CachedData.Get());                // 드래그할 EntryObject
    DragOp->DefaultDragVisual = this;            // 드래그 중 보일 비주얼

    // 드래그 전용 위젯 생성
    if (UDragVisual* Visual = CreateWidget<UDragVisual>(GetWorld(), DragVisualClass))
    {
        Visual->SetData(CachedData->Icon, CachedData->Quantity); // 데이터 세팅 함수
        DragOp->DefaultDragVisual = Visual; // 마우스에 붙는 UI
    }

    OutOperation = DragOp;
}
