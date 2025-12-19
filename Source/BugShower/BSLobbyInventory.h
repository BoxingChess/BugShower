// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Item/BSItemInstance.h"

#include "BSLobbyInventory.generated.h"

//임시로 분기나누는 용도
UENUM(BlueprintType)
enum class InventoryType : uint8
{
	Storage,
	SelectedItems
};


//버튼 클릭시 나타나는 팝업창 ui class
UCLASS()
class BUGSHOWER_API UBSClickPopUp : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateDisplay(class UBSItemInstance* InData);

	UFUNCTION(BlueprintCallable)
	void SetWidgetName(FName InName) { WidgetName = InName; }

	UFUNCTION(BlueprintCallable)
	void GetWidgetName() const { WidgetName; };

	//수량을 직접 입력하고 엔터 누르면 적용
	UFUNCTION(BlueprintCallable)
	void OnEditableTextCommitted(const FText& InText, ETextCommit::Type CommitMethod);

	//슬라이더를 움직여 수량 조절
	UFUNCTION(BlueprintCallable)
	void OnSliderValueChanged(float InValue);

protected:
	//버튼 이벤트 핸들러
	UFUNCTION()
	void OnSelectClicked();
	UFUNCTION()
	void OnCancelClicked();

	//버튼 이벤트 핸들러
	UFUNCTION()
	void OnRetrunStorage();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	InventoryType InventoryMode;

protected:
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MainUI;

	UPROPERTY(meta = (BindWidget))
	class USizeBox* MainUISize;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* DisplayRegion;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* NameRegion;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ItemName;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* DescriptRegion;
	
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* QuantityRegion;
	UPROPERTY(meta = (BindWidget))
	class UEditableText* EditingSelectCounting;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ItemQuantity;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* SliderRegion;
	UPROPERTY(BlueprintReadWrite,meta = (BindWidget))
	class USlider* CountingSlider;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* ButtonRegion;
	UPROPERTY(meta = (BindWidget))
	class UButton* Select;
	UPROPERTY(meta = (BindWidget))
	class UButton* Cancel;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "UI")
	FName WidgetName;
private:

	UPROPERTY()
	UBSItemInstance* ItemData;

};


//버튼위에 마우스 hover시 나타나는 툴팁 ui class
UCLASS()
class BUGSHOWER_API UBSTooltip : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateDisplay(FText InName, FText InDescript, UObject* InIconTex);
	FName GetWidgetName() const;

protected:
	UPROPERTY(meta = (BindWidget))
	class USizeBox* MainUISize;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* DisplayRegion;

	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemName;	

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemDescript;

private:
	FName WidgetName;
	
};



//*
//로비에서 플레이어가 보는 개별 아이템에 대한 tile ui class
//*/
UCLASS()
class BUGSHOWER_API UBSTileItem : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION(BlueprintCallable)
	void UpdateDisplay();

protected:
	UPROPERTY(meta = (BindWidget))
	class USizeBox* MainUISize;
	UPROPERTY(meta = (BindWidget))
	class UButton* ItemSelect;
	UPROPERTY(meta = (BindWidget))
	class UOverlay* DisplayRegion;
	UPROPERTY(meta = (BindWidget))
	class UImage* BackGround;
	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* NameSize;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemName;

	// 버튼 이벤트 핸들러
	UFUNCTION()
	void OnItemClicked();

	UFUNCTION()
	void OnItemHovered();

	UFUNCTION()
	void OnItemUnHovered();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ClickPopUpUIName;

private:
	UPROPERTY()
	class UBSItemInstance* ItemData;
};





/**
 *로비에서 플레이어가 아이템을 파악하기위한 tile view ui class
 */
UCLASS()
class BUGSHOWER_API UBSLobbyInventory : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;

	//블루프린트 그래프에서 게임 인스턴스를 통해 원하는 아이템 목록 가져와야함
	UFUNCTION(BlueprintCallable,Category = "Inventory")
	void InitializeInventory(const TArray<UBSItemInstance*>& InItems);

	UFUNCTION(BlueprintCallable)
	void SetItemList(TArray<UBSItemInstance*>& InItems);

	UFUNCTION(BlueprintCallable)
	void UpdateDisplay();

public:
      // 인벤토리 새로고침
      UFUNCTION(BlueprintCallable, Category = "Inventory")
      void RefreshInventory(const TArray<UBSItemInstance*>& InItems);

protected:
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MainUI;
	UPROPERTY(meta = (BindWidget))
	class USizeBox* MainUISize;
	UPROPERTY(meta = (BindWidget))
	class UOverlay* DisplayRegion;
	UPROPERTY(meta = (BindWidget))
	class UImage* BackGround;
	UPROPERTY(meta = (BindWidget))
	class UTileView* Inventory;
	UPROPERTY(meta = (BindWidget))
	class UButton* Close;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	InventoryType InventoryMode;

	UPROPERTY(BlueprintReadWrite)
	TArray<UBSItemInstance*> SavedItems;
private:

};
