// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponTypes.h"
#include "WeaponDataAsset.generated.h"

/**
 * 무기의 모든 정적 데이터를 담는 DataAsset
 * Blueprint에서 각 무기마다 생성 (DA_M416, DA_AK47, DA_AWM 등)
 */
UCLASS()
class BUGSHOWER_API UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ========================================
	// 기본 정보
	// ========================================

	// 무기 이름 (UI에 표시될 이름)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText WeaponName;

	// 무기 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText Description;

	// 무기 타입 (소총, 저격총, 산탄총 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	EWeaponType WeaponType;

	// 무기 아이콘 (인벤토리에서 보여질 이미지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	UTexture2D* Icon;

	// ========================================
	// 메쉬 & 비주얼
	// ========================================

	// 무기 스켈레탈 메쉬 (실제 무기 3D 모델)
	// 테스트 시: /Engine/BasicShapes/Sphere 사용
	// 실제: /Game/Weapons/Meshes/SK_M416 같은 실제 메쉬로 교체
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* WeaponMesh;

	// 손에 부착될 때 위치 오프셋 (무기마다 다름)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FVector MeshOffset = FVector::ZeroVector;

	// 손에 부착될 때 회전 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FRotator MeshRotation = FRotator::ZeroRotator;

	// 손에 부착될 때 크기 (테스트용 Sphere는 0.1 정도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	// 총구 위치 소켓 이름 (총알이 발사되는 지점)
	// 무기 메쉬에 "Muzzle" 소켓이 있어야 함 -> 나중에 추가해야겠지?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	FName MuzzleSocketName = TEXT("Muzzle");

	// ========================================
	// 발사 메커니즘
	// ========================================

	// 발사 모드 (단발/연발/점사)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Basic")
	EFireMode FireMode;

	// 발사 시작점 (카메라 중심 or 총구)
	// Camera: FPS처럼 정확한 조준, Muzzle: TPS처럼 총구에서 발사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Basic")
	EFireOrigin FireOrigin = EFireOrigin::Camera;

	// 분당 발사 속도 (RPM - Rounds Per Minute)
	// 예: 600 = 1초에 10발, 900 = 1초에 15발
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Basic", meta = (ClampMin = "1", ClampMax = "1200"))
	float FireRate = 600.0f;

	// 점사 모드일 때 한 번에 발사되는 탄환 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Basic", meta = (ClampMin = "1", ClampMax = "10", EditCondition = "FireMode == EFireMode::Burst"))
	int32 BurstCount = 3;

	// ========================================
	// 프로젝타일 방식 (총알을 물리 오브젝트로 발사)
	// ========================================

	// 프로젝타일 방식 사용 여부
	// true: 프로젝타일을 스폰하여 발사 (물리 기반, 느린 총알)
	// false: 레이캐스트 방식 (즉시 히트, 빠른 총알)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile")
	bool bUseProjectile = true;

	// 발사할 프로젝타일 클래스 (bUseProjectile이 true일 때만 사용)
	// 예: ABullet::StaticClass()
	// 기본값은 Blueprint에서 설정하거나, 아래 생성자에서 설정 가능
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile", meta = (EditCondition = "bUseProjectile"))
	TSubclassOf<class AProjectileBase> ProjectileClass;

	// 프로젝타일 속도 (cm/s)
	// 기본값: 10000 (100 m/s) - 빠른 총알
	// 느린 무기: 3000-5000, 빠른 무기: 10000-20000
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile", meta = (ClampMin = "100", ClampMax = "50000", EditCondition = "bUseProjectile"))
	float ProjectileSpeed = 10000.0f;

	// 프로젝타일 중력 배율 (0.0 = 중력 없음, 1.0 = 기본 중력)
	// 총알은 보통 0.0 (직선), 유탄발사기는 1.0 (포물선)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile", meta = (ClampMin = "0.0", ClampMax = "5.0", EditCondition = "bUseProjectile"))
	float ProjectileGravityScale = 0.0f;

	// ========================================
	// 프로젝타일 외형 (총알 모양)
	// ========================================

	// 총알 메쉬 (에디터에서 선택 가능)
	// 기본값: Cylinder (원통형)
	// 예: /Engine/BasicShapes/Sphere, /Engine/BasicShapes/Cylinder, /Engine/BasicShapes/Cone
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile|Appearance", meta = (EditCondition = "bUseProjectile"))
	UStaticMesh* ProjectileMesh;

	// 총알 메쉬 크기
	// 예: (0.02, 0.02, 0.1) = 작고 긴 총알
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile|Appearance", meta = (EditCondition = "bUseProjectile"))
	FVector ProjectileMeshScale = FVector(0.02f, 0.02f, 0.1f);

	// 총알 메쉬 회전
	// 예: (90, 0, 0) = 앞으로 향하게 회전
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Projectile|Appearance", meta = (EditCondition = "bUseProjectile"))
	FRotator ProjectileMeshRotation = FRotator(90.0f, 0.0f, 0.0f);

	// ========================================
	// 데미지 & 사거리
	// ========================================

	// 기본 데미지 (근거리에서 명중 시)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Damage", meta = (ClampMin = "1", ClampMax = "200"))
	float BaseDamage = 40.0f;

	// 헤드샷 데미지 배율 (2.0 = 2배 데미지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Damage", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float HeadshotMultiplier = 2.0f;

	// 최대 사거리 (이 거리 이상은 데미지 0)
	// Line Trace 길이도 이 값 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Damage", meta = (ClampMin = "100", ClampMax = "100000"))
	float MaxRange = 50000.0f;

	// 거리 감쇠 시작 거리 (이 거리부터 데미지가 줄어들기 시작)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Damage", meta = (ClampMin = "100", ClampMax = "50000"))
	float DamageDropoffStart = 5000.0f;

	// 거리 감쇠율 (거리가 멀어질수록 데미지 감소 비율)
	// 0.5 = 최대 사거리에서 50% 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Damage", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float DamageDropoffRate = 0.5f;

	// ========================================
	// 정확도 & 탄 퍼짐
	// ========================================

	// 기본 탄 퍼짐 각도 (단위: 도)
	// 0 = 완벽한 정확도, 1 = 약간 퍼짐, 5 = 많이 퍼짐
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float BaseSpreadAngle = 0.5f;

	// 조준 중일 때 탄 퍼짐 배율 (0.5 = 50% 감소)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AimingSpreadMultiplier = 0.5f;

	// 이동 중일 때 탄 퍼짐 추가 각도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MovingSpreadIncrease = 2.0f;

	// 점프 중일 때 탄 퍼짐 추가 각도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float JumpingSpreadIncrease = 5.0f;

	// 발사할 때마다 증가하는 탄 퍼짐 (연사 시 정확도 감소)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float SpreadIncreasePerShot = 0.3f;

	// 탄 퍼짐 회복 속도 (초당 감소하는 각도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SpreadRecoveryRate = 2.0f;

	// 최대 탄 퍼짐 각도 (연사해도 이 이상 퍼지지 않음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Accuracy", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float MaxSpreadAngle = 5.0f;

	// ========================================
	// 산탄총 전용 (여러 발 동시 발사)
	// ========================================

	// 한 번에 발사되는 탄환 개수 (산탄총: 6~12개)
	// 일반 총기는 1, 산탄총만 여러 개
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Shotgun", meta = (ClampMin = "1", ClampMax = "20", EditCondition = "WeaponType == EWeaponType::Shotgun"))
	int32 PelletsPerShot = 8;

	// 산탄 퍼짐 패턴 각도 (산탄총만 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Shotgun", meta = (ClampMin = "1.0", ClampMax = "30.0", EditCondition = "WeaponType == EWeaponType::Shotgun"))
	float ShotgunSpreadAngle = 10.0f;

	// ========================================
	// 반동
	// ========================================

	// 발사 시 수직 반동 (카메라가 위로 올라가는 정도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Recoil", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float VerticalRecoil = 1.5f;

	// 발사 시 수평 반동 (카메라가 좌우로 흔들리는 정도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Recoil", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HorizontalRecoil = 0.5f;

	// 반동 회복 속도 (카메라가 원래 위치로 돌아오는 속도)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firing|Recoil", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float RecoilRecoverySpeed = 5.0f;

	// ========================================
	// 탄약
	// ========================================

	// 사용하는 탄약 종류
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	EAmmoType AmmoType;

	// 탄창 크기 (한 탄창에 들어가는 탄약 수)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1", ClampMax = "200"))
	int32 MagSize = 30;

	// 재장전 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float ReloadTime = 2.5f;

	// ========================================
	// 사운드 & 이펙트 (추후 추가)
	// ========================================

	// 발사 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	USoundBase* FireSound;

	// 재장전 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	USoundBase* ReloadSound;

	// 총구 섬광 파티클
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* MuzzleFlash;

	// 탄피 배출 파티클
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ShellEject;

	// 피격 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* ImpactEffect;
};
