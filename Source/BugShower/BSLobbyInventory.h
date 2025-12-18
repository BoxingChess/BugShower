// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Item/BSItemInstance.h"

#include "BSLobbyInventory.generated.h"

UCLASS()
class BUGSHOWER_API UBSClickPopUp : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MainUI;

	UPROPERTY(meta = (BindWidget))
	class USizeBox* MainUISize;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* DisplayRegion;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* NameRegion;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemName;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* DescriptRegion;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemDescript;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* QuantityRegion;
	UPROPERTY(meta = (BindWidget))
	class UEditableText* EditingSelectCounting;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemQuantity;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* SliderRegion;
	UPROPERTY(meta = (BindWidget))
	class USlider* CountingSlider;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* ButtonRegion;
	UPROPERTY(meta = (BindWidget))
	class UButton* Select;
	UPROPERTY(meta = (BindWidget))
	class UButton* Cancel;

};


UCLASS()
class BUGSHOWER_API UBSTooltip : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateDisplay(FText InName, FText InDescript, UObject* InIconTex);

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
};




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
	void NativeConstruct() override;


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

private:

	UPROPERTY()
	TArray<UBSTileItem*> SavedItems;

};
