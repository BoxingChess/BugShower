// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"
#include "Weapon/WeaponActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Projectile/Bullet.h"
#include "Projectile/ProjectileBase.h"
#include "Manager/UIManager/BSUIManager.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// 초기값 설정
	CurrentWeaponData = nullptr;
	CurrentAmmo = 0;
	ReserveAmmo = 0;
	bIsReloading = false;
	bIsFiring = false;
	LastFireTime = 0.0f;
	CurrentBurstCount = 0;
	CurrentSpreadAngle = 0.0f;
	AccumulatedRecoil = FVector2D::ZeroVector;
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 탄 퍼짐 점진적으로 감소 (시간이 지나면 정확도 회복)
	DecreaseSpread(DeltaTime);

	// 연발 모드일 때 계속 발사
	if (bIsFiring && CurrentWeaponData && CurrentWeaponData->FireMode == EFireMode::Auto)
	{
		if (CanFire() && HasFireIntervalPassed())
		{
			Fire();
		}
	}
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponComponent, CurrentWeaponData);
	DOREPLIFETIME(UWeaponComponent, CurrentAmmo);
	DOREPLIFETIME(UWeaponComponent, ReserveAmmo);
	DOREPLIFETIME(UWeaponComponent, bIsReloading);
}

void UWeaponComponent::OnRep_CurrentWeaponData()
{
	// 무기 데이터가 변경됨 (WeaponActor에서 메쉬 업데이트 처리)
	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::OnRep_CurrentWeaponData - Weapon data replicated"));
}

// ========================================
// 무기 장착/해제
// ========================================

void UWeaponComponent::EquipWeapon(UWeaponDataAsset* WeaponData, int32 AmmoCount)
{
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponComponent::EquipWeapon - WeaponData is null!"));
		return;
	}

	// 기존 무기 해제
	if (CurrentWeaponData)
	{
		UnequipWeapon();
	}

	// 새 무기 설정
	CurrentWeaponData = WeaponData;
	CurrentAmmo = WeaponData->MagSize;  // 탄창 가득 채움
	ReserveAmmo = AmmoCount;
	bIsReloading = false;
	bIsFiring = false;
	CurrentSpreadAngle = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::EquipWeapon - Equipped %s (Ammo: %d/%d)"),
		*WeaponData->WeaponName.ToString(), CurrentAmmo, ReserveAmmo);

	// UI 업데이트
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UBSUIManager* UIManager = GI->GetSubsystem<UBSUIManager>())
		{
			UIManager->UpdateAmmoUI(CurrentAmmo, ReserveAmmo);
		}
	}
}

void UWeaponComponent::UnequipWeapon()
{
	if (!CurrentWeaponData)
	{
		return;
	}

	// 발사 중지
	StopFire();

	// 재장전 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}

	// 데이터 초기화
	CurrentWeaponData = nullptr;
	CurrentAmmo = 0;
	ReserveAmmo = 0;
	bIsReloading = false;
	bIsFiring = false;
	CurrentSpreadAngle = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::UnequipWeapon - Weapon unequipped"));
}


// ========================================
// 발사
// ========================================

void UWeaponComponent::StartFire()
{
	if (!CurrentWeaponData)
	{
		return;
	}

	bIsFiring = true;
	CurrentBurstCount = 0;

	// 단발/점사 모드는 즉시 발사
	if (CurrentWeaponData->FireMode == EFireMode::Single || CurrentWeaponData->FireMode == EFireMode::Burst)
	{
		if (CanFire())
		{
			Fire();
		}
	}
	// 연발 모드는 Tick에서 처리
}

void UWeaponComponent::StopFire()
{
	bIsFiring = false;
	CurrentBurstCount = 0;
}

bool UWeaponComponent::CanFire() const
{
	// 무기가 없으면 발사 불가
	if (!CurrentWeaponData)
	{
		return false;
	}

	// 재장전 중이면 발사 불가
	if (bIsReloading)
	{
		return false;
	}

	// 탄약이 없으면 발사 불가
	if (CurrentAmmo <= 0)
	{
		// TODO: 빈 총 사운드 재생
		UE_LOG(LogTemp, Warning, TEXT("WeaponComponent::CanFire - Out of ammo!"));
		return false;
	}

	return true;
}

