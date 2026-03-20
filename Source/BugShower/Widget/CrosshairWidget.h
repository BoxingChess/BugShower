// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/**
 * 크로스헤어 위젯
 * 화면 중앙에 항상 표시되는 조준점
 *
 * 특징:
 * - AlwaysVisible 타입: 게임 시작부터 끝까지 항상 표시
 * - 화면 중앙 고정: 해상도 변경 시에도 중앙 유지
 * - 정적 이미지: 크로스헤어 텍스처만 표시 (동적 효과 없음)
 *
 * 사용 방법:
 * 1. WBP_Crosshair Blueprint 생성 (Parent Class: CrosshairWidget)
 * 2. Blueprint에 Image 위젯 추가 (Anchor: Center, T_CH001 텍스처)
 * 3. DA_BugShowerUI에 등록 (Behavior: AlwaysVisible, ZOrder: 10)
 */
UCLASS()
class BUGSHOWER_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

};
