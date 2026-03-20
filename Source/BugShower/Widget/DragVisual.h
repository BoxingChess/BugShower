// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DragVisual.generated.h"
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class BUGSHOWER_API UDragVisual : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))  
    UImage* DragIcon_Img;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* DragText_Quntity;

public:
    // 데이터 세팅용 함수
    UFUNCTION(BlueprintCallable)
    void SetData(UTexture2D* InIcon, const FText& InQuantity);
};