bool UWeaponComponent::HasFireIntervalPassed() const
{
	if (!CurrentWeaponData)
	{
		return false;
	}

	// RPM(분당 발사 속도)을 초당 발사 간격으로 변환
	// 예: 600 RPM = 60초 / 600발 = 0.1초 간격
	float FireInterval = 60.0f / CurrentWeaponData->FireRate;

	// 마지막 발사 이후 충분한 시간이 지났는지 확인
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastFireTime) >= FireInterval;
}

void UWeaponComponent::Fire()
{
	if (!CanFire())
	{
		return;
	}

	// 발사 간격 체크
	if (!HasFireIntervalPassed())
	{
		return;
	}

	// 프로젝타일 방식 vs 레이캐스트 방식 분기
	if (CurrentWeaponData->bUseProjectile)
	{
		// 프로젝타일 방식: 물리 기반 총알 발사
		// 산탄총 처리: 여러 발의 프로젝타일을 동시에 발사
		if (CurrentWeaponData->WeaponType == EWeaponType::Shotgun)
		{
			// PelletsPerShot 만큼 반복 발사
			for (int32 i = 0; i < CurrentWeaponData->PelletsPerShot; i++)
			{
				FireSingleProjectile();
			}
		}
		// 일반 총기: 1발만 발사
		else
		{
			FireSingleProjectile();
		}
	}
	else
	{
		// 레이캐스트 방식: 즉시 히트스캔
		// 산탄총 처리: 여러 발의 탄환을 동시에 발사
		if (CurrentWeaponData->WeaponType == EWeaponType::Shotgun)
		{
			// PelletsPerShot 만큼 반복 발사
			for (int32 i = 0; i < CurrentWeaponData->PelletsPerShot; i++)
			{
				FireSingleTrace();
			}
		}
		// 일반 총기: 1발만 발사
		else
		{
			FireSingleTrace();
		}
	}

	// 탄약 소모
	CurrentAmmo--;

	// 발사 시간 기록
	LastFireTime = GetWorld()->GetTimeSeconds();

	// 탄 퍼짐 증가
	IncreaseSpread();

	// 반동 적용
	ApplyRecoil();

	// 점사 모드 처리
	if (CurrentWeaponData->FireMode == EFireMode::Burst)
	{
		CurrentBurstCount++;
		if (CurrentBurstCount >= CurrentWeaponData->BurstCount)
		{
			// 점사 완료
			bIsFiring = false;
			CurrentBurstCount = 0;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::Fire - Fired! (Ammo: %d/%d)"), CurrentAmmo, ReserveAmmo);

	// UI 업데이트
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UBSUIManager* UIManager = GI->GetSubsystem<UBSUIManager>())
		{
			UIManager->UpdateAmmoUI(CurrentAmmo, ReserveAmmo);
		}
	}
}

bool UWeaponComponent::FireSingleTrace()
{
	if (!CurrentWeaponData)
	{
		return false;
	}

	// 발사 시작 위치
	FVector Start = GetFireStartLocation();

	// 발사 방향 (탄 퍼짐 포함)
	FVector Direction = GetFireDirection();

	// 발사 종료 위치 (사거리만큼)
	FVector End = Start + (Direction * CurrentWeaponData->MaxRange);

	// Line Trace 파라미터 설정
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());  // WeaponActor 무시
	if (ACharacter* OwnerChar = GetOwnerCharacter())
	{
		QueryParams.AddIgnoredActor(OwnerChar);  // 캐릭터도 무시
	}
	QueryParams.bTraceComplex = true;  // 복잡한 콜리전도 체크
	QueryParams.bReturnPhysicalMaterial = true;  // 물리 머티리얼 정보 가져오기 (헤드샷 판정용)

	// Line Trace 수행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,  // 가시성 채널 사용
		QueryParams
	);

	// 디버그 라인 그리기 (개발 빌드에서만)
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	DrawDebugLine(
		GetWorld(),
		Start,
		bHit ? HitResult.ImpactPoint : End,
		bHit ? FColor::Red : FColor::Green,
		false,
		0.5f,
		0,
		1.0f
	);
