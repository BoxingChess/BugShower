// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Spawnable.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void ISpawnable::Activate(AActor* Actor, const FVector& Position)
{
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

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
