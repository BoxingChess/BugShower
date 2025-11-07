// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor.h"
#include "Aid_Kit.generated.h"

/**
 구급상자입니다.
 */
UCLASS()
class BUGSHOWER_API AAid_Kit : public AItemActor
{
	GENERATED_BODY()
public:
	AAid_Kit();

public:
	void BeginPlay();
};
