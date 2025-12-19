// Fill out your copyright notice in the Description page of Project Settings.


#include "BSLobbyInventory.h"
#include "Game/BSGameInstance.h"

//ui
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Manager/UIManager/BSUIManager.h"


//debug
#include "Logging/BugShowerLog.h"

//// Click PopUp UI
void UBSClickPopUp::NativeConstruct()
{

}




////// Tootip UI

void UBSTooltip::NativeConstruct()
{
	WidgetName = TEXT("ToolTip");
}



void UBSTooltip::UpdateDisplay(FText InName, FText InDescript, UObject* InIconTex)
{
	if (InIconTex == nullptr || ItemIcon == nullptr || ItemName == nullptr || ItemDescript == nullptr)
	{
		return;
	}

	ItemIcon->SetBrushResourceObject(InIconTex);
	ItemName->SetText(InName);
	ItemDescript->SetText(InDescript);
}

FName UBSTooltip::GetWidgetName() const
{
	return WidgetName;
}

///// Tile Item UI

void UBSTileItem::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩
	if (ItemSelect)
	{
		ItemSelect->OnClicked.AddDynamic(this, &UBSTileItem::OnItemClicked);
		ItemSelect->OnHovered.AddDynamic(this, &UBSTileItem::OnItemHovered);
		ItemSelect->OnUnhovered.AddDynamic(this, &UBSTileItem::OnItemUnHovered);
	}
}

void UBSTileItem::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UBSItemInstance* CurData = Cast<UBSItemInstance>(ListItemObject);
	if (CurData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("ItemData is NULL"));
	}

	ItemData = CurData;

	UpdateDisplay();
}

void UBSTileItem::UpdateDisplay()
{
	if (ItemData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("ItemData is NULL"));
		return;
	}

	//고정된 데이터
	TObjectPtr<const UBSStaticItemDataAsset> StaticData = ItemData->GetItemStaticData();
	FBS_Item DynmicData = ItemData->GetItemData();

	ItemIcon->SetBrushFromTexture(StaticData->Icon);
	ItemName->SetText(StaticData->DisplayName);
}

void UBSTileItem::OnItemClicked()
{
	if (ItemData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnItemClicked: ItemData is NULL"));
		return;
	}

	LOG_LOGIC_INFO(TEXT("Item Clicked: %s"), *ItemData->GetItemStaticData()->DisplayName.ToString());

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnItemClicked: GameInstance is NULL"));
		return;
	}


	UBSUIManager* BSUIManager = GameInstance->GetSubsystem<UBSUIManager>();

	if (BSUIManager == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnItemClicked: BSUIManager is NULL"));
		return;
	}

	BSUIManager->ShowWidget("");
}

void UBSTileItem::OnItemHovered()
{
	if (ItemData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnItemHovered: ItemData is NULL"));
		return;
	}

	LOG_LOGIC_INFO(TEXT("Item Hovered: %s"), *ItemData->GetItemStaticData()->DisplayName.ToString());

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnItemHovered: GameInstance is NULL"));
		return;
	}


	UBSUIManager* BSUIManager = GameInstance->GetSubsystem<UBSUIManager>();

	if (BSUIManager == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnItemHovered: BSUIManager is NULL"));
		return;
	}

	if (ItemSelect->GetToolTip() == nullptr)
	{
		UUserWidget* Widget = BSUIManager->GetWidget("Tooltip");
		UBSTooltip* SharedTooltip = Cast<UBSTooltip>(Widget);

		if (SharedTooltip == nullptr)
		{
			LOG_LOGIC_INFO(TEXT("OnItemHovered: SharedTooltip is NULL"));
			return;
		}

		ItemSelect->SetToolTip(SharedTooltip);

		if (SharedTooltip == nullptr)
		{
			LOG_LOGIC_INFO(TEXT("OnItemHovered: SharedTooltip is NULL"));
			return;
		}

		FText Name = ItemData->StaticData->DisplayName;
		FText Descript = ItemData->StaticData->Description;
		UTexture2D* Icon = ItemData->StaticData->Icon;

		//tootip은 자동 on off됨
		SharedTooltip->UpdateDisplay(Name, Descript, Icon);
	}
	else
	{
		FText Name = ItemData->StaticData->DisplayName;
		FText Descript = ItemData->StaticData->Description;
		UTexture2D* Icon = ItemData->StaticData->Icon;

		UBSTooltip* SharedTooltip = Cast<UBSTooltip>(ItemSelect->GetToolTip());

		if (SharedTooltip == nullptr)
		{
			LOG_LOGIC_INFO(TEXT("OnItemHovered: SharedTooltip is NULL"));
			return;
		}

		//tootip은 자동 on off됨
		SharedTooltip->UpdateDisplay(Name, Descript, Icon);
	}
}



void UBSTileItem::OnItemUnHovered()
{
	
}

///// Inventory UI



void UBSLobbyInventory::InitializeInventory(const TArray<UBSItemInstance*>& InItems)
{

	// TileView 초기화 및 아이템 설정
	Inventory->ClearListItems();
	SavedItems.Empty();

	// 아이템 목록을 TileView에 추가
	for (UBSItemInstance* Item : InItems)
	{
		if (Item != nullptr)
		{
			Inventory->AddItem(Item);
			SavedItems.Add(Item);
		}
	}

	LOG_LOGIC_INFO(TEXT("Loaded %d items to inventory"), SavedItems.Num());
}

void UBSLobbyInventory::SetItemList(TArray<UBSItemInstance*>& InItems)
{
	if (Inventory == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("Inventory TileView is NULL"));
		return;
	}

}

void UBSLobbyInventory::UpdateDisplay()
{

}

