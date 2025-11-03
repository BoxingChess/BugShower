// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DetectPlayer.h"
#include "AI/MonsterBlackBoardKey.h"
#include "Logging/BugShowerLog.h"
#include "Kismet/GameplayStatics.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"	


#include "NPC/MonsterBase.h"
#include "AIController.h"
#include "Engine/OverlapResult.h"






UDetectPlayer::UDetectPlayer()
{
	bNotifyTick = true;
	NodeName = TEXT("DetectClosetPlayer");
	Interval = 0.1f;
}

void UDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

	APawn* Monster = OwnerComp.GetAIOwner()->GetPawn();
	UBlackboardComponent* BlackBoard = OwnerComp.GetBlackboardComponent();
	if (!Monster)
	{
		LOG_BT_WARNING(TEXT("Monster is Null"));
		return;
	}

	if (!Monster->HasAuthority())
	{
		LOG_BT_WARNING(TEXT("Monster has not Authority"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_BT_WARNING(TEXT("World is Null"));
		return;
	}

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Find the closest player pawn
	{
		AActor* CurClosestPlayer = nullptr;
		float ClosestDist = FLT_MAX;

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC)
			{
				APawn* PlayerPawn = PC->GetPawn();
				if (PlayerPawn)
				{
					float Dist = FVector::Dist(Monster->GetActorLocation(), PlayerPawn->GetActorLocation());

					// Find closest
					if (Dist < ClosestDist)
					{
						ClosestDist = Dist;
						CurClosestPlayer = PlayerPawn;
					}
				}
			}
		}


		if (CurClosestPlayer)
		{
			UObject* CurTargetValue = BlackBoard->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR);
			AActor* PreClosest = Cast<AActor>(CurTargetValue);

			// Check if target changed
			if (PreClosest != CurClosestPlayer)
			{
				LOG_BT(TEXT(">>> TARGET CHANGED: %s → %s (%.1f units)"),
					PreClosest ? *PreClosest->GetName() : TEXT("NONE"),
					*CurClosestPlayer->GetName(),
					ClosestDist);

				// Update TargetActor
				BlackBoard->SetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR, CurClosestPlayer);
			}
		}
		else
		{
			LOG_BT_WARNING(TEXT("No target! Monster[%s] found players but none valid"),
				*Monster->GetName());

			BlackBoard->ClearValue(MONSTER_BOARD_KEY_TARGETACTOR);
		}
	}
}