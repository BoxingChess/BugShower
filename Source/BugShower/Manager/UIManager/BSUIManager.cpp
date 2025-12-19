// Fill out your copyright notice in the Description page of Project Settings.

#include "BSUIManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "Item/ItemActor.h"
#include "Player/BSPlayerController.h"
#include "Widget/LineTraceUI.h"
#include "Widget/HealthBarWidget.h"
#include "Widget/AmmoWidget.h"
///꼭 읽어봐!!
/**
 * UI Manager 초기화 (게임 시작 시 1회 호출)
 *
 * 호출 시점:
 * - 게임 시작 시 자동 호출 (GameInstance 생성 시)
 * - Subsystem이므로 수동 호출 불필요
 * - 게임 종료까지 1번만 호출됨
 *
 * 주의사항:
 * - 로비/인게임 전환 시 이 함수를 호출하지 마세요!
 * - 대신 SwitchUIConfig() 사용:
 *   예) UIManager->SwitchUIConfig(BSUIConfigNames::Lobby);
 *
 * 동작 흐름:
 *   게임 시작
    ↓
  Initialize() 자동 호출 (1회)
    - CSV에서 Config 로드
    - "InGame" Config 자동 선택
    ↓
  로비 맵 로드
    ↓
  SwitchUIConfig("Lobby") 호출
    - 기존 UI 정리
    - Lobby Config 로드
    - Lobby UI 생성
    ↓
  매치 시작
    ↓
  SwitchUIConfig("InGame") 호출
    - Lobby UI 정리
    - InGame UI 생성
 */
void UBSUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ========================================
	// 1단계: UIConfigs TMap 채우기
	// ========================================
	// DataTable(CSV)에서 UI Config 목록을 자동으로 로드
	// CSV 위치: Content/DataTables/DT_UIConfigs.csv
	// ========================================

	// UIConfigTable이 없으면 기본 경로에서 자동 로드
	if (!UIConfigTable && UIConfigs.Num() == 0)
	{
		UIConfigTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DataTables/DT_UIConfigs.DT_UIConfigs"));
		if (UIConfigTable)
		{
			UE_LOG(LogTemp, Log, TEXT("BSUIManager - Auto-loaded UIConfigTable from /Game/DataTables/DT_UIConfigs"));
		}
	}

	// 우선순위 1: UIConfigTable이 있으면 DataTable에서 로드
	if (UIConfigTable && UIConfigs.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("BSUIManager - Loading UI Configs from DataTable..."));

		// DataTable에서 모든 Row 읽기
		TArray<FBSUIConfigTableRow*> Rows;
		UIConfigTable->GetAllRows<FBSUIConfigTableRow>(TEXT("LoadUIConfigs"), Rows);

		for (FBSUIConfigTableRow* Row : Rows)
		{
			if (!Row || Row->ConfigName.IsNone())
			{
				UE_LOG(LogTemp, Warning, TEXT("BSUIManager - Skipping invalid row in UIConfigTable"));
				continue;
			}

			// AssetPath로 Config 동기 로드
			UBSUIConfig* Config = Cast<UBSUIConfig>(Row->ConfigAssetPath.TryLoad());
			if (Config)
			{
				UIConfigs.Add(Row->ConfigName, Config);
				UE_LOG(LogTemp, Log, TEXT("BSUIManager - Loaded Config '%s' from DataTable: %s"),
					*Row->ConfigName.ToString(), *Row->ConfigAssetPath.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BSUIManager - Failed to load Config '%s' from path: %s"),
					*Row->ConfigName.ToString(), *Row->ConfigAssetPath.ToString());
			}
		}

		if (UIConfigs.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("BSUIManager - Successfully loaded %d configs from DataTable"), UIConfigs.Num());
		}
		else
		{
			// DataTable은 있지만 Config가 없음 - CSV 파일 확인 필요
			UE_LOG(LogTemp, Error, TEXT("BSUIManager - DataTable is set but no configs were loaded!"));
			UE_LOG(LogTemp, Error, TEXT("Check CSV file: Content/DataTables/DT_UIConfigs.csv"));
			return;
		}
	}
	// UIConfigs가 에디터에서 직접 설정된 경우 (옵션)
	else if (UIConfigs.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("BSUIManager - Using UIConfigs set directly in editor (%d configs)"), UIConfigs.Num());
	}
	// UIConfigTable도 없고 UIConfigs도 비어있음 - 초기화 실패
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSUIManager - No UIConfigTable found!"));
		UE_LOG(LogTemp, Error, TEXT("Make sure DT_UIConfigs exists at: Content/DataTables/DT_UIConfigs.csv"));
		UE_LOG(LogTemp, Error, TEXT("And DataTable asset is created with Row Structure: FBSUIConfigTableRow"));
		return;
	}

	// ========================================
	// 2단계: UIConfigs TMap을 사용해서 초기화 (새 방식 - 무조건 실행!)
	// ========================================
	// 목적: 여러 Config 중에서 초기 Config를 선택하여 로드
	// 동작: 우선순위에 따라 Config를 선택하고 WidgetConfigs를 설정
	// 우선순위: "InGame" (1순위) > "Default" (2순위) > 첫 번째 항목 (3순위)
	// ========================================
	UE_LOG(LogTemp, Log, TEXT("BSUIManager - Found %d UI Configs in TMap"), UIConfigs.Num());

	// 초기 Config와 이름을 저장할 변수
	UBSUIConfig* InitialConfig = nullptr;
	FName InitialConfigName;

	// 1순위: "InGame" Config (게임 플레이가 기본)
	if (UIConfigs.Contains(TEXT("InGame")))
	{
		InitialConfigName = TEXT("InGame");
		InitialConfig = UIConfigs[InitialConfigName];
	}
	// 2순위: "Default" Config (명시적 기본값)
	else if (UIConfigs.Contains(TEXT("Default")))
	{
		InitialConfigName = TEXT("Default");
		InitialConfig = UIConfigs[InitialConfigName];
	}
	// 3순위: TMap의 첫 번째 Config (정의된 순서)
	else
	{
		for (const auto& Pair : UIConfigs)
		{
			InitialConfigName = Pair.Key;
			InitialConfig = Pair.Value;
			break;  // 첫 번째 항목만 가져오고 종료
		}
	}

	// ========================================
	// 3단계: 선택한 Config를 현재 Config로 설정
	// ========================================
	// 목적: 선택한 Config의 WidgetConfigs를 로드하여 UI 초기화 준비
	// 동작: CurrentConfigName과 WidgetConfigs를 설정
	// ========================================
	if (InitialConfig)
	{
		CurrentConfigName = InitialConfigName;
		WidgetConfigs = InitialConfig->WidgetConfigs;
		UE_LOG(LogTemp, Log, TEXT("BSUIManager - Loaded initial UI Config: '%s' with %d widgets"),
			*InitialConfigName.ToString(), WidgetConfigs.Num());
	}
	else
	{
		// 유효한 Config가 없으면 초기화 실패
		UE_LOG(LogTemp, Error, TEXT("BSUIManager - No valid UI Config found!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BSUIManager Initialized"));
}

void UBSUIManager::Deinitialize()
{
	// UI 정리
	CleanupPlayerUI(LocalPlayerController);

	Super::Deinitialize();
}

