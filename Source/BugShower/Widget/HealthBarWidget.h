// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

/*
 항상 표시되는 체력바 위젯이다.
 트링크나 진통제를 먹으면 스태미어 상승도 시각적으로 보이게 구현해줄꺼다.
 */
UCLASS()
class BUGSHOWER_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
};
