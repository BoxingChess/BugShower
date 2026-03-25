// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ThrowOrangeNotify.h"
#include "NPC/MonsterBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"
#include "Logging/BugShowerLog.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Projectile/MonsterProjectile.h"

FString SocketName = TEXT("hand_LSocket");

void UThrowOrangeNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	APawn* OwningPawn = Cast<APawn>(MeshComp->GetOwner());
	AMonsterBase* Monster = Cast<AMonsterBase>(OwningPawn);
	if (!Monster)
	{
		return;
	}

	if (OwningPawn)
	{
		// 폰을 조종하는 AI 컨트롤러 가져오기
		AAIController* AIC = Cast<AAIController>(OwningPawn->GetController());
		if (AIC)
		{
			// AI 컨트롤러가 사용하는 블랙보드 컴포넌트 접근
			UBlackboardComponent* BB = AIC->GetBlackboardComponent();
			if (BB)
			{

				AActor* Target = Cast<AActor>(BB->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR));

				// 기존 경로 삭제 (공격 후 재계산을 위해)
				AIC->StopMovement();

				TArray<AActor*> AttachedActors;
				Monster->GetAttachedActors(AttachedActors);

				for (AActor* AttachedActor : AttachedActors)
				{
					// 특정 소켓에 붙어 있는지 확인
					if (AttachedActor->GetRootComponent()->GetAttachSocketName() == SocketName)
					{
						// 탈착 규칙 설정
						// Location, Rotation, Scale 모두 KeepWorld로 설정하여 
						// 떨어지는 순간	소켓에 있던 그 자리에 그대로 멈춰 있게 합니다.
						FDetachmentTransformRules DetachRules(
							EDetachmentRule::KeepWorld,
							EDetachmentRule::KeepWorld,
							EDetachmentRule::KeepWorld,
							true // 가속도 유지 여부
						);

						// 부착되어 있던 액터(예: Orange)에서 호출
						AttachedActor->DetachFromActor(DetachRules);
					}
				}


				//발사
				Monster->FireProjectile(Target);
			}
		}
	}

}

void UThrowEndNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	APawn* OwningPawn = Cast<APawn>(MeshComp->GetOwner());
	AMonsterBase* Monster = Cast<AMonsterBase>(OwningPawn);

	if (!Monster)
	{
		return;
	}

	Monster->bIsThrowingOrange = false;


	if (OwningPawn)
	{
		// 폰을 조종하는 AI 컨트롤러 가져오기
		AAIController* AIC = Cast<AAIController>(OwningPawn->GetController());
		if (AIC)
		{
			// AI 컨트롤러가 사용하는 블랙보드 컴포넌트 접근
			UBlackboardComponent* BB = AIC->GetBlackboardComponent();
			if (BB)
			{
				FAIMoveRequest MoveReq(Monster->GetActorLocation());

				//던지기 초기화
				BB->SetValueAsBool(MONSTER_BOARD_KEY_ISINRANGEDRANGE, false);
			}
		}
	}
}

void UAttachOrangeNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);



	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	UPoolingSubsystem* PoolingManager = World->GetSubsystem<UPoolingSubsystem>();

	if (!PoolingManager)
	{
		return;
	}



	APawn* OwningPawn = Cast<APawn>(MeshComp->GetOwner());
	AMonsterBase* Monster = Cast<AMonsterBase>(OwningPawn);

	if (!Monster)
	{
		return;
	}

	//TScriptInterface<ISpawnable> Spawnable = PoolingManager->SpawnFromClass(Monster->BulletClass, FVector::Zero());
	/*if (!Spawnable)
	{
		return;
	}
	AMonsterProjectile* Orange = Cast<AMonsterProjectile>(Spawnable.GetObject());

	if (!Orange)
	{
		return;
	}*/

	// Monster가 들고 있을 액터(예: 오렌지)를 손 소켓에 부착
	//FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	//Orange->AttachToComponent(MeshComp, AttachRules, FName(SocketName));

}
