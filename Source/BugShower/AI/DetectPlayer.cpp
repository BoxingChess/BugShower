// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/DetectPlayer.h"
#include "NPC/MonsterBase.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"	
#include "Engine/OverlapResult.h"
#include "AI/MonsterBlackBoardKey.h"


UDetectPlayer::UDetectPlayer()
{
	bNotifyTick = true;
	NodeName = TEXT("Detect");	//노드이름
	Interval = 1.0f;	//간격
}

void UDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	//UE_LOG(LogTemp, Log, TEXT("DetectPlayer::TickNode called."));
	//Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


	//APawn* Monster = OwnerComp.GetAIOwner()->GetPawn();

	//if (!Monster)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Monster is Null in UDetectePlayer"));
	//	return;
	//}

	//UWorld* World = GetWorld();
	//if (!World)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("World is Null in UDetectePlayer"));
	//	return;
	//}

	//UMonsterStatComponent* MonsterStat = Monster->FindComponentByClass<UMonsterStatComponent>();

	//if (MonsterStat == nullptr)
	//{
	//	return;
	//}


	//FVector Center = Monster->GetActorLocation();
	//float DetectRadius = MonsterStat->GetPlayerDetectedRadius();

	//TArray<FOverlapResult> OverlapResults;
	//FCollisionShape CollisionShape = FCollisionShape::MakeSphere(DetectRadius);
	//FCollisionQueryParams CollisionQueryParam(SCENE_QUERY_STAT(Detect), false, Monster); //ingnor ovelap self

	//bool IsHit = World->OverlapMultiByChannel(OverlapResults, Center, FQuat::Identity, CHANNEL_HITCHECK, CollisionShape, CollisionQueryParam);

	//if (IsHit)
	//{
	//	for (const FOverlapResult& overlap : OverlapResults)
	//	{
	//		APlayerCharacter* Player = Cast<APlayerCharacter>(overlap.GetActor());
	//		if (Player)
	//		{
	//			OwnerComp.GetBlackboardComponent()->SetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR, Player);

	//			//DrawDebugSphere(World, Center, DetectRadius, 12, FColor::Green, false, 2.f);
	//			//DrawDebugLine(World, Center, Player->GetActorLocation(), FColor::Red, false, 2.f, 0, 2.f);
	//			//UE_LOG(LogTemp, Log, TEXT("Detected Player: %s"), *Player->GetName());
	//			// Here you can add logic to handle the detected player
	//			return;
	//		}

	//	}
	//}

 //  	OwnerComp.GetBlackboardComponent()->SetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR, nullptr);
	//DrawDebugSphere(World, Center, DetectRadius, 12, FColor::Green, false, 2.f);
}