#endif

	// 명중 처리
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// 거리 계산
			float Distance = FVector::Dist(Start, HitResult.ImpactPoint);

			// 헤드샷 판정 (Bone 이름이 "head"를 포함하는지 확인)
			bool bIsHeadshot = false;
			if (HitResult.BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase))
			{
				bIsHeadshot = true;
			}

			// 데미지 계산
			float Damage = CalculateDamage(Distance, bIsHeadshot);

			// 데미지 적용 (Instigator는 캐릭터 컨트롤러)
			ACharacter* OwnerChar = GetOwnerCharacter();
			AController* InstigatorController = OwnerChar ? OwnerChar->GetController() : nullptr;

			UGameplayStatics::ApplyDamage(
				HitActor,
				Damage,
				InstigatorController,
				OwnerChar ? OwnerChar : GetOwner(),
				UDamageType::StaticClass()
			);

			UE_LOG(LogTemp, Log, TEXT("WeaponComponent::FireSingleTrace - Hit %s at %.1f units (Damage: %.1f%s)"),
				*HitActor->GetName(), Distance, Damage, bIsHeadshot ? TEXT(" HEADSHOT!") : TEXT(""));

			// TODO: 피격 이펙트 생성 (파티클, 사운드 등)
		}

		return true;
	}

	return false;
}

