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
	Actor->SetActorLocation(SafePos, false, nullptr, ETeleportType::TeleportPhysics);

	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	
	//RenderProxy reactivation
	{
		for (UActorComponent* Comp : Components)
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
			{
				Prim->SetHiddenInGame(false);
				Prim->SetVisibility(true, true);

				if (!Prim->IsRegistered())
				{
					Prim->RegisterComponent();
				}

				if (!Prim->IsRenderStateCreated())
				{
					Prim->RecreateRenderState_Concurrent();
				}
				else
				{
					Prim->MarkRenderStateDirty();
				}
			}
		}
	}
}

void ISpawnable::Deactivate(AActor* Actor)
{
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
}