void UBSUIManager::InitializePlayerUI(APlayerController* PC)
{
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::InitializePlayerUI - Invalid PlayerController"));
		return;
	}

	// 데디케이티드 서버에서는 UI 생성하지 않음
	if (GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Log, TEXT("BSUIManager::InitializePlayerUI - Skipping UI creation on dedicated server"));
		return;
	}

	// 로컬 플레이어만 UI 생성 (리모트 플레이어 제외)
	if (!PC->IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("BSUIManager::InitializePlayerUI - Skipping UI creation for remote player"));
		return;
	}

	// 이미 초기화되어 있으면 스킵
	if (LocalPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::InitializePlayerUI - Already initialized"));
		return;
	}

	// 로컬 플레이어 컨트롤러 저장
	LocalPlayerController = PC;

	// 모든 Widget 생성
	for (const auto& Pair : WidgetConfigs)
	{
		const FName& WidgetName = Pair.Key;
		const FBSWidgetConfig& Config = Pair.Value;

		if (!Config.WidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("BSUIManager::InitializePlayerUI - Widget '%s' has no WidgetClass"), *WidgetName.ToString());
			continue;
		}

		// Widget 생성
		UUserWidget* Widget = BSCreateWidget(WidgetName);
		if (!Widget)
		{
			UE_LOG(LogTemp, Error, TEXT("BSUIManager::InitializePlayerUI - Failed to create Widget '%s'"), *WidgetName.ToString());
			continue;
		}

		// Viewport에 추가 (플래그가 true일 때만)
		if (Config.bAddToViewport)
		{
			Widget->AddToViewport(Config.ZOrder);

			// 동작 방식에 따라 초기 표시 설정
			if (Config.Behavior == EWidgetBehavior::AlwaysVisible)
			{
				Widget->SetVisibility(ESlateVisibility::Visible);
				UIData.VisibleWidgets.Add(WidgetName);
			}
			else
			{
				Widget->SetVisibility(ESlateVisibility::Collapsed);
			}

			UE_LOG(LogTemp, Log, TEXT("BSUIManager::InitializePlayerUI - Created Widget '%s' (Added to Viewport)"), *WidgetName.ToString());
		}
		else
		{
			// AddToViewport 하지 않음 (툴팁 등 특수 용도)
			UE_LOG(LogTemp, Log, TEXT("BSUIManager::InitializePlayerUI - Created Widget '%s' (Not added to Viewport)"), *WidgetName.ToString());
		}

		UE_LOG(LogTemp, Log, TEXT("BSUIManager::InitializePlayerUI - Created Widget '%s'"), *WidgetName.ToString());
	}

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::InitializePlayerUI - UI initialized for local player"));
}

void UBSUIManager::CleanupPlayerUI(APlayerController* PC)
{
	if (!LocalPlayerController)
	{
		return;
	}

	// 모든 Widget 제거
	for (auto& Pair : UIData.Widgets)
	{
		if (UUserWidget* Widget = Pair.Value)
		{
			Widget->RemoveFromParent();
		}
	}

	UIData.Widgets.Empty();
	UIData.VisibleWidgets.Empty();
	LocalPlayerController = nullptr;

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::CleanupPlayerUI - Cleaned up UI"));
}

void UBSUIManager::ShowWidget(FName WidgetName, APlayerController* PC)
{
	if (!LocalPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::ShowWidget - UI not initialized"));
		return;
	}

	UUserWidget* Widget = GetWidget(WidgetName, PC);
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::ShowWidget - Widget '%s' not found"), *WidgetName.ToString());
		return;
	}

	// 이미 표시 중이면 스킵
	ESlateVisibility CurrentVis = Widget->GetVisibility();
	UE_LOG(LogTemp, Warning, TEXT("ShowWidget '%s' - Current visibility before change: %d"),
		*WidgetName.ToString(), (int32)CurrentVis);

	if (CurrentVis == ESlateVisibility::Visible)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowWidget '%s' - Already visible, skipping"), *WidgetName.ToString());
		return;
	}

	// Widget 표시
	Widget->SetVisibility(ESlateVisibility::Visible);
	UIData.VisibleWidgets.AddUnique(WidgetName);

	// 설정 후 확인
	ESlateVisibility NewVis = Widget->GetVisibility();
	UE_LOG(LogTemp, Warning, TEXT("ShowWidget '%s' - Visibility after SetVisibility: %d"),
		*WidgetName.ToString(), (int32)NewVis);

	// Input 모드 업데이트
	UpdateInputMode();

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::ShowWidget - Showing Widget '%s'"), *WidgetName.ToString());
}

