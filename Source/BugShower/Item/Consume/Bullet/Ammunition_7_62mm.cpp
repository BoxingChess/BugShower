// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammunition_7_62mm.h"

AAmmunition_7_62mm::AAmmunition_7_62mm()
{

}

void AAmmunition_7_62mm::BeginPlay()
{
	Super::BeginPlay();

	FBS_Item defaultItem;
	defaultItem.ItemType = EItemType::Consumable;
	defaultItem.ItemID = static_cast<uint8>(EConsumableID::Ammunition_7_62mm);
	defaultItem.Quantity = 30;
	InitializeItemBS_Item(defaultItem);
}
