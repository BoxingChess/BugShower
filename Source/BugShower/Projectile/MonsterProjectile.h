// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterProjectile.generated.h"

UCLASS()
class BUGSHOWER_API AMonsterProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMonsterProjectile();

protected:
	virtual void BeginPlay() override;

public:
	// Initialize projectile with direction and damage
	void InitializeProjectile(const FVector& Direction, float InDamage, AActor* InOwner);

protected:
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

	// Damage to deal on hit
	UPROPERTY()
	float Damage;

	// Owner of this projectile (the monster that fired it)
	UPROPERTY()
	AActor* ProjectileOwner;
};
