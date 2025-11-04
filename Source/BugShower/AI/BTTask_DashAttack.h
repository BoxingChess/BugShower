// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DashAttack.generated.h"

/**
 * Dash Attack Task - Monster dashes toward target in a straight line
 */
UCLASS()
class BUGSHOWER_API UBTTask_DashAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DashAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// Store original speed to restore after dash
	float OriginalMaxSpeed;
};