void UBSUIManager::HideWidget(FName WidgetName, APlayerController* PC)
{
	if (!LocalPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::HideWidget - UI not initialized"));
		return;
	}

	UUserWidget* Widget = GetWidget(WidgetName, PC);
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::HideWidget - Widget '%s' not found"), *WidgetName.ToString());
		return;
	}

	// 이미 숨겨져 있으면 스킵
	ESlateVisibility CurrentVis = Widget->GetVisibility();
	UE_LOG(LogTemp, Warning, TEXT("HideWidget '%s' - Current visibility before change: %d"),
		*WidgetName.ToString(), (int32)CurrentVis);

	if (CurrentVis != ESlateVisibility::Visible)
	{
		UE_LOG(LogTemp, Warning, TEXT("HideWidget '%s' - Already hidden, skipping"), *WidgetName.ToString());
		return;
	}

	// Widget 숨김
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	UIData.VisibleWidgets.Remove(WidgetName);

	// 설정 후 확인
	ESlateVisibility NewVis = Widget->GetVisibility();
	UE_LOG(LogTemp, Warning, TEXT("HideWidget '%s' - Visibility after SetVisibility: %d"),
		*WidgetName.ToString(), (int32)NewVis);

	// Input 모드 업데이트
	UpdateInputMode();

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::HideWidget - Hiding Widget '%s'"), *WidgetName.ToString());
}

void UBSUIManager::ToggleWidget(FName WidgetName, APlayerController* PC)
{
	bool bIsVisible = IsWidgetVisible(WidgetName, PC);
	UE_LOG(LogTemp, Warning, TEXT("ToggleWidget '%s' - Current visibility: %s"),
		*WidgetName.ToString(), bIsVisible ? TEXT("VISIBLE") : TEXT("HIDDEN"));

	if (bIsVisible)
	{
		HideWidget(WidgetName, PC);
	}
	else
	{
		ShowWidget(WidgetName, PC);
	}
}

bool UBSUIManager::IsWidgetVisible(FName WidgetName, APlayerController* PC) const
{
	// Use the tracked VisibleWidgets array instead of GetVisibility()
	// GetVisibility() may not reflect the actual state correctly in some cases
	bool bIsInArray = UIData.VisibleWidgets.Contains(WidgetName);

	UE_LOG(LogTemp, Warning, TEXT("IsWidgetVisible '%s' - In VisibleWidgets array: %s"),
		*WidgetName.ToString(), bIsInArray ? TEXT("YES") : TEXT("NO"));

	return bIsInArray;
}

UUserWidget* UBSUIManager::GetWidget(FName WidgetName, APlayerController* PC) const
{
	if (!LocalPlayerController)
	{
		return nullptr;
	}

	TObjectPtr<UUserWidget> const* WidgetPtr = UIData.Widgets.Find(WidgetName);
	return WidgetPtr ? *WidgetPtr : nullptr;
}

void UBSUIManager::UpdateInventoryUI(UInventoryComponent* InventoryComp)
{
	// TODO: InventoryWidget 구현 후 연동
	UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdateInventoryUI - Not implemented yet"));
}

void UBSUIManager::UpdateHealthUI(float Health, float MaxHealth)
{
	if (!LocalPlayerController)
	{
		return;
	}

	// HealthBar Widget 가져오기
	UUserWidget* Widget = GetWidget(BSUINames::HealthBar);
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdateHealthUI - HealthBar widget not found"));
		return;
	}

	// HealthBarWidget으로 캐스팅
	if (UHealthBarWidget* HealthWidget = Cast<UHealthBarWidget>(Widget))
	{
		// Blueprint에서 구현한 UpdateHealth 이벤트 호출
		HealthWidget->UpdateHealth(Health, MaxHealth);
		UE_LOG(LogTemp, Log, TEXT("BSUIManager::UpdateHealthUI - Updated HP: %.1f / %.1f"), Health, MaxHealth);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdateHealthUI - Widget is not UHealthBarWidget type!"));
	}
}

