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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	//돌진 공격 기능
	//장판 공격 기능
	//원겨리 투사체 공격 기능
	//공중 이동 기능

	//behaviortree 동작



	//stat
	UPROPERTY(Replicated,EditAnywhere, Category = "Stat")
	FString Name;

	UPROPERTY(Replicated,EditAnywhere, Category = "Stat")
	Grade MonsterGrade;

	UPROPERTY(Replicated,EditAnywhere, Category = "Stat")
	uint8 HP;

	UPROPERTY(Replicated,EditAnywhere, Category = "Stat")
	uint8 Damage;
};
