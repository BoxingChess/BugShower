// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/InventoryWidget.h"
#include "Widget/SplitQuantityDialog.h"
#include "Components/ListView.h"
#include "Item/ItemActor.h"
#include "Component/PickUp/PickUpDetectorComponent.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Blueprint/DragDropOperation.h"

// 3D 프리뷰 관련
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] NativeConstruct START"));

	// PickUpDetectorComponent 찾아서 델리게이트 바인딩
	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] GetOwningPlayerPawn returned NULL"));
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] Cannot setup 3D preview"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] OwnerPawn found: %s"), *OwnerPawn->GetName());

	if (true)  // 블록 유지
	{
		if (UPickUpDetectorComponent* PickUp = OwnerPawn->FindComponentByClass<UPickUpDetectorComponent>())
		{
			CachedPickUpDetector = PickUp;

			// 델리게이트 바인딩 - 주변 아이템이 갱신될 때마다 VicinityList 업데이트
			PickUp->OnRefreshNearbyList.AddUniqueDynamic(this, &UInventoryWidget::OnNearbyItemsRefreshed);

			UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Bound to OnRefreshNearbyList delegate"));

			// 초기 주변 아이템 목록 로드
			OnNearbyItemsRefreshed();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InventoryWidget: Could not find PickUpDetectorComponent"));
		}

		// InventoryComponent 찾아서 델리게이트 바인딩
		if (UInventoryComponent* Inventory = OwnerPawn->FindComponentByClass<UInventoryComponent>())
		{
			CachedInventory = Inventory;

			// 델리게이트 바인딩 - 인벤토리가 변경될 때마다 InventoryList 업데이트
			Inventory->OnInventoryChanged.AddUniqueDynamic(this, &UInventoryWidget::OnInventoryItemsChanged);

			UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Bound to OnInventoryChanged delegate"));

			// 초기 인벤토리 데이터 로드
			OnInventoryItemsChanged();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InventoryWidget: Could not find InventoryComponent"));
		}

		// 3D 캐릭터 프리뷰 씬 초기화
		UE_LOG(LogTemp, Warning, TEXT("========================================"));
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Calling SetupPreviewScene"));
		UE_LOG(LogTemp, Warning, TEXT("========================================"));
		SetupPreviewScene();
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] SetupPreviewScene returned"));
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] NativeConstruct COMPLETE"));
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Auto-rotate camera around character
	if (SceneCapture)
	{
		// Rotation speed: 20 degrees per second
		float RotationSpeed = 20.0f;
		CurrentYaw += RotationSpeed * InDeltaTime;

		// Keep Yaw in 0-360 range
		if (CurrentYaw >= 360.0f)
		{
			CurrentYaw -= 360.0f;
		}

		// Update camera position
		UpdateCameraPosition(CurrentYaw);
	}
}

void UInventoryWidget::NativeDestruct()
{
	// 델리게이트 언바인딩 (메모리 누수 방지)
	if (CachedPickUpDetector.IsValid())
	{
		CachedPickUpDetector->OnRefreshNearbyList.RemoveDynamic(this, &UInventoryWidget::OnNearbyItemsRefreshed);
		UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Unbound from OnRefreshNearbyList delegate"));
	}

	if (CachedInventory.IsValid())
	{
		CachedInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::OnInventoryItemsChanged);
		UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Unbound from OnInventoryChanged delegate"));
	}

	// 3D 프리뷰 씬 정리
	CleanupPreviewScene();

	Super::NativeDestruct();
}

void UInventoryWidget::OnNearbyItemsRefreshed()
{
	if (!CachedPickUpDetector.IsValid())
	{
		return;
	}

	// PickUpDetectorComponent에서 현재 주변 아이템 목록 가져오기
	const TArray<AActor*>& NearbyItems = CachedPickUpDetector->GetNearbyItems();

	UE_LOG(LogTemp, Log, TEXT("InventoryWidget: OnNearbyItemsRefreshed - Updating VicinityList with %d items"), NearbyItems.Num());

	// VicinityList 업데이트
	SetVicinity(NearbyItems);
}

