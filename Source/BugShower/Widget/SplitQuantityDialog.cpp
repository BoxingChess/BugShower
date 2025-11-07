#include "Widget/SplitQuantityDialog.h"
#include "Components/TextBlock.h"
#include "Components/SpinBox.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

bool USplitQuantityDialog::Initialize()
{
    const bool bOk = Super::Initialize();
    if (Btn_Ok)     Btn_Ok->OnClicked.AddDynamic(this, &USplitQuantityDialog::HandleOk);
    if (Btn_Cancel) Btn_Cancel->OnClicked.AddDynamic(this, &USplitQuantityDialog::HandleCancel);
    SetIsFocusable(true);
    return bOk;
}

void USplitQuantityDialog::InitDialog(int32 InMax, int32 InDefault, const FText& InTitle)
{
    MaxQty = FMath::Max(1, InMax);

    if (Txt_Title && !InTitle.IsEmpty())
        Txt_Title->SetText(InTitle);

    if (Txt_Max)
        Txt_Max->SetText(FText::AsNumber(MaxQty));

    if (Spin_Input)
    {
        Spin_Input->SetMinValue(1.0);
        Spin_Input->SetMaxValue((double)MaxQty);
        Spin_Input->SetDelta(1.0);
        Spin_Input->SetValue((double)(InDefault > 0 ? FMath::Clamp(InDefault, 1, MaxQty) : MaxQty));
    }

    // 포커스 & 입력 모드(UI Only)
    if (APlayerController* PC = GetOwningPlayer())
    {
        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, this);
        PC->SetShowMouseCursor(true);
        SetKeyboardFocus();
    }
}

void USplitQuantityDialog::HandleOk()
{
    int32 Qty = MaxQty;
    if (Spin_Input)
        Qty = FMath::Clamp((int32)FMath::RoundHalfFromZero(Spin_Input->GetValue()), 1, MaxQty);

    OnConfirmed.Broadcast(Qty);

    // InputMode 원복
    if (APlayerController* PC = GetOwningPlayer())
    {
        UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PC); // or SetInputMode_GameOnly if 인벤토리 닫고싶을 때
        PC->SetShowMouseCursor(false);
    }

    RemoveFromParent();
}

void USplitQuantityDialog::HandleCancel()
{
    OnCanceled.Broadcast();

    if (APlayerController* PC = GetOwningPlayer())
    {
        UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PC);
        PC->SetShowMouseCursor(false);
    }

    RemoveFromParent();
}

FReply USplitQuantityDialog::NativeOnKeyDown(const FGeometry& InGeo, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::Virtual_Accept)
    {
        HandleOk();
        return FReply::Handled();
    }
    if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Virtual_Back)
    {
        HandleCancel();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeo, InKeyEvent);
}