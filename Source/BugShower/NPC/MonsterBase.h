// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterBase.generated.h"


UENUM()
enum class Grade : uint8
{
	NORMAL,
	ELITE,
	BOSS
};

// Base class for all monsters
UCLASS()
class BUGSHOWER_API AMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


	// Get MonsterStatComponent
	UFUNCTION(BlueprintCallable, Category = "Stat")
	class UMonsterStatComponent* GetMonsterStatComponent() const { return MonsterStatComp; }

	// Drop item on death
	UFUNCTION()
	void OnDeath(AActor* DeadMonster);


protected:
	// Item drop table - set in Blueprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<TSubclassOf<class AItemBase>> DropTable;

	// Drop chance (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropChance;

	// Number of items to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MinDropCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MaxDropCount;

	// Drop items at monster location
	void DropItems();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UMonsterStatComponent* MonsterStatComp;

	//need animation comp
};
