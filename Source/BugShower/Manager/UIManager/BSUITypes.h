// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BSUITypes.generated.h"

/**
 * Widget 동작 타입
 * Widget이 언제/어떻게 표시될지 결정
 */
UENUM(BlueprintType)
enum class EWidgetBehavior : uint8
{
	AlwaysVisible,		// 항상 표시 (예: HealthBar, Crosshair) - 게임 시작부터 끝까지 표시
	Toggle,				// 토글 방식 (예: Inventory, Map) - ShowWidget()/HideWidget()으로 수동 제어
	Context				// 조건부 표시 (예: PickupPrompt, LineTraceUI) - 특정 상황에서만 코드로 표시
};

/**
 * UI Input 모드
 * Widget이 표시될 때 플레이어의 입력을 어떻게 처리할지 결정
 */
UENUM(BlueprintType)
enum class EUIInputMode : uint8
{
	GameOnly,			// 게임만 (일반 플레이) - 마우스 숨김, 캐릭터 조작 가능
	GameAndUI,			// 게임 + UI (인벤토리 등) - 마우스 표시, 캐릭터 조작 + UI 클릭 가능
	UIOnly				// UI만 (메인 메뉴 등) - 마우스 표시, 캐릭터 조작 불가, UI만 조작
};

/**
 * Widget 설정 정보 (정적 설정 데이터)
 *
 * 용도: Blueprint/DataAsset에서 미리 정의하는 "Widget이 어떻게 동작할지"에 대한 설정
 * 시점: 개발 시 (설계 단계에서 정의)
 * 예시: "Inventory Widget은 ZOrder 10이고, 토글 방식이고, GameAndUI 모드다"
 *
 * FPlayerUIData와의 차이:
 * - FBSWidgetConfig: 설계도/레시피 (Blueprint에서 설정한 정적 데이터)
 * - FPlayerUIData: 실제 생성된 Widget 인스턴스들 (런타임 동적 데이터)
 */
USTRUCT(BlueprintType)
struct FBSWidgetConfig
{
	GENERATED_BODY()

	// Widget 클래스 - 어떤 Widget Blueprint를 생성할지 (예: WBP_Inventory)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> WidgetClass;

	// ViewPort Z-Order - Widget 겹칠 때 순서 (0=뒤, 높을수록 앞)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	int32 ZOrder = 0;

	// Widget 동작 방식 - 언제 표시될지 (AlwaysVisible/Toggle/Context)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	EWidgetBehavior Behavior = EWidgetBehavior::Toggle;

	// 마우스 커서 필요 여부 (현재 사용 안 함 - InputMode로 자동 처리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bRequiresMouseCursor = false;

	// Input 모드 - Widget 표시 시 입력 처리 방식 (GameOnly/GameAndUI/UIOnly)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	EUIInputMode InputMode = EUIInputMode::GameOnly;

	// Viewport 추가 여부 - false면 생성만 하고 AddToViewport() 호출 안 함 (툴팁용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bAddToViewport = true;

	FBSWidgetConfig()
		: WidgetClass(nullptr)
		, ZOrder(0)
		, Behavior(EWidgetBehavior::Toggle)
		, bRequiresMouseCursor(false)
		, InputMode(EUIInputMode::GameOnly)
		, bAddToViewport(true)
	{}
};

/**
 * 플레이어 UI 런타임 데이터 (동적 실행 데이터)
 *
 * 용도: 게임 실행 중 실제로 생성된 Widget 인스턴스들과 표시 상태를 관리
 * 시점: 런타임 (게임 플레이 중, InitializePlayerUI()에서 생성)
 * 예시: "현재 Inventory Widget 인스턴스가 생성되어 있고, 화면에 표시 중이다"
 *
 * FBSWidgetConfig와의 차이:
 * - FBSWidgetConfig: 설계도/레시피 (Blueprint에서 정의한 정적 설정)
 * - FPlayerUIData: 실제 만들어진 결과물 (실행 중인 Widget 인스턴스들)
 *
 * 비유:
 * - FBSWidgetConfig = 요리 레시피 ("피자는 이렇게 만든다")
 * - FPlayerUIData = 실제 만들어진 피자 ("지금 내 앞에 있는 피자")
 */
