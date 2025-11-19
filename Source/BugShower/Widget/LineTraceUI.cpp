// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LineTraceUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void ULineTraceUI::SetData(UTexture2D* InIcon, const FText& InItemName)
{
	// F키 아이콘 설정 (현재는 InIcon 파라미터 사용 안 함)
	// TODO: Image_FKeyIcon에 F키 이미지 설정 (필요 시)

	// 아이템 이름 + "습득" 텍스트 설정
	if (Text_ItemInfo)
	{
		FText DisplayText = FText::Format(INVTEXT("{0} 습득"), InItemName);
		Text_ItemInfo->SetText(DisplayText);
	}

	// 수량 텍스트는 현재 파라미터에 없음
	// InItemName에 수량이 포함되어 있다고 가정하거나, 별도 처리 필요
	if (Text_Quntity)
	{
		// TODO: 수량 설정 로직 추가
		Text_Quntity->SetText(INVTEXT(""));
	}
}