void UInventoryWidget::OnInventoryItemsChanged()
{
	if (!CachedInventory.IsValid())
	{
		return;
	}

	// InventoryComponent에서 현재 인벤토리 아이템 목록 가져오기
	const TArray<UBSItemInstance*>& InventoryItems = CachedInventory->GetItemInventory();

	UE_LOG(LogTemp, Log, TEXT("InventoryWidget: OnInventoryItemsChanged - Updating InventoryList with %d items"), InventoryItems.Num());

	// InventoryList 업데이트
	SetInventory(InventoryItems);
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

		const UBSStaticItemDataAsset* Static = Item->GetItemStaticData();
		if (!Static) continue;

		const int32 StackCount = Item->GetItemData().Quantity;

		Rows.Add(MakeRow(this, Static->Icon, Static->DisplayName, StackCount, Item, nullptr));
	}

	VicinityList->SetListItems(Rows);
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

		// [추가] 드롭 위치 확인
		// 1 = VicinityList 위, 2 = InventoryList 위, 0 = 알 수 없음
		int32 DropTarget = GetDropTargetWidget(InGeometry, InDragDropEvent);

		// [추가] 드래그 소스 확인
		// SourceActor가 있으면 Vicinity에서 온 것
		// SourceInstance가 있으면 Inventory에서 온 것
		bool bFromVicinity = DroppedObj->SourceActor.IsValid();
		bool bFromInventory = (DroppedObj->SourceInstance != nullptr);

		// [추가] 유효성 검사: 잘못된 드롭 방지
		// - Vicinity 아이템을 Vicinity에 드롭: 무시
		// - Inventory 아이템을 Inventory에 드롭: 무시
		// - 알 수 없는 위치에 드롭: 무시
		if (DropTarget == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("드롭 실패: 유효하지 않은 드롭 위치"));
			return false;
		}

		if (bFromVicinity && DropTarget == 1) // Vicinity → Vicinity
		{
			UE_LOG(LogTemp, Warning, TEXT("드롭 실패: Vicinity 아이템을 Vicinity에 드롭할 수 없음"));
			return false;
		}

		if (bFromInventory && DropTarget == 2) // Inventory → Inventory
		{
			UE_LOG(LogTemp, Warning, TEXT("드롭 실패: Inventory 아이템을 Inventory에 드롭할 수 없음"));
			return false;
		}

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

				// [수정] 드롭 위치도 함께 저장
				if (bFromVicinity && DropTarget == 2) // Vicinity → Inventory
				{
					PendingMode = ESplitMode::VicinityToInventory;
				}
				else if (bFromInventory && DropTarget == 1) // Inventory → Vicinity
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

				// [수정] Vicinity → Inventory (줍기)
				// 조건: SourceActor 있음 && InventoryList에 드롭
				if (AItemActor* Source = DroppedObj->SourceActor.Get())     // EntryObject에 월드 액터가 들어있으면
				{
					if (DropTarget == 2) // InventoryList에 드롭
					{
						if (PickUp && Inv)
						{
							// 서버에서 처리 후 자동으로 리플리케이트되어 OnInventoryChanged가 호출됨
							PickUp->ServerTryPickup(Source);
							PickUp->RefreshNearbyList(1);
							UE_LOG(LogTemp, Log, TEXT("아이템 줍기 요청: %s"), *DroppedObj->Name.ToString());
						}
						return true;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("드롭 실패: Vicinity 아이템은 InventoryList에만 드롭 가능"));
						return false;
					}
				}

				// [수정] Inventory → Vicinity (버리기)
				// 조건: SourceInstance 있음 && VicinityList에 드롭
				if (UBSItemInstance* SourceInstance = DroppedObj->SourceInstance)  // EntryObject에 인벤토리 아이템이 들어있으면
				{
					if (DropTarget == 1) // VicinityList에 드롭
					{
						if (UWorld* World = GetWorld())
						{
							// [데디서버 대응] Find item index and call ServerDiscardItem RPC
							int32 ItemIndex = Inv->FindItemIndex(SourceInstance);
							if (ItemIndex != INDEX_NONE)
							{
								// Alt 안 눌렀을 때: 전체 수량 버리기
								int32 TotalQuantity = SourceInstance->Dynamic.Quantity;
								Inv->ServerDiscardItem(ItemIndex, TotalQuantity);
								UE_LOG(LogTemp, Log, TEXT("아이템 버리기 요청: %s (Index: %d, 수량: %d)"), *DroppedObj->Name.ToString(), ItemIndex, TotalQuantity);
							}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("아이템 버리기 실패: 인벤토리에서 아이템을 찾을 수 없음"));
							}
						}
						return true;    // 드랍 처리 성공
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("드롭 실패: Inventory 아이템은 VicinityList에만 드롭 가능"));
						return false;
					}
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
					// 서버에서 처리 후 자동으로 리플리케이트되어 OnInventoryChanged가 호출됨
					if (PickUp) PickUp->ServerTryPickupPartial(Src, Quantity);
					if (PickUp) PickUp->RefreshNearbyList(true);
				}
			}
			break;

			case ESplitMode::InventoryToVicinity:
			{
				// 인벤토리 → 주변 : 일부 버리기
				if (UBSItemInstance* Inst = PendingSplitObj->SourceInstance)
				{
					// [데디서버 대응] Find item index and call ServerDiscardItem RPC
					int32 ItemIndex = Inv->FindItemIndex(Inst);
					if (ItemIndex != INDEX_NONE)
					{
						Inv->ServerDiscardItem(ItemIndex, Quantity);
						UE_LOG(LogTemp, Log, TEXT("아이템 일부 버리기 요청: Index %d, 수량: %d"), ItemIndex, Quantity);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("아이템 버리기 실패: 인벤토리에서 아이템을 찾을 수 없음"));
					}
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

// ========================================
// 3D 캐릭터 프리뷰 구현
// ========================================

void UInventoryWidget::SetupPreviewScene()
{
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] SetupPreviewScene START"));
	UE_LOG(LogTemp, Warning, TEXT("========================================"));

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] GetWorld returned NULL"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] World found: %s"), *World->GetName());

	// 1. Render Target 생성 (512x512 해상도)
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Step 1: Creating RenderTarget"));
	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] RenderTarget creation FAILED"));
		return;
	}

	RenderTarget->InitAutoFormat(512, 512);
	RenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	RenderTarget->bAutoGenerateMips = false;
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] RenderTarget created: %s"), *RenderTarget->GetName());
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] RenderTarget Size: %d x %d"), RenderTarget->SizeX, RenderTarget->SizeY);

	// 2. Image 위젯에 Render Target 연결
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Step 2: Connecting to Image widget"));
	if (!Img_CharacterPreview)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] Img_CharacterPreview is NULL"));
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] Check WBP_Inventory: Image widget named Img_CharacterPreview with Is Variable checked"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Image widget found: %s"), *Img_CharacterPreview->GetName());

	FSlateBrush Brush;
	Brush.SetResourceObject(RenderTarget);
	Brush.ImageSize = FVector2D(512.0f, 512.0f);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.Tiling = ESlateBrushTileType::NoTile;
	Brush.TintColor = FLinearColor::White;
	Img_CharacterPreview->SetBrush(Brush);
	Img_CharacterPreview->SetColorAndOpacity(FLinearColor::White);
	Img_CharacterPreview->SetOpacity(1.0f);
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Image widget brush configured"));

	// 3. 플레이어 캐릭터의 InventoryCamera 가져오기
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Step 3: Finding Player Character"));
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] Player Character NOT FOUND"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Player Character found: %s"), *PlayerCharacter->GetName());

	// BSCharacterPlayer의 InventoryCamera 찾기
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Searching for InventoryCamera component"));
	USceneCaptureComponent2D* InventoryCamera = PlayerCharacter->FindComponentByClass<USceneCaptureComponent2D>();
	if (!InventoryCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] InventoryCamera NOT FOUND on Player Character"));
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] Check BSCharacterPlayer has InventoryCamera component"));

		// 모든 컴포넌트 나열
		TArray<UActorComponent*> Components;
		PlayerCharacter->GetComponents(Components);
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Available components on Player Character:"));
		for (UActorComponent* Comp : Components)
		{
			if (Comp)
			{
				UE_LOG(LogTemp, Warning, TEXT("  - %s (Class: %s)"), *Comp->GetName(), *Comp->GetClass()->GetName());
			}
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] InventoryCamera FOUND: %s"), *InventoryCamera->GetName());
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] InventoryCamera Class: %s"), *InventoryCamera->GetClass()->GetName());

	// 4. InventoryCamera에 RenderTarget 연결 및 활성화
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Step 4: Configuring InventoryCamera"));

	// IMPORTANT: ShowOnlyComponents 설정
	USkeletalMeshComponent* PlayerMesh = PlayerCharacter->GetMesh();
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] PlayerMesh pointer: %p"), PlayerMesh);

	if (!PlayerMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] CRITICAL: PlayerMesh is NULL"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] PlayerMesh name: %s"), *PlayerMesh->GetName());
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] PlayerMesh class: %s"), *PlayerMesh->GetClass()->GetName());
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] ShowOnlyComponents Count BEFORE clear: %d"), InventoryCamera->ShowOnlyComponents.Num());

	// ShowOnlyComponents로 캐릭터만 렌더링
	InventoryCamera->ShowOnlyComponents.Empty();
	InventoryCamera->ShowOnlyComponents.Add(PlayerMesh);
	InventoryCamera->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] ShowOnlyComponents: Rendering ONLY CharacterMesh"));
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Component count: %d"), InventoryCamera->ShowOnlyComponents.Num());

	// 현재 상태 로그
	int32 ComponentCount = InventoryCamera->ShowOnlyComponents.Num();
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] ShowOnlyComponents Count AFTER FORCE ADD: %d"), ComponentCount);

	// ShowOnlyComponents 내용 확인
	if (ComponentCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Listing %d ShowOnlyComponents:"), ComponentCount);
		for (int32 i = 0; i < ComponentCount; i++)
		{
			UPrimitiveComponent* Comp = InventoryCamera->ShowOnlyComponents[i].Get();
			if (Comp)
			{
				UE_LOG(LogTemp, Warning, TEXT("  Index %d: %s (Class: %s, Valid: YES)"),
					i, *Comp->GetName(), *Comp->GetClass()->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("  Index %d: NULL COMPONENT (TWeakObjectPtr exists but Get() returns nullptr)"), i);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] ERROR: ShowOnlyComponents is EMPTY"));
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] BLACK SCREEN expected - no mesh to render"));
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] FOV=%.1f Source=%d RenderMode=%d"),
		InventoryCamera->FOVAngle, (int32)InventoryCamera->CaptureSource, (int32)InventoryCamera->PrimitiveRenderMode);

	// ========================================
	// Camera Positioning: Initial setup
	// ========================================

	// Initialize camera at 0 degrees (front of character)
	CurrentYaw = 0.0f;
	UpdateCameraPosition(CurrentYaw);

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Initial camera position set (Yaw=0)"));

	// RenderTarget 연결
	InventoryCamera->TextureTarget = RenderTarget;
	// [수정] bCaptureEveryFrame을 false로 설정 (성능 최적화)
	// NativeTick에서 수동으로 CaptureScene() 호출
	InventoryCamera->bCaptureEveryFrame = false;
	InventoryCamera->bCaptureOnMovement = false;

	// 즉시 캡처
	InventoryCamera->CaptureScene();
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] CaptureScene called"));

	// SceneCapture 참조 저장
	SceneCapture = InventoryCamera;

	UE_LOG(LogTemp, Warning, TEXT("========================================"));
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] SetupPreviewScene COMPLETE"));
	UE_LOG(LogTemp, Warning, TEXT("========================================"));
}

