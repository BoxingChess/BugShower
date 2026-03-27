// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "NPC/Spawnable.h"
#include "MonsterProjectile.generated.h"


UCLASS()
class BUGSHOWER_API AMonsterProjectile : public AActor, public ISpawnable
{
	GENERATED_BODY()

public:
	virtual void Spawn(const FVector pos) override;
	virtual void ReturnPool() override;
	virtual EPoolType GetPoolType() const override { return EPoolType::Bullet; }
	virtual UPrimitiveComponent* GetPrimaryRenderComponent() override;
	UFUNCTION()
	virtual void DeSpawn() override;
protected:

public:
	AMonsterProjectile();

protected:
	virtual void BeginPlay() override;

public:
	// Initialize projectile with direction and damage
	void InitializeProjectile(const FVector& Direction, float InDamage, AActor* InOwner);

	// Initialize projectile with exact velocity (for arc trajectories)
	void InitializeProjectileWithVelocity(const FVector& Velocity, float InDamage, AActor* InOwner);

	virtual void LifeSpanExpired() override;

	void PauseMovement();
	void ResumeMovement(bool bReset);
	void SetProjectileOwner(AActor* InOwner) { ProjectileOwner = InOwner; }


protected:
	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// Called when projectile hits something
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	// Collision component
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* CollisionComp;

	// Projectile movement component (handles arc trajectory)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	// Visual representation (optional - can be set in Blueprint)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* ProjectileMesh;

	TWeakObjectPtr<AActor> ProjectileOwner;

	// 풀 반환 전 공통 정리 — 모든 반환 경로에서 호출
	void ResetProjectileState();

	// Damage to deal on hit
	UPROPERTY()
	float Damage;

	UPROPERTY()
	float Life;
};
