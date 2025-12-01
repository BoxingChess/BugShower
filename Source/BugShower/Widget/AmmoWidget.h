// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoWidget.generated.h"

/**
 * 탄약 표시 위젯
 * 현재 탄창 탄약과 예비 탄약을 표시
 */
UCLASS()
class BUGSHOWER_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 탄약 업데이트 (C++에서 호출, Blueprint에서 구현)
	 * @param CurrentAmmo 현재 탄창 탄약
	 * @param ReserveAmmo 예비 탄약
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateAmmo(int32 CurrentAmmo, int32 ReserveAmmo);
};
