// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Bullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABullet::ABullet()
{
	// 총알은 매우 빠르고 중력의 영향을 거의 받지 않음
	InitialSpeed = 10000.0f;  // 매우 빠른 속도 (10,000 cm/s = 100 m/s)
	MaxSpeed = 10000.0f;
	GravityScale = 0.0f;  // 중력 없음 (직선 비행)
	Damage = 40.0f;  // 기본 데미지 (실제로는 SetBulletDamage로 설정됨)

	// 충돌 반경 작게 (총알은 작음)
	if (CollisionComponent)
	{
		CollisionComponent->InitSphereRadius(2.0f);
	}

	// 기본 메쉬 설정 (Cylinder - 원통형 총알)
	// WeaponDataAsset에서 설정하지 않았을 때 사용됨
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultBulletMeshRef(TEXT("/Engine/BasicShapes/Cylinder"));
	if (DefaultBulletMeshRef.Succeeded())
	{
		BulletMesh = DefaultBulletMeshRef.Object;

		// 기본 메쉬 적용 (나중에 WeaponDataAsset에서 덮어씌워질 수 있음)
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(BulletMesh);
			MeshComponent->SetRelativeScale3D(BulletMeshScale);
			MeshComponent->SetRelativeRotation(BulletMeshRotation);
		}
	}

	// ProjectileMovement 설정
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InitialSpeed;
		ProjectileMovement->MaxSpeed = MaxSpeed;
		ProjectileMovement->ProjectileGravityScale = GravityScale;
		ProjectileMovement->bShouldBounce = false;  // 총알은 튕기지 않음
	}

	// 짧은 수명 (총알은 멀리 나가면 사라짐)
	InitialLifeSpan = 5.0f;

	// 헤드샷 배율 초기값
	HeadshotMultiplier = 2.0f;
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	// 메쉬는 WeaponComponent의 FireSingleProjectile에서 설정됨
	// 여기서는 추가 초기화 작업만 수행
}

void ABullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 자기 자신이나 Owner는 무시
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		// 헤드샷 판정 (Bone 이름이 "head"를 포함하는지 확인)
		bool bIsHeadshot = false;
		float FinalDamage = Damage;

		if (Hit.BoneName.IsValid() && Hit.BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase))
		{
			bIsHeadshot = true;
			FinalDamage *= HeadshotMultiplier;
		}

		// 데미지 적용
		UGameplayStatics::ApplyDamage(
			OtherActor,
			FinalDamage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		UE_LOG(LogTemp, Warning, TEXT("Bullet hit: %s - Damage: %.1f%s (BoneName: %s)"),
			*OtherActor->GetName(),
			FinalDamage,
			bIsHeadshot ? TEXT(" HEADSHOT!") : TEXT(""),
			*Hit.BoneName.ToString());

		// TODO: 피격 이펙트 생성 (파티클, 사운드 등)

		// 총알은 타격 즉시 파괴
		Destroy();
	}
}

void ABullet::SetBulletDamage(float InDamage, float InHeadshotMultiplier)
{
	Damage = InDamage;
	HeadshotMultiplier = InHeadshotMultiplier;
}
