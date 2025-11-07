// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BSItem.h"
#include "BSStaticItemDataAsset.h"

#include "ItemActor.generated.h"

UCLASS()
class BUGSHOWER_API AItemActor : public AActor
{
	GENERATED_BODY()
	
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

	FBS_Item& GetItemData() { return ItemInformation; }
	const UBSStaticItemDataAsset* GetItemStaticData() const { return StaticItemInfo; }

protected:
	// Dynamic item data (runtime state)
	UPROPERTY()
	FBS_Item ItemInformation;

	// Static item data (asset reference) - Now GC safe
	UPROPERTY()
	TObjectPtr<const UBSStaticItemDataAsset> StaticItemInfo = nullptr;

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
};
