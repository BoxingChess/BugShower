// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ThrowOrangeNotify.h"
#include "NPC/MonsterBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterBlackBoardKey.h"
#include "Logging/BugShowerLog.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Projectile/MonsterProjectile.h"

#include "Components/SkeletalMeshComponent.h"   // USkeletalMeshComponent, QuerySupportedSockets, GetAllSocketNames
#include "Engine/SkeletalMesh.h"                 // USkeletalMesh, GetMeshOnlySocketList
#include "Engine/SkeletalMeshSocket.h"           // USkeletalMeshSocket (���� ������Ƽ ���� ��)

FString OrangeSocketName = TEXT("hand_LSocket");

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
		// ���� �����ϴ� AI ��Ʈ�ѷ� ��������
		AAIController* AIC = Cast<AAIController>(OwningPawn->GetController());
		if (AIC)
		{
			// AI ��Ʈ�ѷ��� ����ϴ� �������� ������Ʈ ����
			UBlackboardComponent* BB = AIC->GetBlackboardComponent();
			if (BB)
			{

				AActor* Target = Cast<AActor>(BB->GetValueAsObject(MONSTER_BOARD_KEY_TARGETACTOR));

				// ���� ��� ���� (���� �� ������ ����)
				AIC->StopMovement();



				TArray<AActor*> AttachedActors;
				Monster->GetAttachedActors(AttachedActors);

				for (AActor* AttachedActor : AttachedActors)
				{
					// Ư�� ���Ͽ� �پ� �ִ��� Ȯ��
					if (AttachedActor->GetRootComponent()->GetAttachSocketName() == OrangeSocketName)
					{
						// Ż�� ��Ģ ����
						// Location, Rotation, Scale ��� KeepWorld�� �����Ͽ� 
						// �������� ����	���Ͽ� �ִ� �� �ڸ��� �״�� ���� �ְ� �մϴ�.
						FDetachmentTransformRules DetachRules(
							EDetachmentRule::KeepWorld,
							EDetachmentRule::KeepWorld,
							EDetachmentRule::KeepWorld,
							true // ���ӵ� ���� ����
						);

						AttachedActor->DetachFromActor(DetachRules);

						// 탈착 후 CDO 기본 스케일로 복원
						FVector DefaultScale = AttachedActor->GetClass()->GetDefaultObject<AActor>()->GetActorScale3D();
						AttachedActor->SetActorScale3D(DefaultScale);

						// 충돌 복원 (부착 중 QueryOnly로 전환했던 것을 원래대로)
						UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(AttachedActor->GetRootComponent());
						if (Root)
						{
							Root->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						}

						// 발사를 위해 이동 복원
						AMonsterProjectile* Projectile = Cast<AMonsterProjectile>(AttachedActor);
						if (Projectile)
						{
							Projectile->ResumeMovement(true);
						}

						Monster->FireProjectile(Target, AttachedActor);
					}
				}


				//�߻�
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
		// ���� �����ϴ� AI ��Ʈ�ѷ� ��������
		AAIController* AIC = Cast<AAIController>(OwningPawn->GetController());
		if (AIC)
		{
			// AI ��Ʈ�ѷ��� ����ϴ� �������� ������Ʈ ����
			UBlackboardComponent* BB = AIC->GetBlackboardComponent();
			if (BB)
			{
				FAIMoveRequest MoveReq(Monster->GetActorLocation());

				//������ �ʱ�ȭ
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

	TScriptInterface<ISpawnable> Spawnable = PoolingManager->SpawnFromClass(Monster->BulletClass, FVector::Zero());
	/*
	*/
	if (!Spawnable)
	{
		return;
	}
	AMonsterProjectile* Orange = Cast<AMonsterProjectile>(Spawnable.GetObject());


	if (!Orange)
	{
		return;
	}

	// 오너 설정 - AttachToComponent 전에 해야 몬스터 캡슐 Overlap 오탐 방지
	Orange->SetProjectileOwner(Monster);

	// 움직임 중지
	Orange->PauseMovement();

	// 물리 바디가 소켓 트랜스폼 추적을 방해하지 않도록 QueryOnly로 전환
	UPrimitiveComponent* OrangeRoot = Cast<UPrimitiveComponent>(Orange->GetRootComponent());
	if (OrangeRoot)
	{
		OrangeRoot->SetSimulatePhysics(false);
		OrangeRoot->SetEnableGravity(false);
		OrangeRoot->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// Monster의 소켓 위치(예: 손가락)에 붙이기
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget,  // Location
		EAttachmentRule::SnapToTarget,  // Rotation
		EAttachmentRule::KeepRelative,  // Scale - 원본 스케일 유지
		true
	);
	bool bAttached = Orange->AttachToComponent(Monster->GetMesh(), AttachRules, FName(OrangeSocketName));

	// 부착 후 CDO 기본 스케일로 복원
	FVector DefaultScale = Orange->GetClass()->GetDefaultObject<AActor>()->GetActorScale3D();
	Orange->SetActorScale3D(DefaultScale);

	// AttachToComponent가 부모(Monster) 컴포넌트의 bHiddenInGame을 전파하므로 액터/컴포넌트 레벨 모두 복원
	Orange->SetActorHiddenInGame(false);
	for (UActorComponent* Comp : Orange->GetComponents())
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			Prim->SetVisibility(true, false);
		}
	}

	// 소켓 월드 위치
	FVector SocketLocation = Monster->GetMesh()->GetSocketLocation(FName(OrangeSocketName));

	// 부착 후 상태 디버그
	UE_LOG(LogTemp, Warning, TEXT("Orange Attach: %s | Hidden: %d | OrangeLoc: %s | SocketLoc: %s | Scale: %s"),
		bAttached ? TEXT("Success") : TEXT("Failed"),
		Orange->IsHidden(),
		*Orange->GetActorLocation().ToString(),
		*SocketLocation.ToString(),
		*Orange->GetActorScale3D().ToString());
}
