// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "ProjectileBase.generated.h"

UCLASS()
class BUGSHOWER_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	// Components (public으로 변경 - 외부에서 접근 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	// Projectile properties (public으로 변경 - 외부에서 설정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float InitialSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float GravityScale = 1.0f;

	// Initialize projectile velocity
	void InitVelocity(const FVector& ShootDirection);

protected:
	virtual void BeginPlay() override;

	// Collision handling
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
