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
#include "Components/ListViewBase.h"
#include "Components/Border.h"
#include "Manager/UIManager/BSUIManager.h"

// 드래그 앤 드롭
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Widget/DragVisual.h"

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

	// Border 설정 (클릭 가능하도록)
	if (ItemSelect)
	{
		ItemSelect->SetVisibility(ESlateVisibility::Visible);
	}

	// 위젯 자체도 Visible로 설정하여 마우스 이벤트 받기
	SetVisibility(ESlateVisibility::Visible);
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

	// StaticData가 nullptr인 경우 (테스트용 더미 데이터)
	if (StaticData == nullptr)
	{
		// 기본 텍스트 표시
		ItemName->SetText(FText::FromString(FString::Printf(TEXT("Test Item #%d"), DynmicData.ItemID)));
		// 아이콘은 비워둠
		ItemIcon->SetBrushFromTexture(nullptr);
	}
	else
	{
		ItemIcon->SetBrushFromTexture(StaticData->Icon);
		ItemName->SetText(StaticData->DisplayName);
	}
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
	if (ItemData == nullptr || ItemData->StaticData == nullptr)
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

// ========================================
// 마우스 이벤트 - 클릭과 드래그 구분
// ========================================

FReply UBSTileItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 마우스 누름 시작 - 위치 저장
		bIsPressed = true;
		bIsDragging = false;
		PressedMousePosition = InMouseEvent.GetScreenSpacePosition();

		LOG_LOGIC_INFO(TEXT("TileItem: Mouse pressed at (%.1f, %.1f)"),
			PressedMousePosition.X, PressedMousePosition.Y);

		// 마우스 캡처 (Move 이벤트를 받기 위해)
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UBSTileItem::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsPressed)
	{
		bIsPressed = false;

		// 드래그가 시작되지 않았으면 → 클릭으로 판정
		if (!bIsDragging)
		{
			LOG_LOGIC_INFO(TEXT("TileItem: Clicked (no drag detected)"));
			OnItemClicked();  // 기존 클릭 이벤트 호출
		}
		else
		{
			LOG_LOGIC_INFO(TEXT("TileItem: Drag completed"));
		}

		bIsDragging = false;

		// 마우스 캡처 해제
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UBSTileItem::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsPressed && !bIsDragging)
	{
		// 마우스가 눌린 상태에서 이동 거리 계산
		FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
		float Distance = FVector2D::Distance(PressedMousePosition, CurrentMousePosition);

		// 임계값 이상 움직이면 드래그 시작
		if (Distance >= DragThreshold)
		{
			bIsDragging = true;
			LOG_LOGIC_INFO(TEXT("TileItem: Drag started (moved %.1f pixels)"), Distance);

			// 드래그 시작 신호
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UBSTileItem::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// 호버 시작 - 기존 호버 이벤트 호출
	OnItemHovered();
}

void UBSTileItem::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	// 호버 끝 - 기존 언호버 이벤트 호출
	OnItemUnHovered();

	// 마우스가 위젯을 벗어나면 상태 초기화
	if (bIsPressed && !bIsDragging)
	{
		bIsPressed = false;
	}
}

// ========================================
// 드래그 앤 드롭
// ========================================

void UBSTileItem::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemData)
	{
		LOG_LOGIC_INFO(TEXT("TileItem: NativeOnDragDetected - ItemData is NULL"));
		return;
	}

	LOG_LOGIC_INFO(TEXT("TileItem: Drag detected for item"));

	// DragDropOperation 생성
	UDragDropOperation* DragOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
	if (!DragOp)
	{
		LOG_LOGIC_INFO(TEXT("TileItem: Failed to create DragDropOperation"));
		return;
	}

	// Payload에 아이템 데이터 저장
	DragOp->Payload = ItemData;
	DragOp->Pivot = EDragPivot::CenterCenter;

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
		Visual->SetData(ItemData->StaticData->Icon, FText::AsNumber(ItemData->Dynamic.Quantity)); // 데이터 세팅 함수
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

bool UBSTileItem::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// 드롭된 아이템 가져오기
	UBSItemInstance* DroppedItem = Cast<UBSItemInstance>(InOperation->Payload);
	if (!DroppedItem || !ItemData)
	{
		LOG_LOGIC_INFO(TEXT("TileItem: NativeOnDrop - Invalid item data"));
		return false;
	}

	// 같은 아이템이면 무시
	if (DroppedItem == ItemData)
	{
		LOG_LOGIC_INFO(TEXT("TileItem: Dropped on same item, ignoring"));
		bIsDraggedOver = false;
		return false;
	}

	// 부모 인벤토리 찾기 (GetTypedOuter로 부모 위젯 찾기)
	UBSLobbyInventory* ParentInventory = nullptr;
	UWidget* Current = GetParent();
	while (Current)
	{
		ParentInventory = Cast<UBSLobbyInventory>(Current);
		if (ParentInventory)
		{
			break;
		}
		Current = Current->GetParent();
	}

	if (!ParentInventory)
	{
		LOG_LOGIC_INFO(TEXT("TileItem: Could not find parent inventory"));
		bIsDraggedOver = false;
		return false;
	}

	// 인덱스 찾기
	int32 DroppedIndex = ParentInventory->GetItemIndex(DroppedItem);
	int32 CurrentIndex = ParentInventory->GetItemIndex(ItemData);

	if (DroppedIndex == -1 || CurrentIndex == -1)
	{
		LOG_LOGIC_INFO(TEXT("TileItem: Could not find item indices"));
		bIsDraggedOver = false;
		return false;
	}

	// Swap 실행
	ParentInventory->SwapItems(DroppedIndex, CurrentIndex);
	LOG_LOGIC_INFO(TEXT("TileItem: Swapped items at index %d and %d"), DroppedIndex, CurrentIndex);

	bIsDraggedOver = false;
	return true;
}

