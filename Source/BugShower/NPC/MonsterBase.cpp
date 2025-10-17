// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/MonsterBase.h"
#include "Net/UnrealNetwork.h"	

// Sets default values
AMonsterBase::AMonsterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	bReplicates = true;	//Replicate setting
	SetReplicateMovement(true);	//sync with position

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
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 100, 12, FColor::Red, false, 2.f);
}

// Called to bind functionality to input
void AMonsterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HP -= DamageAmount;
	if (HP <= 0)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		HP = 0;
	}

	

	return DamageAmount;
}

void AMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMonsterBase, Name);
	DOREPLIFETIME(AMonsterBase, MonsterGrade);
	DOREPLIFETIME(AMonsterBase, HP);
	DOREPLIFETIME(AMonsterBase, Damage);
}