/**
 * 아이템 픽업 프롬프트 UI 업데이트
 *
 * 동작 방식:
 * 1. LineTraceUI 위젯을 가져옴
 * 2. ItemActor에서 StaticItemData (이름, 아이콘 등) 추출
 * 3. Widget Blueprint의 "UpdateItemData" 커스텀 이벤트를 찾아서 호출
 * 4. 파라미터로 ItemName, ItemIcon, Quantity를 전달
 * 5. Widget을 화면에 표시 (아이템이 있을 때) 또는 숨김 (아이템이 없을 때)
 *
 * Blueprint 연동 방법:
 * 1. WBP_LineTraceUI Widget Blueprint를 엽니다
 * 2. Event Graph에서 "Custom Event" 생성 (이름: UpdateItemData)
 * 3. Input 파라미터 추가:
 *    - ItemName (Text)
 *    - ItemIcon (Texture 2D Object Reference)
 *    - Quantity (Integer)
 * 4. 이 파라미터들을 UI 요소에 연결:
 *    - Text Block에 ItemName 바인딩
 *    - Image에 ItemIcon 바인딩
 *    - 수량 표시 Text에 Quantity 바인딩
 *
 * 주의: UpdateItemData 함수가 없으면 Warning 로그가 출력되고 UI가 업데이트되지 않습니다
 */
void UBSUIManager::UpdatePickupPrompt(AItemActor* Item)
{
	if (!LocalPlayerController)
	{
		return;
	}

	// LineTraceUI 위젯 가져오기
	UUserWidget* Widget = GetWidget(FName("LineTraceUI"));
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdatePickupPrompt - LineTraceUI widget not found"));
		return;
	}

	// 아이템이 있고 StaticData가 유효한 경우
	if (Item && Item->GetItemStaticData())
	{
		// ItemActor에서 정적 데이터 (이름, 아이콘 등) 가져오기
		const UBSStaticItemDataAsset* ItemData = Item->GetItemStaticData();

		// ULineTraceUI의 SetData 함수 호출
		if (ULineTraceUI* LineTraceWidget = Cast<ULineTraceUI>(Widget))
		{
			// 아이템 이름과 아이콘 전달
			LineTraceWidget->SetData(ItemData->Icon, ItemData->DisplayName);
			UE_LOG(LogTemp, Log, TEXT("BSUIManager::UpdatePickupPrompt - Updated LineTraceUI with item: %s"), *ItemData->DisplayName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdatePickupPrompt - Widget is not ULineTraceUI type!"));
		}

		// 위젯을 화면에 표시
		ShowWidget(FName("LineTraceUI"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no ItemStaticData"));
		// 아이템이 없으면 위젯 숨김 (아이템에서 시선을 떼었을 때)
		HideWidget(FName("LineTraceUI"));
	}
}

void UBSUIManager::UpdateAmmoUI(int32 CurrentAmmo, int32 ReserveAmmo)
{
	if (!LocalPlayerController)
	{
		return;
	}

	// AmmoDisplay Widget 가져오기
	UUserWidget* Widget = GetWidget(BSUINames::AmmoDisplay);
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdateAmmoUI - AmmoDisplay widget not found"));
		return;
	}

	// AmmoWidget으로 캐스팅
	if (UAmmoWidget* AmmoWidget = Cast<UAmmoWidget>(Widget))
	{
		// Blueprint에서 구현한 UpdateAmmo 이벤트 호출
		AmmoWidget->UpdateAmmo(CurrentAmmo, ReserveAmmo);
		UE_LOG(LogTemp, Log, TEXT("BSUIManager::UpdateAmmoUI - Updated Ammo: %d / %d"), CurrentAmmo, ReserveAmmo);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdateAmmoUI - Widget is not UAmmoWidget type!"));
	}
}

UUserWidget* UBSUIManager::BSCreateWidget(FName WidgetName)
{
	if (!LocalPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::CreateWidget - No local player controller"));
		return nullptr;
	}

	const FBSWidgetConfig* Config = WidgetConfigs.Find(WidgetName);
	if (!Config || !Config->WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::CreateWidget - Invalid config for Widget '%s'"), *WidgetName.ToString());
		return nullptr;
	}

	// Widget 생성
	UUserWidget* Widget = CreateWidget<UUserWidget>(LocalPlayerController, Config->WidgetClass);
	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("BSUIManager::CreateWidget - Failed to create Widget '%s'"), *WidgetName.ToString());
		return nullptr;
	}

	// UI 데이터에 저장
	UIData.Widgets.Add(WidgetName, Widget);

	return Widget;
}

