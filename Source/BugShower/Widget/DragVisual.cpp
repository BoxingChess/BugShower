// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/DragVisual.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDragVisual::SetData(UTexture2D* InIcon, const FText& InQuantity)
{
		DragIcon_Img->SetBrushFromTexture(InIcon);
        DragText_Quntity->SetText(InQuantity);
}