USTRUCT()
struct FPlayerUIData
{
	GENERATED_BODY()

	// 생성된 Widget 인스턴스들 (Key: Widget 이름, Value: 실제 Widget 객체)
	// 예: {"Inventory" → InventoryWidget*, "HealthBar" → HealthBarWidget*}
	// InitializePlayerUI()에서 FBSWidgetConfig를 기반으로 생성됨
	UPROPERTY()
	TMap<FName, TObjectPtr<UUserWidget>> Widgets;

	// 현재 화면에 표시 중인 Widget 이름 목록 (표시 순서대로 저장)
	// 예: ["HealthBar", "Inventory"] → HealthBar와 Inventory가 표시 중
	// 용도: ESC 키로 마지막에 연 Widget부터 역순으로 닫기
	UPROPERTY()
	TArray<FName> VisibleWidgets;

	FPlayerUIData()
	{
		Widgets.Empty();
		VisibleWidgets.Empty();
	}
};

/**
 * Widget Name 상수 정의
 *
 * 용도: Widget 이름을 문자열 대신 상수로 사용하여 오타 방지
 * 사용: UIManager->ShowWidget(BSUINames::Inventory);
 */
namespace BSUINames
{
	const FName Inventory = TEXT("Inventory");
	const FName HealthBar = TEXT("HealthBar");
	const FName Crosshair = TEXT("Crosshair");
	const FName PickupPrompt = TEXT("PickupPrompt");
	const FName Map = TEXT("Map");
	const FName AmmoDisplay = TEXT("AmmoDisplay");
}

/**
 * UI Config Name 상수 정의
 *
 * 용도: UI Config 이름을 문자열 대신 상수로 사용하여 오타 방지
 * 사용: UIManager->SwitchUIConfig(BSUIConfigNames::InGame);
 *
 * 예시:
 * - Default: 기본 UI (하위 호환용)
 * - InGame: 인게임 UI (HealthBar, AmmoDisplay, Crosshair)
 * - Lobby: 로비 UI (PlayerList, ChatBox, ReadyButton)
 * - Result: 결과 화면 UI (ScoreBoard, Stats)
 */
namespace BSUIConfigNames
{
	const FName Default = TEXT("Default");
	const FName InGame = TEXT("InGame");
	const FName Lobby = TEXT("Lobby");
	const FName Result = TEXT("Result");
}

/**
 * UI Config 테이블 Row (CSV/DataTable용)
 *
 * 용도: CSV 파일 또는 DataTable에서 UI Config 정보를 읽어오기 위한 구조체
 * CSV 형식: ConfigName, AssetPath
 *
 * 예시 CSV:
 * ConfigName,AssetPath
 * InGame,"/Game/DataAsset/DA_BugShowerUI.DA_BugShowerUI"
 * Lobby,"/Game/DataAsset/DA_LobbyUI.DA_LobbyUI"
 * Result,"/Game/DataAsset/DA_ResultUI.DA_ResultUI"
 *
 * 사용법:
 * 1. CSV 파일 생성 (Content/DataTables/DT_UIConfigs.csv)
 * 2. 에디터에서 DataTable 생성 (Row Structure: FBSUIConfigTableRow)
 * 3. CSV Import 또는 에디터에서 직접 편집
 * 4. BSUIManager의 UIConfigTable에 할당
 */
USTRUCT(BlueprintType)
struct FBSUIConfigTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * Config 이름 (예: InGame, Lobby, Result)
	 * SwitchUIConfig()에서 사용할 이름
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FName ConfigName;

	/**
	 * UI Config DataAsset 경로
	 * 예: /Game/DataAsset/DA_BugShowerUI.DA_BugShowerUI
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FSoftObjectPath ConfigAssetPath;

	FBSUIConfigTableRow()
		: ConfigName(NAME_None)
		, ConfigAssetPath()
	{
	}
};
