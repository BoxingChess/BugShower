// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_DashAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"
#include "NPC/MonsterBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "Logging/BugShowerLog.h"

UBTTask_DashAttack::UBTTask_DashAttack()
{
	NodeName = TEXT("Dash Attack");
	bNotifyTick = true;
	StuckCheckTimer = 0.0f;
	TotalElapsedTime = 0.0f;
	LastPosition = FVector::ZeroVector;
}

EBTNodeResult::Type UBTTask_DashAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: AIController is null"));
		return EBTNodeResult::Failed;
	}

	AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: Pawn is not AMonsterBase"));
		return EBTNodeResult::Failed;
	}

	if (!Monster->HasAuthority())
	{
		LOG_BT_WARNING(TEXT("BTTask_DashAttack: No authority"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR));
	if (!Target)
	{
		LOG_BT_WARNING(TEXT("BTTask_DashAttack: No target found"));
		return EBTNodeResult::Failed;
	}

	FVector DashDirection = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
	FVector DashTargetLocation = Monster->GetActorLocation() + DashDirection * Monster->DashDistance;

	LastPosition = Monster->GetActorLocation();
	StuckCheckTimer = 0.0f;
	TotalElapsedTime = 0.0f;

	Monster->StartDash();

	LOG_BT(TEXT("BTTask_DashAttack: MonsterPos=(%.1f, %.1f, %.1f), DashTarget=(%.1f, %.1f, %.1f), DashDistance=%.1f"),
		Monster->GetActorLocation().X, Monster->GetActorLocation().Y, Monster->GetActorLocation().Z,
		DashTargetLocation.X, DashTargetLocation.Y, DashTargetLocation.Z,
		Monster->DashDistance);
	LOG_BT(TEXT("BTTask_DashAttack: MoveStatus before MoveTo: %d"), (int32)AIController->GetMoveStatus());

	UPathFollowingComponent* PFC = AIController->GetPathFollowingComponent();
	if (!PFC)
	{
		Monster->StopDash();
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: PathFollowingComponent is null"));
		return EBTNodeResult::Failed;
	}

	FAIMoveRequest MoveReq(DashTargetLocation);
	MoveReq.SetUsePathfinding(false);
	MoveReq.SetAcceptanceRadius(50.0f);
	//MoveReq.SetProjectGoalLocation(true);
	MoveReq.SetProjectGoalLocation(false);

	FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveReq);
	LOG_BT(TEXT("BTTask_DashAttack: MoveTo result code: %d"), (int32)MoveResult.Code);
	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		Monster->StopDash();
		LOG_BT_ERROR(TEXT("BTTask_DashAttack: MoveTo failed"));
		return EBTNodeResult::Failed;
	}

	LOG_BT(TEXT("BTTask_DashAttack: Starting dash. DashSpeed=%.1f, DashDistance=%.1f"),
		Monster->DashSpeed, Monster->DashDistance);

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

	TotalElapsedTime += DeltaSeconds;
	StuckCheckTimer += DeltaSeconds;

	if (TotalElapsedTime >= MaxDashDuration)
	{
		Monster->StopDash();
		LOG_BT(TEXT("BTTask_DashAttack: Timeout, ending dash"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (StuckCheckTimer >= StuckCheckInterval)
	{
		FVector CurrentPosition = Monster->GetActorLocation();
		float DistanceMoved = FVector::Dist(CurrentPosition, LastPosition);
		if (DistanceMoved < MinMoveDistanceThreshold)
		{
			Monster->StopDash();
			LOG_BT(TEXT("BTTask_DashAttack: Hit obstacle (moved only %.1fcm), ending dash"), DistanceMoved);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
		LastPosition = CurrentPosition;
		StuckCheckTimer = 0.0f;
	}

	EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
	if (MoveStatus != EPathFollowingStatus::Moving)
	{
		Monster->StopDash();
		LOG_BT(TEXT("BTTask_DashAttack: Dash completed"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
