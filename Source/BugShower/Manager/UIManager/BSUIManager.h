// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BSUITypes.h"
#include "BSUIConfig.h"
#include "BSUIManager.generated.h"

class UInventoryComponent;
class AItemActor;

/**
 * UI 관리 Subsystem (CSV 기반 Multi-Config 시스템)
 *
 * ========================================
 * 📋 시스템 개요
 * ========================================
 * 게임의 모든 UI를 관리하는 중앙 시스템
 * - 상황별 UI Config 전환 (로비, 인게임, 결과 화면 등)
 * - CSV 파일로 Config 관리 (하드코딩 제거)
 * - Widget 생명주기 관리 (생성/표시/숨김/삭제)
 *
 * ========================================
 * 🎯 주요 기능
 * ========================================
 * 1. UI Config 관리 (CSV 기반)
 *    - CSV 파일: Content/DataTables/DT_UIConfigs.csv
 *    - 게임 시작 시 자동 로드
 *    - 기획자가 엑셀로 직접 수정 가능
 *
 * 2. Config 전환 (로비 ↔ 인게임 ↔ 결과)
 *    - SwitchUIConfig() 함수 사용
 *    - 기존 UI 완전 정리 후 새 UI 생성
 *    - Widget 메모리 누수 방지
 *
 * 3. Widget 표시/숨김
 *    - ShowWidget() / HideWidget()
 *    - Input Mode 자동 전환 (마우스 커서 제어)
 *
 * ========================================
 * 📝 사용 방법
 * ========================================
 *
 * ▶ 1. Config 추가 (CSV 수정)
 * Content/DataTables/DT_UIConfigs.csv 편집:
 * @code{.csv}
 * Name,ConfigName,ConfigAssetPath
 * InGame,InGame,"/Game/DataAsset/DA_BugShowerUI.DA_BugShowerUI"
 * Lobby,Lobby,"/Game/DataAsset/DA_LobbyUI.DA_LobbyUI"
 * @endcode
 *
 * ▶ 2. Config 전환 (C++)
 * @code
 * // GameMode나 PlayerController에서
 * UGameInstance* GI = GetGameInstance();
 * UBSUIManager* UIManager = GI->GetSubsystem<UBSUIManager>();
 *
 * // 로비 진입
 * UIManager->SwitchUIConfig(BSUIConfigNames::Lobby);
 *
 * // 인게임 진입
 * UIManager->SwitchUIConfig(BSUIConfigNames::InGame);
 * @endcode
 *
 * ▶ 3. Widget 표시/숨김
 * @code
 * // Inventory 열기
 * UIManager->ShowWidget(BSUINames::Inventory);
 *
 * // Inventory 닫기
 * UIManager->HideWidget(BSUINames::Inventory);
 * @endcode
 *
 * ========================================
 * ⚠️ 중요 주의사항
 * ========================================
 * 1. Initialize()는 게임 시작 시 자동 호출됨
 *    - ❌ 로비/인게임 전환 시 호출하지 마세요!
 *    - ✅ SwitchUIConfig() 사용
 *
 * 2. Config 전환 시 Widget 재생성됨
 *    - 기존 Widget 인스턴스는 모두 삭제
 *    - Widget에 저장한 상태는 사라짐
 *    - 필요시 GameState/PlayerState에 저장
 *
 * 3. 데디케이티드 서버 지원
 *    - 서버: UI 생성하지 않음 (헤드리스)
 *    - 클라이언트: 로컬 플레이어 UI만 관리
 *
 * ========================================
 * 🔄 코드 변경 이력
 * ========================================
 * [2025-12-04] CSV 기반 시스템으로 전환
 * - 하드코딩 제거: DataTable(CSV)에서 Config 로드
 * - 하위 호환 코드 제거: UIConfigAsset 삭제
 * - 주석 대폭 개선: 사용법 및 흐름 명시
 * - 에러 메시지 개선: 사용 가능한 Config 목록 표시
 */