void UBSTileItem::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	bIsDraggedOver = true;

	// 시각적 피드백 (배경색 변경 등)
	if (BackGround)
	{
		BackGround->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.5f, 1.0f)); // 노란색 틴트
	}

	LOG_LOGIC_INFO(TEXT("TileItem: Drag entered"));
}

void UBSTileItem::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	bIsDraggedOver = false;

	// 시각적 피드백 원복
	if (BackGround)
	{
		BackGround->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)); // 원래 색상
	}

	LOG_LOGIC_INFO(TEXT("TileItem: Drag left"));
}

///// Inventory UI



void UBSLobbyInventory::NativeConstruct()
{
	Super::NativeConstruct();

	// TileView 스크롤 설정
	if (Inventory)
	{
		// 스크롤바가 필요할 때 자동으로 표시되도록 설정
		Inventory->SetScrollbarVisibility(ESlateVisibility::Visible);

		// 마우스 휠 스크롤 속도 설정
		Inventory->SetWheelScrollMultiplier(WheelScrollMultiplier);

		// 타일 크기 설정
		Inventory->SetEntryWidth(EntryWidth);
		Inventory->SetEntryHeight(EntryHeight);
	}

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

void UBSLobbyInventory::AddTestItems(int32 ItemCount)
{
	if (!Inventory)
	{
		LOG_LOGIC_INFO(TEXT("AddTestItems: Inventory TileView is NULL"));
		return;
	}

	// 기존 아이템 클리어
	Inventory->ClearListItems();
	SavedItems.Empty();

	// 더미 아이템 생성
	for (int32 i = 0; i < ItemCount; ++i)
	{
		UBSItemInstance* TestItem = NewObject<UBSItemInstance>(this);
		if (TestItem)
		{
			// 더미 데이터 설정
			TestItem->Dynamic.ItemID = i;
			TestItem->Dynamic.ItemType = EItemType::Consumable; // 기본 타입
			TestItem->Dynamic.Quantity = FMath::RandRange(1, 99);

			// StaticData는 nullptr로 두거나, 존재하는 데이터를 사용
			// (실제 테스트에서는 Blueprint에서 StaticData를 설정하거나 기본 데이터를 사용해야 함)
			TestItem->StaticData = nullptr;

			Inventory->AddItem(TestItem);
			SavedItems.Add(TestItem);
		}
	}

	LOG_LOGIC_INFO(TEXT("Added %d test items to inventory"), ItemCount);
}

void UBSLobbyInventory::SwapItems(int32 IndexA, int32 IndexB)
{
	if (!Inventory)
	{
		LOG_LOGIC_INFO(TEXT("SwapItems: Inventory TileView is NULL"));
		return;
	}

	// 인덱스 유효성 검사
	if (IndexA < 0 || IndexA >= SavedItems.Num() || IndexB < 0 || IndexB >= SavedItems.Num())
	{
		LOG_LOGIC_INFO(TEXT("SwapItems: Invalid index (A:%d, B:%d, Max:%d)"), IndexA, IndexB, SavedItems.Num() - 1);
		return;
	}

	// 같은 인덱스면 무시
	if (IndexA == IndexB)
	{
		return;
	}

	// SavedItems 배열에서 swap
	SavedItems.Swap(IndexA, IndexB);

	// TileView 새로고침
	RefreshInventory(SavedItems);

	LOG_LOGIC_INFO(TEXT("Swapped items at index %d and %d"), IndexA, IndexB);
}

void UBSLobbyInventory::MoveItem(int32 FromIndex, int32 ToIndex)
{
	if (!Inventory)
	{
		LOG_LOGIC_INFO(TEXT("MoveItem: Inventory TileView is NULL"));
		return;
	}

	// 인덱스 유효성 검사
	if (FromIndex < 0 || FromIndex >= SavedItems.Num() || ToIndex < 0 || ToIndex >= SavedItems.Num())
	{
		LOG_LOGIC_INFO(TEXT("MoveItem: Invalid index (From:%d, To:%d, Max:%d)"), FromIndex, ToIndex, SavedItems.Num() - 1);
		return;
	}

	// 같은 인덱스면 무시
	if (FromIndex == ToIndex)
	{
		return;
	}

	// 아이템을 배열에서 제거하고 새 위치에 삽입
	UBSItemInstance* ItemToMove = SavedItems[FromIndex];
	SavedItems.RemoveAt(FromIndex);
	SavedItems.Insert(ItemToMove, ToIndex);

	// TileView 새로고침
	RefreshInventory(SavedItems);

	LOG_LOGIC_INFO(TEXT("Moved item from index %d to %d"), FromIndex, ToIndex);
}

int32 UBSLobbyInventory::GetItemIndex(UBSItemInstance* Item) const
{
	if (!Item)
	{
		return -1;
	}

	// SavedItems 배열에서 아이템의 인덱스 찾기
	return SavedItems.Find(Item);
}