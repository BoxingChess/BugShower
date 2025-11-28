// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LineTraceUI.generated.h"

class UImage;
class UTextBlock;

/**
 ///TIP
   BindWidget이 붙으면:
  - Graph에서 변수로 안 나타남
  - Designer에서 자동으로 같은 이름의 위젯과 연결됨
 */
UCLASS()
class BUGSHOWER_API ULineTraceUI : public UUserWidget
{
	GENERATED_BODY()

public:
    //F키 아이콘
	UPROPERTY(meta = (BindWidget))  //같은 이름의 블루프린트 위젯과 자동연결되게씀
    UImage* Image_FKeyIcon;

    //아이템 이름 + "습득" <-(고정) 이 되게끔.
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemInfo;
    
    //수량
    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Quntity;

public:
    // 데이터 세팅용 함수
    UFUNCTION(BlueprintCallable)
    void SetData(UTexture2D* InIcon, const FText& InQuantity);


};



