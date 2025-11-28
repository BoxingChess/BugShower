// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/ItemListEntryObject.h"
#include "InventoryWidget.generated.h"
/**
 * 
 */

// 분기 모드
UENUM()
enum class ESplitMode : uint8
{
    None,
    VicinityToInventory,   // 주변(월드) → 인벤토리 : 줍기(일부)
    InventoryToVicinity    // 인벤토리 → 주변 : 버리기(일부)
};

class UListView;                    // 전방선언
class USplitQuantityDialog;

UCLASS()
class BUGSHOWER_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void BeginPlay();

public:
	UPROPERTY(meta = (BindWidget))
	UListView* VicinityList;

	UPROPERTY(meta = (BindWidget))
	UListView* InventoryList;
	
    // 에디터에서 WBP_SplitQuantityDialog 지정
    UPROPERTY(EditDefaultsOnly, Category="DragDrop")
    TSubclassOf<USplitQuantityDialog> SplitDialogClass;

public:
	//주변 아이템을 세팅한다.
	UFUNCTION(BlueprintCallable) 
	void SetVicinity(const TArray<AActor*>& Rows);

	//인벤토리 아이템을 세팅한다.
    UFUNCTION(BlueprintCallable) 
	void SetInventory(const TArray<UBSItemInstance*>& InventoryItems);

private:
	UItemListEntryObject* MakeRow(UObject* Outer, UTexture2D* Icon, const FText& Name, int32 Qty, AItemActor* Source, UBSItemInstance* SourceInstance);

public:
	//드래그 앤 드랍에서 드랍이 발생했을 때 실행되는 함수.
	//InGeometry : 이 위젯의 화면 상 위치 / 크기 정보.드랍 좌표 계산 등에 사용할 수 있음.
	//InDragDropEvent : 드래그 상태(마우스 좌표 등)가 담긴 이벤트.
	//InOperation : 드래그 시작 시 생성된 UDragDropOperation 객체.여기 안의 Payload에 드래그한 실제 데이터가 들어 있음.
	//반환값 bool : 드랍을 처리했으면 true, 무시하면 false.
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	private:
    // Alt-드랍 시 컨텍스트 보관
    UPROPERTY() 
	TWeakObjectPtr<UItemListEntryObject> PendingSplitObj = nullptr;

    UPROPERTY() 
	ESplitMode PendingMode = ESplitMode::None;

    UFUNCTION() 
	void HandleSplitConfirm(int32 Quantity);

    UFUNCTION() 
	void HandleSplitCancel();

};