void UBSUIManager::SetInputMode(EUIInputMode InputMode, UUserWidget* WidgetToFocus)
{
	if (!LocalPlayerController)
	{
		return;
	}

	// Cast to BSPlayerController to access EnableGameInput/DisableGameInput
	ABSPlayerController* BSController = Cast<ABSPlayerController>(LocalPlayerController);

	switch (InputMode)
	{
	case EUIInputMode::GameOnly:
		LocalPlayerController->SetInputMode(FInputModeGameOnly());
		LocalPlayerController->SetShowMouseCursor(false);
		UE_LOG(LogTemp, Warning, TEXT("SetInputMode: GameOnly - Hiding mouse cursor"));
		break;

	case EUIInputMode::GameAndUI:
		{
			FInputModeGameAndUI Mode;
			if (WidgetToFocus)
			{
				Mode.SetWidgetToFocus(WidgetToFocus->TakeWidget());
			}
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			LocalPlayerController->SetInputMode(Mode);
			LocalPlayerController->SetShowMouseCursor(true);
		}
		break;

	case EUIInputMode::UIOnly:
		{
			// GameAndUI 모드 사용 (UIOnly는 Tab 키까지 막아버림)
			FInputModeGameAndUI Mode;
			// SetWidgetToFocus를 호출하지 않음 -> 포커스가 UI에 고정되지 않아서 Tab 키가 계속 작동
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			Mode.SetHideCursorDuringCapture(false);
			LocalPlayerController->SetInputMode(Mode);
			LocalPlayerController->SetShowMouseCursor(true);
			// InputMappingContext는 유지 (Tab 키가 작동해야 하므로)
			UE_LOG(LogTemp, Warning, TEXT("SetInputMode: UIOnly -> Using GameAndUI (NO FOCUS) to keep Tab key working"));
		}
		break;
	}
}

void UBSUIManager::UpdateInputMode()
{
	if (!LocalPlayerController)
	{
		return;
	}

	// 표시 중인 Widget이 없으면 GameOnly
	UE_LOG(LogTemp, Error, TEXT(">>> UpdateInputMode - VisibleWidgets count: %d"), UIData.VisibleWidgets.Num());
	for (const FName& WidgetName : UIData.VisibleWidgets)
	{
		UE_LOG(LogTemp, Error, TEXT("    - Visible: %s"), *WidgetName.ToString());
	}

	if (UIData.VisibleWidgets.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> UpdateInputMode - NO visible widgets, setting GameOnly"));
		SetInputMode(EUIInputMode::GameOnly);
		return;
	}

	// 표시 중인 Widget 중 가장 높은 우선순위의 Input 모드 찾기
	EUIInputMode TargetMode = EUIInputMode::GameOnly;
	UUserWidget* WidgetToFocus = nullptr;

	for (const FName& VisibleWidgetName : UIData.VisibleWidgets)
	{
		// LineTraceUI는 입력 모드 변경하지 않음 (아이템 보고 있을 때 카메라 회전 유지)
		if (VisibleWidgetName == FName("LineTraceUI"))
		{
			continue;
		}

		const FBSWidgetConfig* Config = WidgetConfigs.Find(VisibleWidgetName);
		if (!Config)
		{
			UE_LOG(LogTemp, Warning, TEXT("UpdateInputMode - No config for widget '%s'"), *VisibleWidgetName.ToString());
			continue;
		}

		// UIOnly가 가장 높은 우선순위
		if (Config->InputMode == EUIInputMode::UIOnly)
		{
			TargetMode = EUIInputMode::UIOnly;
			WidgetToFocus = GetWidget(VisibleWidgetName);
			UE_LOG(LogTemp, Error, TEXT(">>> UpdateInputMode - Widget '%s' wants UIOnly"), *VisibleWidgetName.ToString());
			break;
		}

		// GameAndUI가 그 다음
		if (Config->InputMode == EUIInputMode::GameAndUI && TargetMode != EUIInputMode::UIOnly)
		{
			TargetMode = EUIInputMode::GameAndUI;
			WidgetToFocus = GetWidget(VisibleWidgetName);
			UE_LOG(LogTemp, Warning, TEXT("UpdateInputMode - Widget '%s' wants GameAndUI"), *VisibleWidgetName.ToString());
		}
	}

	SetInputMode(TargetMode, WidgetToFocus);
}

