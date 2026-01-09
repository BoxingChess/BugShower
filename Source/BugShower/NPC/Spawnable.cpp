// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Spawnable.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// Add default functionality here for any ISpawnable functions that are not pure virtual.

void ISpawnable::Activate(AActor* Actor, const FVector& Position)
{

    
    FVector SafePos = Position;

    UWorld* World = Actor->GetWorld();
    World->FindTeleportSpot(Actor, SafePos, FRotator::ZeroRotator);

    // 3. Sweep 없이(false) 강제 이동 (텔레포트)
    Actor->SetActorLocation(SafePos, false, nullptr, ETeleportType::TeleportPhysics);

    Actor->SetActorHiddenInGame(false);
    Actor->SetActorEnableCollision(true);
    Actor->SetActorTickEnabled(true);
}

void ISpawnable::Deactivate(AActor* Actor)
{
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
}
