// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_DashAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"
#include "NPC/MonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Logging/BugShowerLog.h"

UBTTask_DashAttack::UBTTask_DashAttack()
{
	NodeName = TEXT("Dash Attack");
	bNotifyTick = true;  // Enable Tick
	OriginalMaxSpeed = 0.0f;
}

EBTNodeResult::Type UBTTask_DashAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get AI Controller
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: AIController is null"));
		return EBTNodeResult::Failed;
	}

	// Get Monster
	AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: Pawn is not AMonsterBase"));
		return EBTNodeResult::Failed;
	}

	// Check authority (network safety)
	if (!Monster->HasAuthority())
	{
		LOG_BT_WARNING(TEXT("BTTask_DashAttack: No authority"));
		return EBTNodeResult::Failed;
	}

	// Get target from Blackboard
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR));

	if (Target == nullptr)
	{
		LOG_BT_WARNING(TEXT("BTTask_DashAttack: No target found"));
		return EBTNodeResult::Failed;
	}

	// Get CharacterMovement
	UCharacterMovementComponent* Movement = Monster->GetCharacterMovement();
	if (!Movement)
	{
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: CharacterMovement is null"));
		return EBTNodeResult::Failed;
	}

	// Calculate dash direction
	FVector DashDirection = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
	FVector DashTargetLocation = Monster->GetActorLocation() + DashDirection * Monster->DashDistance;

	LOG_BT(TEXT("BTTask_DashAttack: Starting dash to target. DashSpeed=%.1f, DashDistance=%.1f"),
		Monster->DashSpeed, Monster->DashDistance);

	// Store original speed
	OriginalMaxSpeed = Movement->MaxWalkSpeed;

	// Set dash speed
	Movement->MaxWalkSpeed = Monster->DashSpeed;

	// Move directly toward (straight line, no pathfinding)
	FAIMoveRequest MoveReq(DashTargetLocation);
	MoveReq.SetUsePathfinding(false);  // Disable pathfinding for straight dash
	MoveReq.SetAcceptanceRadius(50.0f);
	MoveReq.SetProjectGoalLocation(true);

	FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveReq);

	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		// Restore speed on failure
		Movement->MaxWalkSpeed = OriginalMaxSpeed;
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: MoveTo failed"));
		return EBTNodeResult::Failed;
	}

	// Return InProgress and continue in TickTask
	return EBTNodeResult::InProgress;
}

void UBTTask_DashAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check movement status
	EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();

	// If not moving anymore, dash is complete
	if (MoveStatus != EPathFollowingStatus::Moving)
	{
		// Restore original speed
		UCharacterMovementComponent* Movement = Monster->GetCharacterMovement();
		if (Movement)
		{
			Movement->MaxWalkSpeed = OriginalMaxSpeed;
			LOG_BT(TEXT("BTTask_DashAttack: Dash completed, speed restored to %.1f"), OriginalMaxSpeed);
		}

		// Finish task
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