void UInventoryWidget::CleanupPreviewScene()
{
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] CleanupPreviewScene START"));

	// SceneCapture (=InventoryCamera) 비활성화
	if (SceneCapture)
	{
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Disabling InventoryCamera: %s"), *SceneCapture->GetName());

		// 캡처 비활성화 (성능 최적화)
		SceneCapture->bCaptureEveryFrame = false;
		SceneCapture->bCaptureOnMovement = false;
		SceneCapture->TextureTarget = nullptr;

		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] InventoryCamera disabled"));
		SceneCapture = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] SceneCapture was NULL"));
	}

	// CameraArm 정리
	if (CameraArm)
	{
		CameraArm = nullptr;
	}

	// PreviewMesh 정리
	if (PreviewMesh)
	{
		PreviewMesh = nullptr;
	}

	// RenderTarget은 GC가 자동 처리
	if (RenderTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] Releasing RenderTarget: %s"), *RenderTarget->GetName());
		RenderTarget = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY WIDGET] CleanupPreviewScene COMPLETE"));
}

// ========================================
// 카메라 위치 업데이트 (공전)
// ========================================

void UInventoryWidget::UpdateCameraPosition(float Yaw)
{
	if (!SceneCapture)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY WIDGET] UpdateCameraPosition: SceneCapture is NULL"));
		return;
	}

	// Get player character
	UWorld* World = GetWorld();
	if (!World) return;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!PlayerCharacter) return;

	// Character position
	FVector CharacterLocation = PlayerCharacter->GetActorLocation();

	// Camera settings
	float CameraDistance = 400.0f;
	float CameraHeight = 90.0f;

	// Calculate camera position using Yaw angle (orbit around character)
	FRotator OrbitRotation(0.0f, Yaw, 0.0f);
	FVector OrbitDirection = OrbitRotation.RotateVector(FVector::ForwardVector);
	FVector CameraLocation = CharacterLocation + (OrbitDirection * CameraDistance) + FVector(0, 0, CameraHeight);

	// Calculate rotation to look at character
	FVector LookAtTarget = CharacterLocation + FVector(0, 0, CameraHeight);
	FVector DirectionToCharacter = LookAtTarget - CameraLocation;
	FRotator LookAtRotation = DirectionToCharacter.Rotation();

	// Update camera transform
	SceneCapture->SetWorldLocation(CameraLocation);
	SceneCapture->SetWorldRotation(LookAtRotation);

	// Capture scene
	SceneCapture->CaptureScene();
}

