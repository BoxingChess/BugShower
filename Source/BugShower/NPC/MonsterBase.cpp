// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterBase.h"
#include "NPC/MonsterAIController.h"
#include "Net/UnrealNetwork.h"	
#include "NPC/MonsterStatComponent.h"

// Sets default values
AMonsterBase::AMonsterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;	//Replicate setting
	SetReplicateMovement(true);	//sync with position

	AIControllerClass = AMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::Spawned;

	// Create MonsterStatComponent
	MonsterStatComp = CreateDefaultSubobject<UMonsterStatComponent>(TEXT("MonsterStatComponent"));
}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MonsterStatComp->ApplyDamage(DeltaTime);
}

float AMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	MonsterStatComp->ApplyDamage(DamageAmount);
	
	return DamageAmount;
}

