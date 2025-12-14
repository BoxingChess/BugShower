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

// ========================================
// 드래그 감지 활성화
// ========================================
FReply UItemEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 마우스 버튼으로 드래그 감지 시작
    // DetectDrag를 호출하면 마우스를 움직일 때 NativeOnDragDetected가 호출됨
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        UE_LOG(LogTemp, Log, TEXT("ItemEntryWidget: Mouse button down - Starting drag detection"));
        return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }

    return FReply::Unhandled();
}


void UItemEntryWidget::NativeOnDragDetected(const FGeometry& Geo, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation)
{
    UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: NativeOnDragDetected called!"));

    if (!CachedData)
    {
        UE_LOG(LogTemp, Error, TEXT("ItemEntryWidget: CachedData is NULL!"));
        return;
    }

    UDragDropOperation* DragOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
    DragOp->Payload = Cast<UObject>(CachedData.Get());                // 드래그할 EntryObject

    // Pivot 설정 (0.5, 0.5 = 중앙, 드래그 비주얼이 마우스 중앙에 위치)
    DragOp->Pivot = EDragPivot::CenterCenter;

    UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: DragVisualClass is %s"),
        DragVisualClass ? *DragVisualClass->GetName() : TEXT("NULL"));

    // 드래그 전용 위젯 생성
    UDragVisual* Visual = nullptr;
    if (DragVisualClass)
    {
        Visual = CreateWidget<UDragVisual>(GetWorld(), DragVisualClass);
        UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: CreateWidget returned %s"),
            Visual ? TEXT("Success") : TEXT("NULL"));
    }

    if (Visual)
    {
        Visual->SetData(CachedData->Icon, CachedData->Quantity); // 데이터 세팅 함수
        DragOp->DefaultDragVisual = Visual; // 마우스에 붙는 UI (DragVisual만 표시)

        UE_LOG(LogTemp, Log, TEXT("ItemEntryWidget: DragVisual created successfully"));
    }
    else
    {
        // DragVisual 생성 실패
        UE_LOG(LogTemp, Error, TEXT("ItemEntryWidget: Failed to create DragVisual! DragVisualClass is %s"),
            DragVisualClass ? TEXT("valid") : TEXT("NULL"));

        // Fallback 없음 - nullptr로 두면 아무것도 안 보임
        DragOp->DefaultDragVisual = nullptr;
    }

    OutOperation = DragOp;
}