UCLASS()
class BUGSHOWER_API UBSUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	/**
	 * 플레이어 UI 초기화 (플레이어 접속 시 호출)
	 * @param PC 플레이어 컨트롤러
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializePlayerUI(APlayerController* PC);

	/**
	 * 플레이어 UI 정리 (플레이어 해제 시 호출)
	 * @param PC 플레이어 컨트롤러
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CleanupPlayerUI(APlayerController* PC);

	/**
	 * Widget 표시
	 * @param WidgetName Widget 이름
	 * @param PC 플레이어 컨트롤러 (nullptr이면 첫 번째 로컬 플레이어)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowWidget(FName WidgetName, APlayerController* PC = nullptr);

	/**
	 * Widget 숨김
	 * @param WidgetName Widget 이름
	 * @param PC 플레이어 컨트롤러 (nullptr이면 첫 번째 로컬 플레이어)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideWidget(FName WidgetName, APlayerController* PC = nullptr);

	/**
	 * Widget 토글 (열기/닫기)
	 * @param WidgetName Widget 이름
	 * @param PC 플레이어 컨트롤러 (nullptr이면 첫 번째 로컬 플레이어)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleWidget(FName WidgetName, APlayerController* PC = nullptr);

	/**
	 * Widget 표시 상태 확인
	 * @param WidgetName Widget 이름
	 * @param PC 플레이어 컨트롤러 (nullptr이면 첫 번째 로컬 플레이어)
	 * @return Widget이 표시 중인지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsWidgetVisible(FName WidgetName, APlayerController* PC = nullptr) const;

	/**
	 * Widget 인스턴스 가져오기
	 * @param WidgetName Widget 이름
	 * @param PC 플레이어 컨트롤러 (nullptr이면 첫 번째 로컬 플레이어)
	 * @return Widget 인스턴스
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	UUserWidget* GetWidget(FName WidgetName, APlayerController* PC = nullptr) const;

	/**
	 * 인벤토리 UI 업데이트
	 * @param InventoryComp 인벤토리 컴포넌트
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateInventoryUI(UInventoryComponent* InventoryComp);

	/**
	 * 체력 UI 업데이트
	 * @param Health 현재 체력
	 * @param MaxHealth 최대 체력
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthUI(float Health, float MaxHealth);

	/**
	 * 아이템 획득 프롬프트 UI 업데이트
	 * @param Item 포커스된 아이템 (nullptr이면 숨김)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdatePickupPrompt(AItemActor* Item);

	/**
	 * 탄약 UI 업데이트
	 * @param CurrentAmmo 현재 탄창 탄약
	 * @param ReserveAmmo 예비 탄약
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateAmmoUI(int32 CurrentAmmo, int32 ReserveAmmo);

	/**
	 * UI Config 전환 (로비 → 인게임 → 결과창 등)
	 * @param ConfigName Config 이름 (예: "InGame", "Lobby", "Result")
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SwitchUIConfig(FName ConfigName);

	/**
	 * 현재 활성화된 UI Config 이름 가져오기
	 * @return 현재 Config 이름
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	FName GetCurrentConfigName() const { return CurrentConfigName; }

	/**
	 * 특정 Config 가져오기
	 * @param ConfigName Config 이름
	 * @return UI Config (없으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	UBSUIConfig* GetUIConfig(FName ConfigName) const;

protected:
	/**
	 * UI Config 테이블 (CSV/DataTable에서 Config 목록 로드) - 권장 방식!
	 *
	 * 사용 방법:
	 * 1. CSV 파일 생성: Content/DataTables/DT_UIConfigs.csv
	 *    형식: ConfigName, AssetPath
	 *    예시: InGame,"/Game/DataAsset/DA_BugShowerUI.DA_BugShowerUI"
	 * 2. 에디터에서 DataTable 생성 (Row Structure: FBSUIConfigTableRow)
	 * 3. CSV Import 또는 에디터에서 직접 편집
	 * 4. 여기에 할당
	 *
	 * 장점:
	 * - 하드코딩 제거 (경로를 CSV에서 관리)
	 * - 기획자가 엑셀로 직접 수정 가능
	 * - Git에서 텍스트 비교 가능
	 */
	/**
	 * UI Config 테이블 (CSV/DataTable에서 Config 목록 자동 로드)
	 *
	 * CSV 파일 위치: Content/DataTables/DT_UIConfigs.csv
	 * CSV 형식:
	 *   Name,ConfigName,ConfigAssetPath
	 *   InGame,InGame,"/Game/DataAsset/DA_BugShowerUI.DA_BugShowerUI"
	 *   Lobby,Lobby,"/Game/DataAsset/DA_LobbyUI.DA_LobbyUI"
	 *
	 * 설정 방법:
	 * 1. CSV 파일 수정 (엑셀에서 편집 가능)
	 * 2. Content Browser에서 DT_UIConfigs 우클릭 → Reimport
	 * 3. 자동으로 로드됨 (코드 수정 불필요)
	 *
	 * 장점:
	 * - 하드코딩 제거
	 * - 기획자가 직접 수정 가능
	 * - Git에서 텍스트 비교 가능
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (ToolTip = "UI Config 목록을 CSV/DataTable에서 자동 로드 (Content/DataTables/DT_UIConfigs.csv)"))
	TObjectPtr<UDataTable> UIConfigTable;

	/**
	 * UI Config 목록 (런타임 캐시)
	 * Key: Config 이름 (예: "InGame", "Lobby", "Result")
	 * Value: UI Config DataAsset
	 *
	 * 로드 방법:
	 * - Initialize() 시 UIConfigTable(DataTable)에서 자동으로 채워짐
	 * - 직접 수정하지 말 것! CSV 파일을 수정하세요.
	 *
	 * 사용 방법:
	 * - 게임 시작: 자동으로 "InGame" Config 로드
	 * - 로비 진입: SwitchUIConfig(BSUIConfigNames::Lobby)
	 * - 인게임 진입: SwitchUIConfig(BSUIConfigNames::InGame)
	 * - 결과 화면: SwitchUIConfig(BSUIConfigNames::Result)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (ToolTip = "런타임에 로드된 UI Config 목록 (읽기 전용)"))
	TMap<FName, TObjectPtr<UBSUIConfig>> UIConfigs;

	/**
	 * 현재 활성화된 UI Config 이름
	 */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	FName CurrentConfigName;

private:
	/**
	 * 로컬 플레이어 컨트롤러 (클라이언트당 1개)
	 */
	UPROPERTY()
	TObjectPtr<APlayerController> LocalPlayerController;

	/**
	 * 로컬 플레이어 UI 데이터 (클라이언트당 1개)
	 */
	UPROPERTY()
	FPlayerUIData UIData;

	/**
	 * Widget 설정 (런타임 캐시)
	 * Initialize()에서 UIConfigAsset으로부터 로드됨
	 */
	TMap<FName, FBSWidgetConfig> WidgetConfigs;

	/**
	 * Widget 생성
	 * @param WidgetName Widget 이름
	 * @return 생성된 Widget
	 */
	UUserWidget* BSCreateWidget(FName WidgetName);

	/**
	 * Input 모드 설정
	 * @param InputMode Input 모드
	 * @param WidgetToFocus 포커스할 Widget (선택)
	 */
	void SetInputMode(EUIInputMode InputMode, UUserWidget* WidgetToFocus = nullptr);

	/**
	 * 현재 활성 Widget에 맞는 Input 모드 계산
	 */
	void UpdateInputMode();
};