/**
 * UI Config 전환 (로비 ↔ 인게임 ↔ 결과 화면 등)
 *
 * 사용 시점:
 * - 로비 → 인게임: SwitchUIConfig(BSUIConfigNames::InGame)
 * - 인게임 → 로비: SwitchUIConfig(BSUIConfigNames::Lobby)
 * - 게임 종료 → 결과: SwitchUIConfig(BSUIConfigNames::Result)
 *
 * 동작 과정:
 * 1. 기존 UI 완전 정리 (CleanupPlayerUI)
 *    - 모든 Widget RemoveFromParent()
 *    - Widget 인스턴스 삭제
 * 2. 새 Config 적용
 *    - WidgetConfigs 교체
 *    - CurrentConfigName 업데이트
 * 3. 새 UI 생성 (InitializePlayerUI)
 *    - 새 Config의 Widget들 생성
 *    - Viewport에 추가
 *
 * 사용 예시 (C++):
 * @code
 * // GameMode에서 호출
 * UGameInstance* GI = GetGameInstance();
 * UBSUIManager* UIManager = GI->GetSubsystem<UBSUIManager>();
 * UIManager->SwitchUIConfig(BSUIConfigNames::Lobby);
 * @endcode
 *
 * 사용 예시 (Blueprint):
 * Get Game Instance → Get Subsystem (BSUIManager) → Switch UI Config ("Lobby")
 *
 * 주의사항:
 * - ConfigName이 UIConfigs에 없으면 실패
 * - CSV에 Config를 추가했으면 반드시 DataAsset도 생성해야 함
 *
 * @param ConfigName 전환할 Config 이름 (예: "InGame", "Lobby", "Result")
 */
void UBSUIManager::SwitchUIConfig(FName ConfigName)
{
	// Config 유효성 검사
	UBSUIConfig* NewConfig = UIConfigs.FindRef(ConfigName);
	if (!NewConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("BSUIManager::SwitchUIConfig - Config '%s' not found!"), *ConfigName.ToString());
		UE_LOG(LogTemp, Error, TEXT("Available configs:"));
		for (const auto& Pair : UIConfigs)
		{
			UE_LOG(LogTemp, Error, TEXT("  - %s"), *Pair.Key.ToString());
		}
		return;
	}

	// 이미 같은 Config를 사용 중이면 스킵
	if (CurrentConfigName == ConfigName)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::SwitchUIConfig - Already using Config '%s'"), *ConfigName.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::SwitchUIConfig - Switching from '%s' to '%s'"),
		*CurrentConfigName.ToString(), *ConfigName.ToString());

	// LocalPlayerController를 먼저 저장 (CleanupPlayerUI가 null로 만들기 전에!)
	APlayerController* SavedPC = LocalPlayerController;

	// 1단계: 기존 UI 정리
	CleanupPlayerUI(SavedPC);

	// 2단계: 새 Config 적용
	CurrentConfigName = ConfigName;
	WidgetConfigs = NewConfig->WidgetConfigs;

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::SwitchUIConfig - Loaded %d widget configs from '%s'"),
		WidgetConfigs.Num(), *ConfigName.ToString());

	// 3단계: 새 UI 생성
	InitializePlayerUI(SavedPC);

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::SwitchUIConfig - Successfully switched to Config '%s'"),
		*ConfigName.ToString());
}

UBSUIConfig* UBSUIManager::GetUIConfig(FName ConfigName) const
{
	return UIConfigs.FindRef(ConfigName);
}
