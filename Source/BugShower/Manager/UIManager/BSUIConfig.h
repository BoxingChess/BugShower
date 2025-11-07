// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BSUITypes.h"
#include "BSUIConfig.generated.h"

/**
 * UI 설정 DataAsset
 *
 * 용도: Blueprint에서 편집 가능한 Widget 설정 저장소
 * 사용: Content Browser에서 DataAsset 생성 후 Widget 설정 입력
 * 예시: DA_BugShowerUI 에셋에 Inventory, HealthBar 등의 설정 저장
 *
 * 사용 흐름:
 * 1. Content Browser에서 BSUIConfig DataAsset 생성 (DA_BugShowerUI)
 * 2. Widget Configs 맵에 Widget 설정 추가
 * 3. GameInstance 또는 Project Settings에서 이 DataAsset 연결
 * 4. 게임 시작 시 UIManager가 자동으로 로드
 */
UCLASS(BlueprintType)
class BUGSHOWER_API UBSUIConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Widget 설정 목록
	 * Key: Widget 이름 (예: "Inventory", "HealthBar")
	 * Value: Widget 설정 정보 (FBSWidgetConfig)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (ToolTip = "게임에서 사용할 모든 Widget의 설정을 정의합니다."))
	TMap<FName, FBSWidgetConfig> WidgetConfigs;
};
