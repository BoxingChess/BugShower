// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/ItemEntryWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Item/ItemListEntryObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Widget/DragVisual.h"                  // UDragVisual (드래그 비주얼 위젯)
#include "Widget/InventoryWidget.h"             // InventoryWidget (우클릭 사용용)

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
// 드래그 감지 활성화 & 우클릭 사용
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

    // 우클릭으로 아이템 사용
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        UE_LOG(LogTemp, Log, TEXT("ItemEntryWidget: Right mouse button - Using item"));

        if (!CachedData)
        {
            UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: CachedData is invalid!"));
            return FReply::Handled();
        }

        // SourceInstance 가져오기 (인벤토리 아이템만 사용 가능)
        UBSItemInstance* ItemInstance = CachedData->SourceInstance;
        if (!ItemInstance)
        {
            UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: Not an inventory item (SourceInstance is null)"));
            return FReply::Handled();
        }

        // StaticData 확인 및 사용 가능 여부 체크
        if (!ItemInstance->StaticData)
        {
            UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: StaticData is null!"));
            return FReply::Handled();
        }

        // 사용 가능한 아이템인지 체크 (힐/에블라 아이템만)
        if (!ItemInstance->StaticData->IsUsableItem())
        {
            UE_LOG(LogTemp, Log, TEXT("ItemEntryWidget: Item cannot be used directly (Name: %s, ID: %d)"),
                *ItemInstance->StaticData->DisplayName.ToString(),
                ItemInstance->StaticData->ItemID);
            return FReply::Handled();
        }

        // 부모 InventoryWidget 찾기
        UInventoryWidget* InventoryWidget = GetTypedOuter<UInventoryWidget>();
        if (!InventoryWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("ItemEntryWidget: InventoryWidget not found!"));
            return FReply::Handled();
        }

        // 아이템 사용
        InventoryWidget->UseInventoryItem(ItemInstance);

        return FReply::Handled();
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
