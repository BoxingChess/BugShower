// Fill out your copyright notice in the Description page of Project Settings.

#include "BSUIManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "Item/ItemActor.h"
void UBSUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// DataAsset에서 Widget 설정 로드
	if (!UIConfigAsset)
	{
		// Try to load DataAsset from default path
		UIConfigAsset = LoadObject<UBSUIConfig>(nullptr, TEXT("/Game/DataAsset/DA_BugShowerUI.DA_BugShowerUI"));

		if (!UIConfigAsset)
		{
			UE_LOG(LogTemp, Error, TEXT("BSUIManager - Failed to load DA_BugShowerUI! Make sure it exists at /Game/DataAsset/DA_BugShowerUI"));
			return;
		}
	}

	if (UIConfigAsset)
	{
		WidgetConfigs = UIConfigAsset->WidgetConfigs;
		UE_LOG(LogTemp, Log, TEXT("BSUIManager - Loaded %d widget configs from DataAsset"), WidgetConfigs.Num());
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

		// Viewport에 추가
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
	if (Widget->GetVisibility() == ESlateVisibility::Visible)
	{
		return;
	}

	// Widget 표시
	Widget->SetVisibility(ESlateVisibility::Visible);
	UIData.VisibleWidgets.AddUnique(WidgetName);

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
	if (Widget->GetVisibility() != ESlateVisibility::Visible)
	{
		return;
	}

	// Widget 숨김
	Widget->SetVisibility(ESlateVisibility::Collapsed);
	UIData.VisibleWidgets.Remove(WidgetName);

	// Input 모드 업데이트
	UpdateInputMode();

	UE_LOG(LogTemp, Log, TEXT("BSUIManager::HideWidget - Hiding Widget '%s'"), *WidgetName.ToString());
}

void UBSUIManager::ToggleWidget(FName WidgetName, APlayerController* PC)
{
	if (IsWidgetVisible(WidgetName, PC))
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
	const UUserWidget* Widget = GetWidget(WidgetName, PC);
	if (!Widget)
	{
		return false;
	}

	return Widget->GetVisibility() == ESlateVisibility::Visible;
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
	// TODO: HealthBarWidget 구현 후 연동
	UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdateHealthUI - Not implemented yet"));
}

void UBSUIManager::UpdatePickupPrompt(AItemActor* Item)
{
	if (!LocalPlayerController)
	{
		return;
	}

	UUserWidget* Widget = GetWidget(FName("LineTraceUI"));
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdatePickupPrompt - LineTraceUI widget not found"));
		return;
	}

	if (Item && Item->GetItemStaticData())
	{
		// Show widget and update with item data
		const UBSStaticItemDataAsset* ItemData = Item->GetItemStaticData();

		// Call Blueprint-exposed function to update UI
		// The widget should implement a function called "UpdateItemData" or use variables

		// Method 1: Using Blueprint-callable function (recommended)
		UFunction* UpdateFunc = Widget->FindFunction(FName("UpdateItemData"));
		if (UpdateFunc)
		{
			struct FUpdateItemDataParams
			{
				FText ItemName;
				UTexture2D* ItemIcon;
				int32 Quantity;
			};

			FUpdateItemDataParams Params;
			Params.ItemName = ItemData->DisplayName;
			Params.ItemIcon = ItemData->Icon;
			Params.Quantity = 1; // TODO: Get actual quantity from item instance

			Widget->ProcessEvent(UpdateFunc, &Params);
			UE_LOG(LogTemp, Log, TEXT("BSUIManager::UpdatePickupPrompt - Updated LineTraceUI with item: %s"), *ItemData->DisplayName.ToString());
		}
		else
		{
			// Method 2: Set variables directly (fallback)
			UE_LOG(LogTemp, Warning, TEXT("BSUIManager::UpdatePickupPrompt - UpdateItemData function not found in widget"));
		}

		// Show the widget
		ShowWidget(FName("LineTraceUI"));
	}
	else
	{
		// Hide the widget when no item
		HideWidget(FName("LineTraceUI"));
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

	switch (InputMode)
	{
	case EUIInputMode::GameOnly:
		LocalPlayerController->SetInputMode(FInputModeGameOnly());
		LocalPlayerController->SetShowMouseCursor(false);
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
			FInputModeUIOnly Mode;
			if (WidgetToFocus)
			{
				Mode.SetWidgetToFocus(WidgetToFocus->TakeWidget());
			}
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			LocalPlayerController->SetInputMode(Mode);
			LocalPlayerController->SetShowMouseCursor(true);
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
	if (UIData.VisibleWidgets.Num() == 0)
	{
		SetInputMode(EUIInputMode::GameOnly);
		return;
	}

	// 표시 중인 Widget 중 가장 높은 우선순위의 Input 모드 찾기
	EUIInputMode TargetMode = EUIInputMode::GameOnly;
	UUserWidget* WidgetToFocus = nullptr;

	for (const FName& VisibleWidgetName : UIData.VisibleWidgets)
	{
		const FBSWidgetConfig* Config = WidgetConfigs.Find(VisibleWidgetName);
		if (!Config)
		{
			continue;
		}

		// UIOnly가 가장 높은 우선순위
		if (Config->InputMode == EUIInputMode::UIOnly)
		{
			TargetMode = EUIInputMode::UIOnly;
			WidgetToFocus = GetWidget(VisibleWidgetName);
			break;
		}

		// GameAndUI가 그 다음
		if (Config->InputMode == EUIInputMode::GameAndUI && TargetMode != EUIInputMode::UIOnly)
		{
			TargetMode = EUIInputMode::GameAndUI;
			WidgetToFocus = GetWidget(VisibleWidgetName);
		}
	}

	SetInputMode(TargetMode, WidgetToFocus);
}
