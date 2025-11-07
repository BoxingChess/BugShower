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
 * UI 관리 Subsystem (데디케이티드 서버 환경용)
 * 각 클라이언트에서 로컬 플레이어의 UI를 관리
 * - 서버: UI 생성하지 않음 (헤드리스)
 * - 클라이언트: 자신의 로컬 플레이어 UI만 관리
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

protected:
	/**
	 * UI 설정 DataAsset
	 * Blueprint에서 설정하거나 Project Settings에서 지정
	 * 게임 시작 시 Initialize()에서 로드됨
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (ToolTip = "Widget 설정을 담고 있는 DataAsset (DA_BugShowerUI)"))
	TObjectPtr<UBSUIConfig> UIConfigAsset;

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
