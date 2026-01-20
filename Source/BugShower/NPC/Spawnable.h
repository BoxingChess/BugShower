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
	virtual EPoolType GetPoolType() const = 0;

	// Activate object at specified position (called when spawning from pool)
	virtual void Spawn(const FVector pos) = 0;

	// Forcing return to pool (called externally)
	virtual void ReturnPool() = 0;

	// Delegate function for event (called when object wants to return itself)
	virtual void DeSpawn() = 0;

	// 각 클래스의 주요 렌더링 컴포넌트 반환 (State 복구용)
	virtual UPrimitiveComponent* GetPrimaryRenderComponent() { return nullptr; }

	// Helper functions for common activation/deactivation
	virtual void Activate(AActor* Actor, const FVector& Position);
	virtual void Deactivate(AActor* Actor);

private:
	void RecreateStateIfNeeded(UPrimitiveComponent* Prim);
};
