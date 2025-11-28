// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BSGameInstance.generated.h"

/**
 * BugShower Game Instance
 * BugShower 게임 인스턴스
 *
 * Main game instance class for BugShower project
 * BugShower 프로젝트의 메인 게임 인스턴스 클래스
 *
 * Uses Subsystem pattern for managers (UIManager, ItemResourceManager)
 * 매니저들은 Subsystem 패턴을 사용 (UIManager, ItemResourceManager)
 * - UIManager: UI 관리 서브시스템
 * - ItemResourceManager: 아이템 리소스 관리 서브시스템
 */
UCLASS()
class BUGSHOWER_API UBSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Game Instance initialization
	// 게임 인스턴스 초기화 (서브시스템 확인)
	virtual void Init() override;
};
