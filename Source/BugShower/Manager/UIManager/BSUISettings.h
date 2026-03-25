// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "BSUISettings.generated.h"

/**
 * UI Manager Settings (Project Settings에서 설정)
 *
 * 설정 방법:
 * 1. Edit > Project Settings 열기
 * 2. "Game" 카테고리에서 "UI Settings" 찾기
 * 3. UIConfigTable에 DT_UIConfigs DataTable 할당
 *
 * 패키징 시 주의:
 * - 여기에 할당된 DataTable은 자동으로 패키징에 포함됨
 * - LoadObject로 하드코딩된 경로 사용 시 패키징에 포함 안 될 수 있음
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="UI Settings"))
class BUGSHOWER_API UBSUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * UI Config DataTable
	 *
	 * CSV 파일: Content/DataTables/DT_UIConfigs.csv
	 * Row Structure: FBSUIConfigTableRow
	 */
	UPROPERTY(Config, EditAnywhere, Category="UI", meta=(DisplayName="UI Config Table"))
	TSoftObjectPtr<UDataTable> UIConfigTable;

	// Project Settings 카테고리 이름
	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