// ========================================
// 마우스 이벤트 처리 (캐릭터 회전)
// ========================================

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (Img_CharacterPreview && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		bIsDraggingCharacter = true;
		DragStartPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		UE_LOG(LogTemp, Log, TEXT("[INVENTORY WIDGET] Started dragging - Yaw: %.1f"), CurrentYaw);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingCharacter)
	{
		bIsDraggingCharacter = false;
		UE_LOG(LogTemp, Log, TEXT("InventoryWidget: Stopped dragging character"));

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingCharacter)
	{
		FVector2D CurrentMousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		FVector2D MouseDelta = CurrentMousePos - DragStartPosition;

		// Mouse X movement -> Yaw rotation (orbit around character)
		float RotationSpeed = 0.3f;
		CurrentYaw += MouseDelta.X * RotationSpeed;

		// Update camera position based on new Yaw
		UpdateCameraPosition(CurrentYaw);

		// Update drag start position for next frame
		DragStartPosition = CurrentMousePos;

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

// ========================================
// 드롭 위치 확인 (어느 ListView에 드롭했는지)
// ========================================

int32 UInventoryWidget::GetDropTargetWidget(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent)
{
	// 드롭된 위치가 VicinityList 위인지, InventoryList 위인지 확인
	//
	// 동작 원리:
	// 1. 마우스의 절대 좌표를 가져옴
	// 2. 각 ListView의 절대 좌표 영역과 비교
	// 3. 겹치는 영역이 있으면 해당 ListView 번호 반환

	if (!VicinityList || !InventoryList)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetDropTargetWidget: VicinityList or InventoryList is NULL"));
		return 0; // 위젯이 없으면 알 수 없음
	}

	// 드롭된 마우스 위치 (스크린 좌표)
	FVector2D ScreenSpacePos = InDragDropEvent.GetScreenSpacePosition();

	// VicinityList의 화면상 영역 가져오기
	FGeometry VicinityGeometry = VicinityList->GetCachedGeometry();
	FVector2D VicinityLocalSize = VicinityGeometry.GetLocalSize();
	FVector2D VicinityAbsolutePos = VicinityGeometry.GetAbsolutePosition();

	// InventoryList의 화면상 영역 가져오기
	FGeometry InventoryGeometry = InventoryList->GetCachedGeometry();
	FVector2D InventoryLocalSize = InventoryGeometry.GetLocalSize();
	FVector2D InventoryAbsolutePos = InventoryGeometry.GetAbsolutePosition();

	UE_LOG(LogTemp, Log, TEXT("ScreenPos: (%.1f, %.1f)"), ScreenSpacePos.X, ScreenSpacePos.Y);
	UE_LOG(LogTemp, Log, TEXT("VicinityList: Pos(%.1f, %.1f) Size(%.1f, %.1f)"),
		VicinityAbsolutePos.X, VicinityAbsolutePos.Y, VicinityLocalSize.X, VicinityLocalSize.Y);
	UE_LOG(LogTemp, Log, TEXT("InventoryList: Pos(%.1f, %.1f) Size(%.1f, %.1f)"),
		InventoryAbsolutePos.X, InventoryAbsolutePos.Y, InventoryLocalSize.X, InventoryLocalSize.Y);

	// VicinityList 영역 체크 (AABB 충돌 검사)
	if (ScreenSpacePos.X >= VicinityAbsolutePos.X &&
		ScreenSpacePos.X <= VicinityAbsolutePos.X + VicinityLocalSize.X &&
		ScreenSpacePos.Y >= VicinityAbsolutePos.Y &&
		ScreenSpacePos.Y <= VicinityAbsolutePos.Y + VicinityLocalSize.Y)
	{
		UE_LOG(LogTemp, Log, TEXT("드롭 위치: VicinityList"));
		return 1; // VicinityList
	}

	// InventoryList 영역 체크
	if (ScreenSpacePos.X >= InventoryAbsolutePos.X &&
		ScreenSpacePos.X <= InventoryAbsolutePos.X + InventoryLocalSize.X &&
		ScreenSpacePos.Y >= InventoryAbsolutePos.Y &&
		ScreenSpacePos.Y <= InventoryAbsolutePos.Y + InventoryLocalSize.Y)
	{
		UE_LOG(LogTemp, Log, TEXT("드롭 위치: InventoryList"));
		return 2; // InventoryList
	}

	UE_LOG(LogTemp, Warning, TEXT("드롭 위치: 알 수 없음 (두 리스트 밖)"));
	return 0; // 둘 다 아님
}