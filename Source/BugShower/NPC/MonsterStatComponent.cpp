// Fill out your copyright notice in the Description page of Project Settings.

#include "NPC/MonsterStatComponent.h"
#include "Net/UnrealNetwork.h"

UMonsterStatComponent::UMonsterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// Default values
	CurHP = 100.f;
	MaxHP = 100.f;
	Damage = 10.f;
	Defense = 5.f;
	Grade = EMonsterGrade::NORMAL;
	Name = TEXT("Monster");
}

void UMonsterStatComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
}

void UMonsterStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMonsterStatComponent, Name);
	DOREPLIFETIME(UMonsterStatComponent, Grade);
	DOREPLIFETIME(UMonsterStatComponent, CurHP);
	DOREPLIFETIME(UMonsterStatComponent, MaxHP);
	DOREPLIFETIME(UMonsterStatComponent, Damage);
	DOREPLIFETIME(UMonsterStatComponent, Defense);
}

void UMonsterStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMonsterStatComponent::OnRep_ChangeHP()
{
	// HP changed on client - broadcast delegate
	OnHPChanged.Broadcast(CurHP, MaxHP);
}

void UMonsterStatComponent::CheckDeath()
{
	if (CurHP <= 0.f)
	{
		CurHP = 0.f;

		// Only broadcast death on server
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("Monster %s is Dead"), *GetOwner()->GetName());
			OnMonsterDeath.Broadcast(GetOwner());
		}
	}
}

void UMonsterStatComponent::SetCurrentHP(float NewHP)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return;
	}

	CurHP = FMath::Clamp(NewHP, 0.f, MaxHP);
	OnHPChanged.Broadcast(CurHP, MaxHP);

	CheckDeath();
}

void UMonsterStatComponent::SetMaxHP(float NewMaxHP)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	MaxHP = FMath::Max(NewMaxHP, 1.f);
	CurHP = FMath::Min(CurHP, MaxHP);
	OnHPChanged.Broadcast(CurHP, MaxHP);
}

void UMonsterStatComponent::ApplyDamage(float DamageAmount)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	if (IsDead())
		return;

	//UE_LOG(LogTemp, Warning, TEXT("Monster %s takes Damage: %f, CurHP : %f"), *GetOwner()->GetName(), DamageAmount,CurHP);
	
	float ActualDamage = FMath::Max(DamageAmount - Defense, 0.f);
	CurHP = FMath::Max(CurHP - ActualDamage, 0.f);

	OnHPChanged.Broadcast(CurHP, MaxHP);

	CheckDeath();
}

void UMonsterStatComponent::ResetHP()
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	CurHP = MaxHP;
	OnHPChanged.Broadcast(CurHP, MaxHP);
}

void UMonsterStatComponent::InitStats(FString InName, EMonsterGrade InGrade, float InMaxHP, float InDamage, float InDefense)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
		return;

	Name = InName;
	Grade = InGrade;
	MaxHP = InMaxHP;
	CurHP = InMaxHP;
	Damage = InDamage;
	Defense = InDefense;

	OnHPChanged.Broadcast(CurHP, MaxHP);
}
