// Fill out your copyright notice in the Description page of Project Settings.


#include "BSLobbyInventory.h"
#include "Game/BSGameInstance.h"

//ui
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TileView.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Manager/UIManager/BSUIManager.h"


//debug
#include "Logging/BugShowerLog.h"

//// Click PopUp UI
void UBSClickPopUp::NativeConstruct()
{
	// 버튼 이벤트 바인딩
	if (Select)
	{
		if (InventoryMode == InventoryType::Storage)
		{
			Select->OnClicked.AddDynamic(this, &UBSClickPopUp::OnSelectClicked);
		}
		else if (InventoryMode == InventoryType::SelectedItems)
		{
			Select->OnClicked.AddDynamic(this, &UBSClickPopUp::OnRetrunStorage);
		}
	}

	if (Cancel)
	{
		Cancel->OnClicked.AddDynamic(this, &UBSClickPopUp::OnCancelClicked);
	}

	if (CountingSlider)
	{
		CountingSlider->OnValueChanged.AddDynamic(this, &UBSClickPopUp::OnSliderValueChanged);
	}

	if (EditingSelectCounting)
	{
		EditingSelectCounting->OnTextCommitted.AddDynamic(this, &UBSClickPopUp::OnEditableTextCommitted);
	}
}


void UBSClickPopUp::UpdateDisplay(UBSItemInstance* InData)
{
	if (InData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("ClickUI UpdateDisplay: InData is NULL"));
		return;
	}

	//데이터 저장
	ItemData = InData;

	//아이콘 이미지 설정
	UTexture2D* IconTex = InData->GetItemStaticData()->Icon;
	ItemIcon->SetBrushResourceObject(IconTex);

	//이름 설정
	FText Name = InData->GetItemStaticData()->DisplayName;
	ItemName->SetText(Name);

	//아이템 설명 설정
	FText Descript = InData->GetItemStaticData()->Description;
	ItemDescript->SetText(Descript);

	//최대 수량 설정
	int MaxQuantity = InData->GetItemData().Quantity;
	ItemQuantity->SetText(FText::AsNumber(MaxQuantity));
	CountingSlider->SetMaxValue(MaxQuantity);

	//현재 선택 수량 설정
	int CurQuantity = FCString::Atoi(*EditingSelectCounting->GetText().ToString());
	CountingSlider->SetValue(CurQuantity);
}

void UBSClickPopUp::OnEditableTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
	//엔터나 마우스 포커스 바뀌면 적용
	if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
	{
		//현재 선택 수량 설정
		int NewCount = FCString::Atoi(*InText.ToString());
		int MaxCount = CountingSlider->GetMaxValue();

		NewCount = FMath::Clamp(NewCount, 0, MaxCount);

		//슬라이더 설정
		CountingSlider->SetValue(NewCount);
		//숫자입력 적용
		EditingSelectCounting->SetText(FText::AsNumber(NewCount));
	}
}

void UBSClickPopUp::OnSliderValueChanged(float InValue)
{
	//소수점으로 올라가니까 올림처리
	int NewCount = FMath::RoundToInt(InValue);
	int MaxCount = CountingSlider->GetMaxValue();

	//0~최대 사이로 조정
	NewCount = FMath::Clamp(NewCount, 0, MaxCount);

	//값 설정
	EditingSelectCounting->SetText(FText::AsNumber(NewCount));
	CountingSlider->SetValue(NewCount);
}

void UBSClickPopUp::OnSelectClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: GameInstance is NULL"));
		return;
	}

	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GameInstance);
	if (BSGameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: BSGameInstance is NULL"));
		return;
	}


	if (ItemData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: ItemData is NULL"));
		return;
	}

	//여기에 선택한 아이템을 게임 인스턴스에 전달하는 로직 추가 필요
	int SelectAmount = FCString::Atoi(*EditingSelectCounting->GetText().ToString());
	BSGameInstance->AddItemsToInventoryForGame(ItemData, SelectAmount);
	UpdateDisplay(ItemData);

	UBSUIManager* BSUIManager = GameInstance->GetSubsystem<UBSUIManager>();
	if (BSUIManager == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: BSUIManager is NULL"));
		return;
	}

	BSUIManager->HideWidget(WidgetName);



}

