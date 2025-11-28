// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "Bullet.generated.h"

/**
 * 총알 프로젝타일
 * 일반 총기에서 발사되는 탄환
 * - 빠른 속도로 직선 이동
 * - 중력 영향 거의 없음
 * - 타격 시 즉시 파괴
 */
UCLASS()
class BUGSHOWER_API ABullet : public AProjectileBase
{
	GENERATED_BODY()

public:
	ABullet();

protected:
	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

public:
	/**
	 * 총알 데미지와 헤드샷 배율 설정
	 * @param InDamage - 총알 기본 데미지
	 * @param InHeadshotMultiplier - 헤드샷 데미지 배율 (기본값: 2.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Bullet")
	void SetBulletDamage(float InDamage, float InHeadshotMultiplier = 2.0f);

protected:
	// 기본 메쉬 (WeaponDataAsset에서 설정하지 않았을 때 사용)
	UPROPERTY()
	UStaticMesh* BulletMesh;

	// 기본 메쉬 크기
	UPROPERTY()
	FVector BulletMeshScale = FVector(0.02f, 0.02f, 0.1f);

	// 기본 메쉬 회전
	UPROPERTY()
	FRotator BulletMeshRotation = FRotator(90.0f, 0.0f, 0.0f);

	// 헤드샷 데미지 배율
	UPROPERTY()
	float HeadshotMultiplier;
};
