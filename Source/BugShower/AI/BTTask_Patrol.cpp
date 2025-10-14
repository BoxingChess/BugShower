// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Patrol.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"


UBTTask_Patrol::UBTTask_Patrol()
{

}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/*EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (ControllingPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());
	if (NavSystem == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UMonsterStatComponent* MonsterStat = ControllingPawn->FindComponentByClass<UMonsterStatComponent>();

	if (MonsterStat == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(MONSTER_BOARD_KEY_HOMEPOS);

	float AIPatrolRadius = MonsterStat->GetPatrolRadius();
	FNavLocation NextPatrolPos;

	if (NavSystem->GetRandomPointInNavigableRadius(Origin, AIPatrolRadius, NextPatrolPos))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(MONSTER_BOARD_KEY_PATROLPOS, NextPatrolPos);
		return EBTNodeResult::Succeeded;
	}*/



	return EBTNodeResult::Failed;
}
