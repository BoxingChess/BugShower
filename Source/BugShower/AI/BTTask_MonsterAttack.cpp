// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_MonsterAttack.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NPC/MonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/MonsterBlackBoardKey.h"

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	//// Get the AI controller from the behavior tree component
	//AAIController* AIController = OwnerComp.GetAIOwner();
	//if (!AIController)
	//{
	//	return EBTNodeResult::Failed;
	//}
	//// Get the controlled pawn
	//AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn());
	//if (!Monster)
	//{
	//	return EBTNodeResult::Failed;
	//}

	//UBlackboardComponent* BlackBoard = OwnerComp.GetAIOwner()->GetBlackboardComponent();
	//bool IsAttacked = BlackBoard->GetValueAsBool(MONSTER_BOARD_KEY_ISATTACKED);
	//if (IsAttacked)
	//{
	//	return EBTNodeResult::Succeeded;
	//}


	//Monster->BaseAttack();

	//FAICharacterAttackFinished EndAttackDelegate;
	//EndAttackDelegate.BindLambda(
	//	[&]()
	//	{
	//		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	//	});

	//Monster->SetEndAnimationDelegate(EndAttackDelegate);

	return EBTNodeResult::InProgress;
}
