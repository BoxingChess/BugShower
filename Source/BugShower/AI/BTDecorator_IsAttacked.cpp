// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsAttacked.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "AI//MonsterBlackBoardKey.h"
#include "AIController.h"

bool UBTDecorator_IsAttacked::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackBoard = OwnerComp.GetAIOwner()->GetBlackboardComponent();

	bResult = BlackBoard->GetValueAsBool(MONSTER_BOARD_KEY_ISATTACKED);


	return bResult;
}
