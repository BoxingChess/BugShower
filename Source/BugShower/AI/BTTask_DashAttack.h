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

	// Stuck detection variables
	FVector LastPosition;
	float StuckCheckTimer;
	float TotalElapsedTime;

	// Stuck detection thresholds
	static constexpr float StuckCheckInterval = 0.3f;  // Check every 0.3 seconds
	static constexpr float MinMoveDistanceThreshold = 50.0f;  // 50cm minimum movement
	static constexpr float MaxDashDuration = 3.0f;  // Maximum dash duration (safety timeout)
};
