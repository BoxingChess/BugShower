// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor.h"
#include "Ammunition_7_62mm.generated.h"

/**
 * 
 */
UCLASS()
class BUGSHOWER_API AAmmunition_7_62mm : public AItemActor
{
	GENERATED_BODY()

public:
	AAmmunition_7_62mm();

public:
	void BeginPlay();

};
