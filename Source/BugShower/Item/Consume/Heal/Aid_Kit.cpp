// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Aid_Kit.h"
#include "Item/ItemEnum.h"

AAid_Kit::AAid_Kit()
{

}

void AAid_Kit::BeginPlay()
{
	Super::BeginPlay();

	FBS_Item defaultItem;
	defaultItem.ItemType = EItemType::Consumable;
	defaultItem.ItemID = static_cast<uint8>(EConsumableID::Aid_Kit);
	defaultItem.Quantity = 1;
	InitializeItemBS_Item(defaultItem);
}
