// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BSItem.h"
#include "BSStaticItemDataAsset.h"

#include "NPC/Spawnable.h"
#include "ItemEnum.h"

#include "ItemActor.generated.h"

UCLASS()
class BUGSHOWER_API AItemActor : public AActor, public ISpawnable
{
	GENERATED_BODY()

// Interface functions
public:
	virtual EPoolType GetPoolType() const override { return EPoolType::Item; }
	virtual void Spawn(const FVector pos) override;
	virtual void ReturnPool() override;

	//don't use super::LifeSpanExpired() in ISpawnable derived classes
	virtual void LifeSpanExpired() override;

	UFUNCTION()
	virtual void DeSpawn() override;


public:	
	// Sets default values for this actor's properties
	AItemActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnConstruction(const FTransform& Transform);
public:
	void InitializeItemBS_Item(FBS_Item& _item);

	FBS_Item& GetItemData();
	const UBSStaticItemDataAsset* GetItemStaticData() const { return StaticItemInfo; }

protected:
	// Dynamic item data (runtime state)
	UPROPERTY()
	FBS_Item ItemInformation;

	// Static item data (asset reference) - Now GC safe
	// 에디터에서 데이터 어셋을 직접 설정할 수 있습니다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<const UBSStaticItemDataAsset> StaticItemInfo = nullptr;

	// Flag to track if this actor came from pooling system
	UPROPERTY()
	bool bIsPooled = false;

public:
	void SetPooled(bool bValue) { bIsPooled = bValue; }
	bool IsPooled() const { return bIsPooled; }

protected:
	// Pickup function
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void OnPickup(AActor* PickupActor);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComponent;
};
