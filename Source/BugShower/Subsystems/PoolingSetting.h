// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PoolingSetting.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Pooling Settings"))
  class UPoolingSettings : public UDeveloperSettings
  {
      GENERATED_BODY()

  public:
      // Pool configuration DataTable
      UPROPERTY(Config, EditAnywhere, Category="Pooling")
      TSoftObjectPtr<UDataTable> PoolConfigTable;

      // Monster drop DataTable
      UPROPERTY(Config, EditAnywhere, Category="Pooling")
      TSoftObjectPtr<UDataTable> MonsterDropTable;

      // 설정 이름 (Project Settings에 표시)
      virtual FName GetCategoryName() const override { return TEXT("Game"); }
  };