bool UWeaponComponent::FireSingleProjectile()
{
	if (!CurrentWeaponData)
	{
		return false;
	}

	// 프로젝타일 클래스가 설정되어 있는지 확인
	if (!CurrentWeaponData->ProjectileClass)
	{
		// 자동으로 Bullet 클래스 찾기 (Blueprint에서 설정하지 않은 경우)
		UE_LOG(LogTemp, Warning, TEXT("WeaponComponent::FireSingleProjectile - ProjectileClass not set, using default Bullet class"));

		// ABullet 클래스를 기본값으로 사용
		CurrentWeaponData->ProjectileClass = ABullet::StaticClass();

		if (!CurrentWeaponData->ProjectileClass)
		{
			UE_LOG(LogTemp, Error, TEXT("WeaponComponent::FireSingleProjectile - Failed to find Bullet class!"));
			return false;
		}
	}

	// 발사 시작 위치
	FVector Start = GetFireStartLocation();

	// 발사 방향 (탄 퍼짐 포함)
	FVector Direction = GetFireDirection();

	// 프로젝타일 스폰 위치와 회전
	// 총구 위치에서 바로 스폰 (SpawnParams에서 Owner Ignore 설정으로 충돌 방지)
	FVector SpawnLocation = Start;
	FRotator SpawnRotation = Direction.Rotation();

	// 스폰 파라미터
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();  // WeaponActor
	SpawnParams.Instigator = GetOwnerCharacter();  // Character
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 프로젝타일 스폰
	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
		CurrentWeaponData->ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (Projectile)
	{
		// 프로젝타일 속도 설정
		if (Projectile->ProjectileMovement)
		{
			Projectile->ProjectileMovement->InitialSpeed = CurrentWeaponData->ProjectileSpeed;
			Projectile->ProjectileMovement->MaxSpeed = CurrentWeaponData->ProjectileSpeed;
			Projectile->ProjectileMovement->ProjectileGravityScale = CurrentWeaponData->ProjectileGravityScale;
		}

		// 발사 방향으로 초기 속도 설정
		Projectile->InitVelocity(Direction);

		// 데미지 설정 (거리 감쇠는 프로젝타일 방식에서는 적용 안 함)
		// 프로젝타일은 거리에 상관없이 동일한 데미지
		Projectile->Damage = CurrentWeaponData->BaseDamage;

		// Bullet 클래스인 경우 추가 설정
		if (ABullet* Bullet = Cast<ABullet>(Projectile))
		{
			// 데미지와 헤드샷 배율 설정
			Bullet->SetBulletDamage(CurrentWeaponData->BaseDamage, CurrentWeaponData->HeadshotMultiplier);

			// 메쉬 설정 (WeaponDataAsset에서 설정한 값 적용)
			if (CurrentWeaponData->ProjectileMesh && Bullet->MeshComponent)
			{
				Bullet->MeshComponent->SetStaticMesh(CurrentWeaponData->ProjectileMesh);
				Bullet->MeshComponent->SetRelativeScale3D(CurrentWeaponData->ProjectileMeshScale);
				Bullet->MeshComponent->SetRelativeRotation(CurrentWeaponData->ProjectileMeshRotation);

				UE_LOG(LogTemp, Log, TEXT("WeaponComponent::FireSingleProjectile - Applied custom mesh: %s"),
					*CurrentWeaponData->ProjectileMesh->GetName());
			}
		}

		UE_LOG(LogTemp, Log, TEXT("WeaponComponent::FireSingleProjectile - Spawned projectile at %.1f, %.1f, %.1f"),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("WeaponComponent::FireSingleProjectile - Failed to spawn projectile!"));
	return false;
}

FVector UWeaponComponent::GetFireStartLocation() const
{
	if (!CurrentWeaponData)
	{
		return FVector::ZeroVector;
	}

	// TPS 방식: 항상 총구에서 발사 (시각적 일관성)
	// 방향은 GetFireDirection()에서 카메라 조준점을 향하도록 계산됨
	USkeletalMeshComponent* WeaponMesh = GetWeaponMesh();
	if (WeaponMesh)
	{
		// 무기 메쉬의 "Muzzle" 소켓 위치
		if (WeaponMesh->DoesSocketExist(CurrentWeaponData->MuzzleSocketName))
		{
			return WeaponMesh->GetSocketLocation(CurrentWeaponData->MuzzleSocketName);
		}

		// 소켓이 없으면 MuzzleOffset 사용 (무기 기준 상대 좌표, Scale 적용!)
		FVector WeaponLoc = WeaponMesh->GetComponentLocation();
		FRotator WeaponRot = WeaponMesh->GetComponentRotation();
		FVector WeaponScale = WeaponMesh->GetComponentScale();

		// MuzzleOffset에 메쉬 Scale 적용
		FVector ScaledOffset = CurrentWeaponData->MuzzleOffset * WeaponScale;
		return WeaponLoc + WeaponRot.RotateVector(ScaledOffset);
	}

	// 폴백: WeaponActor 위치
	if (AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}

FVector UWeaponComponent::GetFireDirection() const
{
	if (!CurrentWeaponData)
	{
		return FVector::ForwardVector;
	}

	// TPS 방식: 카메라 레이캐스트로 조준점 찾기 → 총구에서 조준점으로 방향 계산

	// 1. 카메라에서 레이캐스트로 조준점 찾기
	UCameraComponent* Camera = GetOwnerCamera();
	FVector TargetPoint;

	if (Camera)
	{
		FVector CameraLoc = Camera->GetComponentLocation();
		FVector CameraForward = Camera->GetForwardVector();
		FVector TraceEnd = CameraLoc + (CameraForward * CurrentWeaponData->MaxRange);

		// 레이캐스트 설정
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());  // WeaponActor 무시
		if (ACharacter* OwnerChar = GetOwnerCharacter())
		{
			Params.AddIgnoredActor(OwnerChar);  // 캐릭터 무시
		}

		// 레이캐스트 실행
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit, CameraLoc, TraceEnd, ECC_Visibility, Params
		);

		// 조준점: 히트 지점 또는 최대사거리 끝
		TargetPoint = bHit ? Hit.Location : TraceEnd;
	}
	else
	{
		// 카메라 없으면 폴백: 캐릭터 또는 Actor 방향
		FVector FallbackDirection = FVector::ForwardVector;
		if (ACharacter* OwnerChar = GetOwnerCharacter())
		{
			FallbackDirection = OwnerChar->GetActorForwardVector();
		}
		else if (AActor* OwnerActor = GetOwner())
		{
			FallbackDirection = OwnerActor->GetActorForwardVector();
		}

		// 총구 위치에서 앞으로
		FVector MuzzleLoc = GetFireStartLocation();
		TargetPoint = MuzzleLoc + (FallbackDirection * CurrentWeaponData->MaxRange);
	}

	// 2. 총구에서 조준점으로 방향 계산
	FVector MuzzleLoc = GetFireStartLocation();
	FVector Direction = (TargetPoint - MuzzleLoc).GetSafeNormal();

	// 3. 탄 퍼짐 계산
	float SpreadAngle = CalculateCurrentSpread();

	// 산탄총은 별도의 퍼짐 각도 사용
	if (CurrentWeaponData->WeaponType == EWeaponType::Shotgun)
	{
		SpreadAngle = CurrentWeaponData->ShotgunSpreadAngle;
	}

	// 4. 방향에 랜덤 퍼짐 추가
	return AddSpreadToDirection(Direction, SpreadAngle);
}

