// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BSGameInstance.h"
#include "Manager/UIManager/BSUIManager.h"

void UBSGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - Game Instance initialized"));

	// Check if UIManager Subsystem exists
	UBSUIManager* UIManager = GetSubsystem<UBSUIManager>();
	if (UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::Init - UIManager Subsystem is available"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::Init - UIManager Subsystem NOT found!"));
	}
}

UItemResourceManager* UBSGameInstance::GetItemResourceManager()
{
	if (!ItemResourceManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::GetItemResourceManager - ItemResourceManager is NULL! TODO: Initialize it"));
	}
	return ItemResourceManager;
}
 