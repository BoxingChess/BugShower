// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_RangedAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"
#include "NPC/MonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Logging/BugShowerLog.h"

UBTTask_RangedAttack::UBTTask_RangedAttack()
{
	NodeName = TEXT("Ranged Attack");
	bNotifyTick = false;  // No need for tick, instant attack
}

EBTNodeResult::Type UBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get AI Controller
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		LOG_BT_ERROR(TEXT("BTTask_RangedAttack: AIController is null"));
		return EBTNodeResult::Failed;
	}

	// Get Monster
	AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		LOG_BT_ERROR(TEXT("BTTask_RangedAttack: Pawn is not AMonsterBase"));
		return EBTNodeResult::Failed;
	}

	// Check authority (network safety)
	if (!Monster->HasAuthority())
	{
		LOG_BT_WARNING(TEXT("BTTask_RangedAttack: No authority"));
		return EBTNodeResult::Failed;
	}

	// Get target from Blackboard
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR));

	if (Target == nullptr)
	{
		LOG_BT_WARNING(TEXT("BTTask_RangedAttack: No target found"));
		return EBTNodeResult::Failed;
	}

	// Fire projectile at target
	Monster->FireProjectile(Target);

	LOG_BT(TEXT("BTTask_RangedAttack: Fired projectile at %s"), *Target->GetName());

	// Task completes immediately after firing
	return EBTNodeResult::Succeeded;
}

