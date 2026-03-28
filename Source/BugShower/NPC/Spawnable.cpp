// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Spawnable.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

void ISpawnable::Activate(AActor* Actor, const FVector& Position)
{
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	for (UActorComponent* Comp : Actor->GetComponents())
	{
		Comp->SetComponentTickEnabled(true);

		// 컴포넌트 레벨 visibility 복원
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			Prim->SetVisibility(true, false);
		}

		// 애니메이션 재개
		if (USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(Comp))
		{
			Skel->bPauseAnims = false;
		}

		// CharacterMovement 복원
		if (UCharacterMovementComponent* CMC = Cast<UCharacterMovementComponent>(Comp))
		{
			CMC->GravityScale = 1.0f;
			CMC->SetMovementMode(MOVE_Walking);
		}

		// ProjectileMovement는 여기서 복원하지 않음
		// → 각 클래스의 Spawn()에서 ResumeMovement()로 명시적으로 제어
	}

	FVector SafePos = Position;
	UWorld* World = Actor->GetWorld();
	if (World)
	{
		World->FindTeleportSpot(Actor, SafePos, FRotator::ZeroRotator);
	}
	Actor->SetActorLocation(SafePos, false, nullptr, ETeleportType::TeleportPhysics);

	// State 복구: RootComponent (충돌) + PrimaryRenderComponent (렌더링)
	//RecreateStateIfNeeded(Cast<UPrimitiveComponent>(Actor->GetRootComponent()));
	//RecreateStateIfNeeded(GetPrimaryRenderComponent());
}

void ISpawnable::Deactivate(AActor* Actor)
{
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	for (UActorComponent* Comp : Actor->GetComponents())
	{
		Comp->SetComponentTickEnabled(false);

		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			// 컴포넌트 레벨 visibility 비활성화 (AttachToComponent 시 부모→자식 전파 방지)
			Prim->SetVisibility(false, false);

			// 물리 시뮬레이션 비활성화 (kinematic 상태로 전환 - 바디 재생성 없음)
			if (Prim->IsSimulatingPhysics())
			{
				Prim->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
				Prim->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
				Prim->SetSimulatePhysics(false);
			}
		}

		// 애니메이션: 업데이트 중지
		if (USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>(Comp))
		{
			Skel->SetComponentTickEnabled(false);
			Skel->bPauseAnims = true;
		}

		// CharacterMovement: 자체 중력/낙하 로직 중단
		if (UCharacterMovementComponent* CMC = Cast<UCharacterMovementComponent>(Comp))
		{
			CMC->DisableMovement();
			CMC->GravityScale = 0.0f;
		}

		// ProjectileMovement: 발사체 이동/중력 중단
		if (UProjectileMovementComponent* PMC = Cast<UProjectileMovementComponent>(Comp))
		{
			PMC->StopMovementImmediately();
			PMC->ProjectileGravityScale = 0.0f;
			PMC->Deactivate();
		}
	}
}

void ISpawnable::RecreateStateIfNeeded(UPrimitiveComponent* Prim)
{
	if (!Prim) return;

	if (!Prim->IsRenderStateCreated())
	{
		Prim->RecreateRenderState_Concurrent();

	}
	if (!Prim->IsPhysicsStateCreated())
	{
		Prim->RecreatePhysicsState();
	}
}
