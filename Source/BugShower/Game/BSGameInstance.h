// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"
#include "BSGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BUGSHOWER_API UBSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	public:
	virtual void Init() override;

	// TODO : 나중에 ItemResourceManager 추가하고 이건 Subsystem으로 바꿔주도록한다.
	UPROPERTY()
	UItemResourceManager* ItemResourceManager;

public:
	UItemResourceManager* GetItemResourceManager() ;
	

};
