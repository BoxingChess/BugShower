#include "MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackBoardComponent.h"
#include "AI/MonsterBlackBoardKey.h"

AMonsterAIController::AMonsterAIController()
{

}

void AMonsterAIController::RunAI()
{
	if (BP_BehaviorTree && BP_BlackBoard)
	{
		UE_LOG(LogTemp, Log, TEXT("MonsterAIController OnPossess called."));

		UBlackboardComponent* BlackboardPtr = Blackboard.Get();
		UseBlackboard(BP_BlackBoard, BlackboardPtr);
		BlackboardPtr->SetValueAsVector(MONSTER_BOARD_KEY_HOMEPOS, GetPawn()->GetActorLocation());

		bool isRun = RunBehaviorTree(BP_BehaviorTree);
		ensure(isRun);
	}
}

void AMonsterAIController::StopAI()
{
	UBehaviorTreeComponent* BehaviorTreeComp = Cast<UBehaviorTreeComponent>(BrainComponent);

	if (BehaviorTreeComp)
	{
		BehaviorTreeComp->StopTree();
	}
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AMonsterAIController::OnUnPossess()
{
	Super::OnUnPossess();

	StopAI();
}
