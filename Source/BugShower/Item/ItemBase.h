// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPC/Spawnable.h"
#include "ItemEnum.h"
#include "ItemBase.generated.h"


UCLASS()
class BUGSHOWER_API AItemBase : public AActor, public ISpawnable
{
	GENERATED_BODY()

	// Interface functions
public:
	virtual EPoolType GetPoolType() const override { return EPoolType::Item; }
	virtual void Spawn(const FVector pos) override;
	virtual void ReturnPool() override;
	virtual UPrimitiveComponent* GetPrimaryRenderComponent() override { return MeshComponent; }

	//don't use super::LifeSpanExpired() in ISpawnable derived classes
	virtual void LifeSpanExpired() override;

	UFUNCTION()
	virtual void DeSpawn() override;

protected:

public:
	AItemBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Item properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemValue;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComponent;

	// Pickup function
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void OnPickup(AActor* PickupActor);

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:
	UPROPERTY(EditAnywhere, Category = "Item")
	float ItemLifeSpan;
};
