// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Spawnable.h"
#include "GameFramework/Actor.h"

// Add default functionality here for any ISpawnable functions that are not pure virtual.

void ISpawnable::Activate(AActor* Actor, const FVector& Position)
{
    Actor->SetActorLocation(Position);
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
