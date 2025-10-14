// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_InAttackRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"


UBTDecorator_InAttackRange::UBTDecorator_InAttackRange()
{

}

bool UBTDecorator_InAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	/*
	APawn* Monster =  OwnerComp.GetAIOwner()->GetPawn();

	APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR));

	UMonsterStatComponent* StatComponent = Monster->GetComponentByClass<UMonsterStatComponent>();

	if (nullptr == Monster || nullptr == Player)
	{
		return false;
	}

	float DistanceToTarget = Monster->GetDistanceTo(Player);
	float AttackRangeWithRadius = StatComponent->GetAttackRange();
	bResult = (DistanceToTarget <= AttackRangeWithRadius);
	*/
	return bResult;
}