float UWeaponComponent::CalculateCurrentSpread() const
{
	if (!CurrentWeaponData)
	{
		return 0.0f;
	}

	// 기본 퍼짐
	float TotalSpread = CurrentWeaponData->BaseSpreadAngle;

	// 누적된 퍼짐 추가 (연사 시)
	TotalSpread += CurrentSpreadAngle;

	// 조준 중이면 퍼짐 감소
	// TODO: 조준 상태 확인 로직 추가 필요
	bool bIsAiming = false;  // 나중에 구현
	if (bIsAiming)
	{
		TotalSpread *= CurrentWeaponData->AimingSpreadMultiplier;
	}

	// 이동 중이면 퍼짐 증가
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (OwnerChar)
	{
		UCharacterMovementComponent* Movement = OwnerChar->GetCharacterMovement();
		if (Movement)
		{
			// 이동 중
			if (Movement->Velocity.Size() > 0.1f)
			{
				TotalSpread += CurrentWeaponData->MovingSpreadIncrease;
			}

			// 점프 중
			if (Movement->IsFalling())
			{
				TotalSpread += CurrentWeaponData->JumpingSpreadIncrease;
			}
		}
	}

	// 최대 퍼짐 제한
	TotalSpread = FMath::Clamp(TotalSpread, 0.0f, CurrentWeaponData->MaxSpreadAngle);

	return TotalSpread;
}

FVector UWeaponComponent::AddSpreadToDirection(const FVector& Direction, float SpreadAngle) const
{
	if (SpreadAngle <= 0.0f)
	{
		return Direction;
	}

	// 랜덤한 2D 방향 생성 (원형 분포)
	float RandomAngle = FMath::FRandRange(0.0f, 360.0f);
	float RandomRadius = FMath::FRandRange(0.0f, SpreadAngle);

	// 각도를 라디안으로 변환
	float AngleRad = FMath::DegreesToRadians(RandomAngle);
	float RadiusRad = FMath::DegreesToRadians(RandomRadius);

	// 카메라의 로컬 좌표계에서 퍼짐 계산
	FVector Right = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
	FVector Up = FVector::CrossProduct(Right, Direction).GetSafeNormal();

	// 랜덤 오프셋 추가
	float OffsetX = FMath::Cos(AngleRad) * FMath::Sin(RadiusRad);
	float OffsetY = FMath::Sin(AngleRad) * FMath::Sin(RadiusRad);

	FVector SpreadDirection = Direction + (Right * OffsetX) + (Up * OffsetY);
	return SpreadDirection.GetSafeNormal();
}

float UWeaponComponent::CalculateDamage(float Distance, bool bIsHeadshot) const
{
	if (!CurrentWeaponData)
	{
		return 0.0f;
	}

	// 기본 데미지
	float Damage = CurrentWeaponData->BaseDamage;

	// 헤드샷 배율 적용
	if (bIsHeadshot)
	{
		Damage *= CurrentWeaponData->HeadshotMultiplier;
	}

	// 거리 감쇠 계산
	if (Distance > CurrentWeaponData->DamageDropoffStart)
	{
		// 감쇠 구간에서의 비율 (0.0 ~ 1.0)
		float DropoffRange = CurrentWeaponData->MaxRange - CurrentWeaponData->DamageDropoffStart;
		float DistanceInDropoff = Distance - CurrentWeaponData->DamageDropoffStart;
		float DropoffRatio = FMath::Clamp(DistanceInDropoff / DropoffRange, 0.0f, 1.0f);

		// 데미지 감소 (DamageDropoffRate = 최대 거리에서 남는 데미지 비율)
		float DamageMultiplier = FMath::Lerp(1.0f, CurrentWeaponData->DamageDropoffRate, DropoffRatio);
		Damage *= DamageMultiplier;
	}

	// 최대 사거리 초과 시 데미지 0
	if (Distance > CurrentWeaponData->MaxRange)
	{
		Damage = 0.0f;
	}

	return Damage;
}

void UWeaponComponent::ApplyRecoil()
{
	if (!CurrentWeaponData)
	{
		return;
	}

	// 수직/수평 반동 계산
	float VerticalRecoil = CurrentWeaponData->VerticalRecoil;
	float HorizontalRecoil = FMath::FRandRange(-CurrentWeaponData->HorizontalRecoil, CurrentWeaponData->HorizontalRecoil);

	// 반동 누적
	AccumulatedRecoil.X += HorizontalRecoil;
	AccumulatedRecoil.Y += VerticalRecoil;

	// TODO: 캐릭터 컨트롤러에 반동 적용
	// PlayerController->AddPitchInput(-VerticalRecoil);
	// PlayerController->AddYawInput(HorizontalRecoil);

	UE_LOG(LogTemp, Verbose, TEXT("WeaponComponent::ApplyRecoil - Vertical: %.2f, Horizontal: %.2f"),
		VerticalRecoil, HorizontalRecoil);
}