void UBSClickPopUp::OnCancelClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: GameInstance is NULL"));
		return;
	}

	UBSUIManager* BSUIManager = GameInstance->GetSubsystem<UBSUIManager>();
	if (BSUIManager == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: BSUIManager is NULL"));
		return;
	}

	BSUIManager->HideWidget(WidgetName);
}

void UBSClickPopUp::OnRetrunStorage()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: GameInstance is NULL"));
		return;
	}

	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GameInstance);
	if (BSGameInstance == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: BSGameInstance is NULL"));
		return;
	}


	if (ItemData == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: ItemData is NULL"));
		return;
	}

	//여기에 선택한 아이템을 게임 인스턴스에 전달하는 로직 추가 필요
	int SelectAmount = FCString::Atoi(*EditingSelectCounting->GetText().ToString());
	BSGameInstance->AddItemsToStarage(ItemData, SelectAmount);
	UpdateDisplay(ItemData);

	UBSUIManager* BSUIManager = GameInstance->GetSubsystem<UBSUIManager>();
	if (BSUIManager == nullptr)
	{
		LOG_LOGIC_INFO(TEXT("OnSelectClicked: BSUIManager is NULL"));
		return;
	}

	BSUIManager->HideWidget(WidgetName);
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

	//해당 아이템으로 팝업 UI 업데이트
	UUserWidget* Widget = BSUIManager->GetWidget(ClickPopUpUIName);
	UBSClickPopUp* ClickPopUp = Cast<UBSClickPopUp>(Widget);
	if (ClickPopUp == nullptr)
	{
		return;
	}

	ClickPopUp->UpdateDisplay(ItemData);
	BSUIManager->ShowWidget(ClickPopUpUIName);
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



void UBSLobbyInventory::NativeConstruct()
{
	// GameInstance의 인벤토리 변경 델리게이트 구독
	UGameInstance* GameInstance = GetGameInstance();
	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GameInstance);
	if (BSGameInstance)
	{
		if (InventoryMode == InventoryType::Storage)
		{
			BSGameInstance->OnStorageChanged.AddDynamic(this, &UBSLobbyInventory::RefreshInventory);
		}
		else if (InventoryMode == InventoryType::SelectedItems)
		{
			BSGameInstance->OnSelectedItemsChanged.AddDynamic(this, &UBSLobbyInventory::RefreshInventory);
		}
	}
}

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

void UBSLobbyInventory::RefreshInventory(const TArray<UBSItemInstance*>& InItems)
{
	if (!Inventory) return;

	UBSGameInstance* GameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!GameInstance) return;

	// TileView 전체 새로고침
	Inventory->ClearListItems();

	for (UBSItemInstance* Item : InItems)
	{
		if (Item && Item->Dynamic.Quantity > 0)  // 수량이 0보다 큰 것만
		{
			Inventory->AddItem(Item);
		}
	}

	if (InventoryMode == InventoryType::SelectedItems)
	{
		LOG_LOGIC_INFO(TEXT("Refreshed Selected inventory: %d items"), InItems.Num());
	}
	else if (InventoryMode == InventoryType::Storage)
	{
		LOG_LOGIC_INFO(TEXT("Refreshed Storage inventory: %d items"), InItems.Num());
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("       PLAYER SAVE DATA SUMMARY"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Save Slot: %s"), *GameInstance->GetSaveSlotName());
	UE_LOG(LogTemp, Log, TEXT("Player ID: %s"), GameInstance->GetPlayerID().IsEmpty() ? TEXT("(Default)") : *GameInstance->GetPlayerID());
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Total Items: %d"), SavedItems.Num());
	UE_LOG(LogTemp, Log, TEXT("----------------------------------------"));

	if (SavedItems.Num() > 0)
	{
		for (int32 i = 0; i < SavedItems.Num(); ++i)
		{
			const UBSItemInstance* Item = SavedItems[i];
			if (Item && Item->StaticData)
			{
				UE_LOG(LogTemp, Log, TEXT("[%d] %s (ID: %d) - Quantity: %d"),
					i + 1,
					*Item->StaticData->DisplayName.ToString(),
					Item->Dynamic.ItemID,
					Item->Dynamic.Quantity);
			}
			else if (Item)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%d] Unknown Item (ID: %d) - Quantity: %d (StaticData missing)"),
					i + 1,
					Item->Dynamic.ItemID,
					Item->Dynamic.Quantity);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No items found in save data."));
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));

}