// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/InventoryWidget.h"
#include "Widget/SplitQuantityDialog.h"
#include "Components/ListView.h"
#include "Item/ItemActor.h"
#include "Component/PickUp/PickUpDetectorComponent.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Blueprint/DragDropOperation.h"

void UInventoryWidget::BeginPlay()
{


}

void UInventoryWidget::SetVicinity(const TArray<AActor*>& Actors)
{
	if (!VicinityList) return;

	TArray<UObject*> Rows;
	Rows.Reserve(Actors.Num());

	for (AActor* A : Actors)
	{
		AItemActor* Item = Cast<AItemActor>(A);
		if (!Item) continue;

		// 정적 데이터(아이콘/이름/설명 등)
		const UBSStaticItemDataAsset* Static = Item->GetItemStaticData(); // 네 Getter명에 맞춰 변경
		if (!Static) continue;

		// 수량(인스턴스 데이터) — 네 프로젝트 변수/함수로 교체
		const int32 StackCount = Item->GetItemData().Quantity; // 없으면 1로 두거나 네 변수 사용

		Rows.Add(MakeRow(this, Static->Icon, Static->DisplayName, StackCount, Item, nullptr));
	}

	//VicinityList->ClearListItems();
	VicinityList->SetListItems(Rows);

	UE_LOG(LogTemp, Warning, TEXT("ListItems.Num = %d"), VicinityList->GetListItems().Num());
	UE_LOG(LogTemp, Warning, TEXT("VisibleEntries.Num = %d"), VicinityList->GetDisplayedEntryWidgets().Num());
}

void UInventoryWidget::SetInventory(const TArray<UBSItemInstance*>& InventoryItems)
{
	if (!InventoryList) return;

	TArray<UObject*> Rows;
	Rows.Reserve(InventoryItems.Num());

	for (auto& Item : InventoryItems)
	{

		// 정적 데이터(아이콘/이름/설명 등)
		const UBSStaticItemDataAsset* Static = Item->GetItemStaticData(); // 네 Getter명에 맞춰 변경
		if (!Static) continue;

		// 수량(인스턴스 데이터) — 네 프로젝트 변수/함수로 교체
		const int32 StackCount = Item->GetItemData().Quantity; // 없으면 1로 두거나 네 변수 사용

		Rows.Add(MakeRow(this, Static->Icon, Static->DisplayName, StackCount, nullptr, Item));
	}

	//VicinityList->ClearListItems();
	InventoryList->SetListItems(Rows);

	UE_LOG(LogTemp, Warning, TEXT("ListItems.Num = %d"), InventoryList->GetListItems().Num());
	UE_LOG(LogTemp, Warning, TEXT("VisibleEntries.Num = %d"), InventoryList->GetDisplayedEntryWidgets().Num());

}