void UWeaponComponent::IncreaseSpread()
{
	if (!CurrentWeaponData)
	{
		return;
	}

	// 발사할 때마다 퍼짐 증가
	CurrentSpreadAngle += CurrentWeaponData->SpreadIncreasePerShot;

	// 최대 퍼짐 제한
	CurrentSpreadAngle = FMath::Clamp(CurrentSpreadAngle, 0.0f, CurrentWeaponData->MaxSpreadAngle);
}

void UWeaponComponent::DecreaseSpread(float DeltaTime)
{
	if (!CurrentWeaponData)
	{
		return;
	}

	// 시간이 지나면 퍼짐 감소 (정확도 회복)
	CurrentSpreadAngle -= CurrentWeaponData->SpreadRecoveryRate * DeltaTime;
	CurrentSpreadAngle = FMath::Max(CurrentSpreadAngle, 0.0f);
}

// ========================================
// 재장전
// ========================================

void UWeaponComponent::Reload()
{
	// 이미 재장전 중이면 무시
	if (bIsReloading)
	{
		return;
	}

	// 무기가 없으면 무시
	if (!CurrentWeaponData)
	{
		return;
	}

	// 예비 탄약이 없으면 무시
	if (ReserveAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponComponent::Reload - No reserve ammo!"));
		return;
	}

	// 이미 탄창이 가득 차 있으면 무시
	if (CurrentAmmo >= CurrentWeaponData->MagSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponComponent::Reload - Magazine is already full!"));
		return;
	}

	// 발사 중지
	StopFire();

	// 재장전 시작
	bIsReloading = true;

	// 재장전 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&UWeaponComponent::OnReloadComplete,
		CurrentWeaponData->ReloadTime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::Reload - Reloading... (%.1f seconds)"), CurrentWeaponData->ReloadTime);

	// TODO: 재장전 애니메이션/사운드 재생
}

void UWeaponComponent::OnReloadComplete()
{
	if (!CurrentWeaponData)
	{
		bIsReloading = false;
		return;
	}

	// 필요한 탄약 수 계산
	int32 AmmoNeeded = CurrentWeaponData->MagSize - CurrentAmmo;

	// 예비 탄약에서 가져올 수 있는 만큼 가져옴
	int32 AmmoToReload = FMath::Min(AmmoNeeded, ReserveAmmo);

	// 탄약 이동
	CurrentAmmo += AmmoToReload;
	ReserveAmmo -= AmmoToReload;

	// 재장전 완료
	bIsReloading = false;

	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::OnReloadComplete - Reload complete! (Ammo: %d/%d)"),
		CurrentAmmo, ReserveAmmo);

	// UI 업데이트
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UBSUIManager* UIManager = GI->GetSubsystem<UBSUIManager>())
		{
			UIManager->UpdateAmmoUI(CurrentAmmo, ReserveAmmo);
		}
	}

	// TODO: 재장전 완료 사운드 재생
}

void UWeaponComponent::AddAmmo(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	ReserveAmmo += Amount;

	UE_LOG(LogTemp, Log, TEXT("WeaponComponent::AddAmmo - Added %d ammo (Total: %d)"), Amount, ReserveAmmo);

	// UI 업데이트
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (UBSUIManager* UIManager = GI->GetSubsystem<UBSUIManager>())
		{
			UIManager->UpdateAmmoUI(CurrentAmmo, ReserveAmmo);
		}
	}
}

// ========================================
// 유틸리티
// ========================================

USkeletalMeshComponent* UWeaponComponent::GetWeaponMesh() const
{
	// WeaponComponent의 Owner는 WeaponActor여야 함
	class AWeaponActor* WeaponActor = Cast<class AWeaponActor>(GetOwner());
	if (WeaponActor)
	{
		return WeaponActor->GetWeaponMesh();
	}

	return nullptr;
}

ACharacter* UWeaponComponent::GetOwnerCharacter() const
{
	// WeaponActor의 Owner가 Character
	if (AActor* OwnerActor = GetOwner())
	{
		if (AActor* OuterOwner = OwnerActor->GetOwner())
		{
			return Cast<ACharacter>(OuterOwner);
		}
	}

	return nullptr;
}

UCameraComponent* UWeaponComponent::GetOwnerCamera() const
{
	ACharacter* OwnerChar = GetOwnerCharacter();
	if (!OwnerChar)
	{
		return nullptr;
	}

	// 캐릭터의 카메라 컴포넌트 찾기
	return OwnerChar->FindComponentByClass<UCameraComponent>();
}
