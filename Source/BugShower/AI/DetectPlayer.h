// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "DetectPlayer.generated.h"

/**
 * 
 */
UCLASS()
class BUGSHOWER_API UDetectPlayer : public UBTService
{
	GENERATED_BODY()
public:
	UDetectPlayer();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
