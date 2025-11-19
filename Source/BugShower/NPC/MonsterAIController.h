#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

UCLASS()
class BUGSHOWER_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
public:
	AMonsterAIController();

	void RunAI();
	void StopAI();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;



protected:
	UPROPERTY(EditAnyWhere,BlueprintReadWrite,Category = Behavior, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBehaviorTree> BP_BehaviorTree;	

	UPROPERTY(EditAnyWhere,BlueprintReadWrite,Category = Behavior, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBlackboardData> BP_BlackBoard;
};


