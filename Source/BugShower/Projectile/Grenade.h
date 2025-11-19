// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "Grenade.generated.h"

UCLASS()
class BUGSHOWER_API AGrenade : public AProjectileBase
{
	GENERATED_BODY()

public:
	AGrenade();

protected:
	virtual void BeginPlay() override;

	// Grenade specific properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float ExplosionRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float ExplosionDamage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float FuseTime = 3.0f;

	// Timer handle for explosion
	FTimerHandle ExplosionTimerHandle;

	// Explosion function
	UFUNCTION()
	void Explode();

	// Override OnHit to explode on impact
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
