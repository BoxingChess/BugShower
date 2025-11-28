// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BSGameInstance.h"
#include "Manager/UIManager/BSUIManager.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"

void UBSGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - Game Instance initialized"));

	// Verify that all subsystems are available
	// 모든 서브시스템이 사용 가능한지 확인

	// Check UIManager Subsystem
	// UIManager 서브시스템 확인
	UBSUIManager* UIManager = GetSubsystem<UBSUIManager>();
	if (UIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - UIManager Subsystem is available"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::Init - UIManager Subsystem NOT found!"));
	}

	// Check ItemResourceManager Subsystem
	// ItemResourceManager 서브시스템 확인
	UItemResourceManager* ItemResourceManager = GetSubsystem<UItemResourceManager>();
	if (ItemResourceManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - ItemResourceManager Subsystem is available"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::Init - ItemResourceManager Subsystem NOT found!"));
	}
}
 