// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attacked.h"
#include "AIController.h"
#include "NPC/MonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/MonsterBlackBoardKey.h"

EBTNodeResult::Type UBTTask_Attacked::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/*AMonsterBase* Monster = Cast<AMonsterBase>(OwnerComp.GetAIOwner()->GetPawn());
	
	Monster->SetAttacked();
	
	FAICharacterAttackedFinished EndDelegate;
	EndDelegate.BindLambda(
		[&]()
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			UBlackboardComponent* BlackBoard = OwnerComp.GetAIOwner()->GetBlackboardComponent();
			BlackBoard->SetValueAsBool(MONSTER_BOARD_KEY_ISATTACKED, false);

		});

	Monster->SetEndAttackedAnimationDelegate(EndDelegate);
	*/

	return EBTNodeResult::InProgress;
}
