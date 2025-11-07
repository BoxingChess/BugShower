#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplitQuantityDialog.generated.h"

class UTextBlock;
class USpinBox;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuantityConfirmed, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuantityCanceled);

/*
 - Alt+드래그 시 띄우는 “수량 입력” 팝업 위젯
 - 최대치/기본값/제목을 받아 초기화
 - 입력 위젯은 SpinBox(더블 기반)이지만 정수처럼 사용
*/
UCLASS()
class BUGSHOWER_API USplitQuantityDialog : public UUserWidget
{
    GENERATED_BODY()

public:
    // 블루프린트 위젯 바인딩 (이름을 정확히 맞추세요)
    UPROPERTY(meta=(BindWidget)) UTextBlock* Txt_Title;          // "수량을 입력하세요"
    UPROPERTY(meta=(BindWidget)) UTextBlock* Txt_Max;            // 우측의 최대값(예: 99)
    UPROPERTY(meta=(BindWidget)) USpinBox*  Spin_Input;          // 입력(정수로 사용)
    UPROPERTY(meta=(BindWidget)) UButton*   Btn_Ok;
    UPROPERTY(meta=(BindWidget)) UButton*   Btn_Cancel;

    // 결과 이벤트
    // - 외부(InventoryWidget 등)에서 바인딩해, 입력 완료/취소 시 동작을 이어받도록..
    UPROPERTY(BlueprintAssignable) FOnQuantityConfirmed OnConfirmed;
    UPROPERTY(BlueprintAssignable) FOnQuantityCanceled  OnCanceled;

    // 초기화(최대/기본값 세팅)
    UFUNCTION(BlueprintCallable)
    void InitDialog(int32 InMax, int32 InDefault = -1, const FText& InTitle = FText());

protected:

    //UUserWidget 초기화
    virtual bool Initialize() override;

    /*
        키 입력 처리
         Enter → 확인 처리(HandleOk)
         Esc   → 취소 처리(HandleCancel)
         팝업에서 키로 빠르게 확정 / 취소 가능하게 해 UX 개선한다.
    */
    virtual FReply NativeOnKeyDown(const FGeometry& InGeo, const FKeyEvent& InKeyEvent) override;

private:
    /// 버튼/키에 의해 호출되는 내부 핸들러들
    UFUNCTION() void HandleOk();
    UFUNCTION() void HandleCancel();

    int32 MaxQty = 1;
};