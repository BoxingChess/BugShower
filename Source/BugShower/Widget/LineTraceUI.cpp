// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/LineTraceUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void ULineTraceUI::SetData(UTexture2D* InIcon, const FText& InText)
{
        FKey_Icon->SetBrushFromTexture(InIcon);
        Txt_ItemInfo->SetText(InText);
}
