// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "WeaponComponent.generated.h"

/**
 * 캐릭터에 부착되어 모든 무기 기능을 처리하는 컴포넌트
 * Actor 방식보다 메모리 효율적이고 무기 교체가 간편함
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUGSHOWER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================
	// 무기 데이터
	// ========================================

protected:
	// 현재 장착된 무기의 데이터
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponData)
	TObjectPtr<UWeaponDataAsset> CurrentWeaponData;

	// 무기 데이터 복제 시 호출
	UFUNCTION()
	void OnRep_CurrentWeaponData();

	// ========================================
	// 탄약 & 재장전
	// ========================================

protected:
	// 현재 탄창에 남은 탄약
	UPROPERTY(Replicated)
	int32 CurrentAmmo;

	// 보유 중인 예비 탄약
	UPROPERTY(Replicated)
	int32 ReserveAmmo;

	// 재장전 중인지 여부
	UPROPERTY(Replicated)
	bool bIsReloading;

	// 재장전 타이머
	FTimerHandle ReloadTimerHandle;

	// ========================================
	// 발사 상태
	// ========================================

protected:
	// 현재 발사 중인지 (트리거를 당기고 있는지)
	UPROPERTY(Replicated)
	bool bIsFiring;

	// 마지막 발사 시간 (연사 속도 제어용)
	UPROPERTY()
	float LastFireTime;

	// 점사 모드에서 현재까지 발사한 탄환 수
	UPROPERTY()
	int32 CurrentBurstCount;

	// 현재 누적된 탄 퍼짐 각도
	UPROPERTY()
	float CurrentSpreadAngle;

	// 누적된 반동 (카메라 오프셋)
	UPROPERTY()
	FVector2D AccumulatedRecoil;

	// ========================================
	// 공개 인터페이스
	// ========================================

public:
	/**
	 * 무기 장착
	 * @param WeaponData - 장착할 무기의 DataAsset
	 * @param AmmoCount - 시작 시 보유할 예비 탄약 수 (0이면 탄창만)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(UWeaponDataAsset* WeaponData, int32 AmmoCount = 0);

	/**
	 * 무기 해제 (빈손 상태로)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon();

	/**
	 * 발사 시작 (마우스 왼쪽 버튼 누름)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	/**
	 * 발사 중지 (마우스 왼쪽 버튼 뗌)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	/**
	 * 재장전
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();

	/**
	 * 탄약 추가 (아이템 사용 시)
	 * @param Amount - 추가할 탄약 수
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddAmmo(int32 Amount);

	// ========================================
	// 내부 발사 로직
	// ========================================

protected:
	/**
	 * 실제 발사 처리
	 * Tick 또는 타이머에서 호출됨
	 */
	void Fire();

	/**
	 * 단일 탄환 발사 (Line Trace)
	 * @return 명중 여부
	 */
	bool FireSingleTrace();

	/**
	 * 단일 프로젝타일 발사 (Projectile Spawn)
	 * @return 발사 성공 여부
	 */
	bool FireSingleProjectile();

	/**
	 * 발사 시작 위치 계산
	 * FireOrigin 설정에 따라 카메라 중심 or 총구 위치 반환
	 * @return 발사 시작 위치 (World Space)
	 */
	FVector GetFireStartLocation() const;

	/**
	 * 발사 방향 계산 (탄 퍼짐 포함)
	 * @return 발사 방향 벡터 (정규화됨)
	 */
	FVector GetFireDirection() const;

	/**
	 * 현재 탄 퍼짐 각도 계산
	 * 조준 여부, 이동 여부, 점프 여부 등을 고려
	 * @return 최종 탄 퍼짐 각도 (도 단위)
	 */
	float CalculateCurrentSpread() const;

	/**
	 * 랜덤한 방향으로 벡터 퍼뜨리기
	 * @param Direction - 기본 방향
	 * @param SpreadAngle - 퍼짐 각도 (도 단위)
	 * @return 퍼진 방향 벡터
	 */
	FVector AddSpreadToDirection(const FVector& Direction, float SpreadAngle) const;

	/**
	 * 거리에 따른 데미지 계산
	 * @param Distance - 타겟까지의 거리
	 * @param bIsHeadshot - 헤드샷 여부
	 * @return 최종 데미지
	 */
	float CalculateDamage(float Distance, bool bIsHeadshot) const;

	/**
	 * 반동 적용 (카메라에 영향)
	 */
	void ApplyRecoil();

	/**
	 * 탄 퍼짐 증가 (발사 시)
	 */
	void IncreaseSpread();

	/**
	 * 탄 퍼짐 감소 (시간 경과)
	 * @param DeltaTime - 경과 시간
	 */
	void DecreaseSpread(float DeltaTime);

	/**
	 * 발사 가능 여부 확인
	 * @return 발사 가능하면 true
	 */
	bool CanFire() const;

	/**
	 * 발사 간격 확인 (연사 속도 제어)
	 * @return 충분한 시간이 지났으면 true
	 */
	bool HasFireIntervalPassed() const;

	/**
	 * 재장전 완료 콜백
	 */
	UFUNCTION()
	void OnReloadComplete();

	/**
	 * Owner WeaponActor의 메쉬 가져오기
	 */
	class USkeletalMeshComponent* GetWeaponMesh() const;

	/**
	 * Owner WeaponActor의 Owner Character 가져오기
	 */
	class ACharacter* GetOwnerCharacter() const;

	/**
	 * Owner Character의 카메라 컴포넌트 가져오기
	 */
	class UCameraComponent* GetOwnerCamera() const;

	// ========================================
	// 애니메이션 재생
	// ========================================

	/**
	 * 발사 애니메이션 재생
	 */
	void PlayFireAnimation();

	/**
	 * 재장전 애니메이션 재생
	 */
	void PlayReloadAnimation();

	// ========================================
	// Getter 함수들
	// ========================================

public:
	// 현재 탄창 탄약
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	// 예비 탄약
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	// 무기 장착 여부
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasWeaponEquipped() const { return CurrentWeaponData != nullptr; }

	// 재장전 중 여부
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bIsReloading; }

	// 발사 중 여부 (트리거를 당기고 있는지)
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsFiring() const { return bIsFiring; }

	// 현재 무기 데이터
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UWeaponDataAsset* GetCurrentWeaponData() const { return CurrentWeaponData; }

	// 현재 탄 퍼짐 각도
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentSpread() const { return CurrentSpreadAngle; }
};
