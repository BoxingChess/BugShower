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

}



void UBSTooltip::UpdateDisplay(FText InName, FText InDescript, UObject* InIconTex)
{
	if (InIconTex == nullptr)
	{
		return;
	}

	ItemIcon->SetBrushResourceObject(InIconTex);
	ItemName->SetText(InName);
	ItemDescript->SetText(InDescript);
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



///// Inventory UI


void UBSLobbyInventory::NativeConstruct()
{
	Super::NativeConstruct();

	UGameInstance* GameInstance = GetGameInstance();
	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GameInstance);

	if (BSGameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("BSGameInstance is NULL"));
		return;
	}

	if (Inventory == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("Inventory TileView is NULL"));
		return;
	}
	


	// 게임 인스턴스에서 플레이어 아이템 목록 가져오기
	const TArray<UBSItemInstance*>& ItemList = BSGameInstance->GetPlayerItems();

	// TileView 초기화 및 아이템 설정
	Inventory->ClearListItems();

	// 아이템 목록을 TileView에 추가
	for (UBSItemInstance* Item : ItemList)
	{
		if (Item != nullptr)
		{
			Inventory->AddItem(Item);
		}
	}

	LOG_LOGIC_INFO(TEXT("Loaded %d items to inventory"), ItemList.Num());
}

