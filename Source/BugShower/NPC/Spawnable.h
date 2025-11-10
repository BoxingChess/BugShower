// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NPC/PoolingType.h"
#include "Spawnable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USpawnable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */


class BUGSHOWER_API ISpawnable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//actor setting functions
	virtual void Activate(AActor* Actor, const FVector& Position);
	virtual void Deactivate(AActor* Actor);

	virtual EPoolType GetPoolType() const = 0;
	virtual void InitState(class AActor* InOwningSpawnPool) = 0;
	virtual void Spawn(const FVector pos) = 0;

	//forcing return to pool
	virtual void ReturnPool() = 0; 

	//UFUNCTION()
	//delegate function for event
	virtual void DeSpawn() = 0;	

protected:
};
