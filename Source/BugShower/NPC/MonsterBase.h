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

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Spawn(FVector SpawnLocation);
	void DeSpawn();


private:

	//stat
	UPROPERTY(EditAnywhere, Category = "Stat")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "Stat")
	Grade MonsterGrade;

	UPROPERTY(EditAnywhere, Category = "Stat")
	uint8 HP;

	UPROPERTY(EditAnywhere, Category = "Stat")
	uint8 Damage;
};
