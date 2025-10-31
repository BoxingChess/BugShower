// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	CONSUMABLE UMETA(DisplayName = "Consumable"),//소비
	EQUIPMENT UMETA(DisplayName = "Equipment"),//장비
	QUEST UMETA(DisplayName = "Quest"),//퀘스트
	MATERIAL UMETA(DisplayName = "Material")//재료
};

UCLASS()
class BUGSHOWER_API AItemBase : public AActor
{
	GENERATED_BODY()

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float LifeSpan;

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
	float CurrentLifeTime;
};