// 한 줄 데이터 만들기(원본 액터 → 뷰모델)
UItemListEntryObject* UInventoryWidget::MakeRow(UObject* Outer, UTexture2D* Icon, const FText& Name, int32 Qty, AItemActor* Source, UBSItemInstance* SourceInstance)
{
	//인벤토리가 해당 데이터를 소유하게끔.. 
	UItemListEntryObject* Row = NewObject<UItemListEntryObject>(Outer);

	Row->Icon = Icon;
	Row->Name = Name;
	Row->Quantity = (Qty > 1)
		? FText::Format(FText::FromString(TEXT("x{0}")), FText::AsNumber(Qty))  //수량이 1보다 클시 x3같이 보이게끔
		: FText();

	Row->SourceActor = Source; //원본액터도 저장
	Row->SourceInstance = SourceInstance; //원본액터도 저장

	//UE_LOG(LogTemp, Error, TEXT("%s"), *Row->Name.ToString());
	return Row;
}

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UItemListEntryObject* DroppedObj = Cast<UItemListEntryObject>(InOperation->Payload))
	{
		UE_LOG(LogTemp, Log, TEXT("드랍된 아이템: %s"), *DroppedObj->Name.ToString());

		const bool bAltPressed = InDragDropEvent.GetModifierKeys().IsAltDown();

		///Alt 눌림 -> 수량입력 UI 띄우기 -> 확인 혹은 엔터 눌렸을시 줍거나 버리는 로직 실행되게끔
		if (bAltPressed)
		{
			USplitQuantityDialog* Dialog = CreateWidget<USplitQuantityDialog>(GetWorld(), SplitDialogClass);
			if (Dialog)
			{
				//Vicinity(월드 액터)에서 온 경우: 액터의 현재 수량을 최대치로, 
				//Inventory(인벤토리 인스턴스)에서 온 경우: 인스턴스의 수량을 최대치로
				//둘다 아니면 1
				int32 MaxQty = DroppedObj->SourceActor.IsValid()
					? DroppedObj->SourceActor->GetItemData().Quantity
					: (DroppedObj->SourceInstance ? DroppedObj->SourceInstance->Dynamic.Quantity : 1);

				//다이얼로그 초기화(최대치 / 기본값(= 절반) / 제목)
				Dialog->InitDialog(MaxQty, MaxQty / 2, FText::FromString(TEXT("수량을 입력하시오..")));

				//확인 / 취소 콜백을 현재 위젯의 핸들러와 연결
				Dialog->OnConfirmed.AddDynamic(this, &UInventoryWidget::HandleSplitConfirm);
				Dialog->OnCanceled.AddDynamic(this, &UInventoryWidget::HandleSplitCancel);

				//다이얼로그를 화면에 표시, 100으로 세팅해서 가장위에 띄워지게끔..
				Dialog->AddToViewport(100);

				//어떤 엔트리를 분할하려는지 나중 콜백에서 알 수 있도록 보관
				PendingSplitObj = DroppedObj;
				// PendingMode = Pickup or Drop … 로 표시

				//여기서 분기 모드 세팅
				if (DroppedObj->SourceActor.IsValid())
				{
					PendingMode = ESplitMode::VicinityToInventory;
				}
				else if (DroppedObj->SourceInstance)
				{
					PendingMode = ESplitMode::InventoryToVicinity;
				}
				else
				{
					PendingMode = ESplitMode::None;
				}
			}
			return true; // 여기서는 실제 아이템 이동 안 하고 UI만 띄움
		}

		///Alt 안눌림 -> 바로 먹거나 버린다.
		else
		{

			if (APawn* OwnerPawn = GetOwningPlayerPawn())       // 이 위젯을 소유한 플레이어의 Pawn 가져오기
			{
				UPickUpDetectorComponent* PickUp = OwnerPawn->FindComponentByClass<UPickUpDetectorComponent>();
				UInventoryComponent* Inv = OwnerPawn->FindComponentByClass<UInventoryComponent>();

				// Vicinity → Inventory(줍기)
				if (AItemActor* Source = DroppedObj->SourceActor.Get())     // EntryObject에 월드 액터가 들어있으면
				{
					if (PickUp && Inv)
					{
						PickUp->ServerTryPickup(Source);
						SetInventory(Inv->GetItemInventory());
						PickUp->RefreshNearbyList(1);
					}
					return true;
				}

				// Inventory → Vicinity (버리기)
				if (UBSItemInstance* SourceInstance = DroppedObj->SourceInstance)  // EntryObject에 인벤토리 아이템이 들어있으면
				{
					// 플레이어 앞쪽 100cm 지점에 아이템을 떨어뜨리기 위한 위치 계산
					FVector DropLocation = OwnerPawn->GetActorLocation() + OwnerPawn->GetActorForwardVector() * 100.f;

					if (UWorld* World = GetWorld())
					{
						Inv->DiscardItem(SourceInstance);
						SetInventory(Inv->GetItemInventory());

					}
					return true;    // 드랍 처리 성공
				}
			}
		}
	}
	return false;


}

void UInventoryWidget::HandleSplitConfirm(int32 Quantity)
{
	if (!PendingSplitObj.IsValid()) { PendingMode = ESplitMode::None; return; }

	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		UInventoryComponent* Inv = OwnerPawn->FindComponentByClass<UInventoryComponent>();
		UPickUpDetectorComponent* PickUp = OwnerPawn->FindComponentByClass<UPickUpDetectorComponent>();
		if (!Inv) { PendingSplitObj.Reset(); PendingMode = ESplitMode::None; return; }

		switch (PendingMode)
		{
			case ESplitMode::VicinityToInventory:
			{
				// 주변(월드) → 인벤토리 : 일부만 줍기
				if (AItemActor* Src = PendingSplitObj->SourceActor.Get())
				{
					// 현재 Inv/Detector가 “부분 줍기” API를 지원하지 않으면
					// 아래 한 줄은 전량 줍기로 동작합니다. (TODO: Partial RPC/함수 추가 필요)
					if (PickUp) PickUp->ServerTryPickupPartial(Src, Quantity);

					// 부분 줍기를 구현하려면:
					// 1) ServerTryPickupPartial(AItemActor* Src, int32 Qty) RPC 추가
					// 2) 월드 액터에서 Qty만큼 감소 & 인벤토리에 Qty만큼 누적
					// 3) 여기서 그 RPC 호출
					SetInventory(Inv->GetItemInventory());
					if (PickUp) PickUp->RefreshNearbyList(true);
				}
			}
			break;

			case ESplitMode::InventoryToVicinity:
			{
				// 인벤토리 → 주변 : 일부 버리기
				if (UBSItemInstance* Inst = PendingSplitObj->SourceInstance)
				{
					// UInventoryComponent에 DiscardItem(Inst, Quantity) 오버로드가 필요합니다.
					Inv->DiscardItem(Inst, Quantity);
					SetInventory(Inv->GetItemInventory());
				}
			}
			break;

			default: break;
		}
	}

	PendingSplitObj.Reset();
	PendingMode = ESplitMode::None;
}

void UInventoryWidget::HandleSplitCancel()
{
	PendingSplitObj.Reset();
	PendingMode = ESplitMode::None;
}