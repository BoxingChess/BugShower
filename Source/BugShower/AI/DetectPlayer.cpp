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
		LOG_BT(TEXT("Monster is Null"));
		return;
	}

	if (!Monster->HasAuthority())
	{
		LOG_BT(TEXT("Monster has not Authority"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_BT(TEXT("World is Null"));
		return;
	}

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Find the closest player pawn
	{
		AActor* ClosestPlayer = nullptr;
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

					if (Dist < ClosestDist)
					{
						ClosestDist = Dist;
						ClosestPlayer = PlayerPawn;
					}
				}
			}
		}

		if (ClosestPlayer)
		{
			BlackBoard->SetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR, ClosestPlayer);
			//BlackBoard->ClearValue(MONSTER_BOARD_KEY_TARGETPOS);
			BlackBoard->SetValueAsVector(MONSTER_BOARD_KEY_TARGETPOS, ClosestPlayer->GetActorLocation());

			//LOG_BT(TEXT("Closest Player Found! Distance: %f"), ClosestDist);
		}
	}
}