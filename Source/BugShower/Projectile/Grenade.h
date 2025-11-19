// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "Grenade.generated.h"

/**
 * 수류탄 프로젝타일 클래스
 *
 * 주요 기능:
 * - 포물선 궤도로 날아감 (중력 영향 받음)
 * - 바닥/벽에 튕김 (물리 시뮬레이션)
 * - 3초 후 자동 폭발
 * - 폭발 시 반경 데미지 적용 (RadialDamage)
 *
 * 동작 순서:
 * 1. 투척 → 포물선 비행
 * 2. 바닥/벽 충돌 → 튕김 (OnHit에서 노란색 구체 표시)
 * 3. 3초 후 → 폭발 (Explode 호출)
 * 4. 폭발 범위 내 모든 Actor에 데미지 + 시각 효과
 * 5. 수류탄 제거 (Destroy)
 *
 * 사용 예시:
 * - 플레이어가 우클릭으로 투척
 * - 인벤토리에서 수류탄 아이템 사용
 */
UCLASS()
class BUGSHOWER_API AGrenade : public AProjectileBase
{
	GENERATED_BODY()

public:
	AGrenade();

protected:
	virtual void BeginPlay() override;

	// ========================================
	// 수류탄 속성
	// ========================================

	/**
	 * 폭발 반경 (cm)
	 * 이 반경 내의 모든 Actor가 데미지를 받음
	 * 기본값: 500 (5m)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float ExplosionRadius = 500.0f;

	/**
	 * 폭발 데미지 (중심부)
	 * 폭발 중심에서 받는 최대 데미지
	 * 거리가 멀어질수록 데미지 감소
	 * 기본값: 100
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float ExplosionDamage = 100.0f;

	/**
	 * 폭발까지의 시간 (초)
	 * 수류탄이 생성된 후 이 시간이 지나면 자동 폭발
	 * 기본값: 3.0초
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float FuseTime = 3.0f;

	// ========================================
	// 내부 상태
	// ========================================

	/**
	 * 폭발 타이머 핸들
	 * FuseTime 후에 Explode() 함수를 호출하는 타이머
	 */
	FTimerHandle ExplosionTimerHandle;

	// ========================================
	// 폭발 로직
	// ========================================

	/**
	 * 수류탄 폭발 처리
	 *
	 * 수행 작업:
	 * 1. 폭발 위치 계산 (Z축 보정으로 지상에 표시)
	 * 2. RadialDamage 적용 (반경 내 모든 Actor에 데미지)
	 * 3. 시각 효과 표시:
	 *    - 주황색 구체 3개 겹쳐서 표시 (2.5초)
	 *    - 와이어프레임 구체 (3초)
	 * 4. 수류탄 제거 (Destroy)
	 *
	 * 호출 시점:
	 * - FuseTime(3초) 후 타이머에 의해 자동 호출
	 */
	UFUNCTION()
	void Explode();

	/**
	 * 충돌 이벤트 처리 (오버라이드)
	 *
	 * 수류탄은 충돌 시 즉시 파괴되지 않고:
	 * - 바닥/벽에서 튕김 (bShouldBounce = true)
	 * - 노란색 구체로 폭발 범위 미리보기 표시 (2초)
	 * - 데미지는 폭발 시에만 적용 (Damage = 0)
	 *
	 * @param HitComp - 충돌한 자신의 컴포넌트
	 * @param OtherActor - 충돌 대상 Actor
	 * @param OtherComp - 충돌 대상 컴포넌트
	 * @param NormalImpulse - 충돌 충격량
	 * @param Hit - 충돌 정보 (위치, 노멀 등)
	 */
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};
