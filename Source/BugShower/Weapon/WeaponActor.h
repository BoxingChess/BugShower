// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "WeaponActor.generated.h"

/**
 * 월드에 배치되거나 플레이어가 장착할 수 있는 무기 Actor
 * ItemActor와 유사하지만 무기 전용 기능 포함
 */
UCLASS()
class BUGSHOWER_API AWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	AWeaponActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// ========================================
	// Components
	// ========================================

protected:
	// 무기 메쉬 (Skeletal Mesh for animations)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	// 무기 기능 컴포넌트 (발사, 재장전 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponComponent* WeaponComponent;

	// ========================================
	// Weapon Data
	// ========================================

protected:
	// 무기의 정적 데이터 (메쉬, 데미지, 발사 속도 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	// 현재 탄창에 남은 탄약 (픽업 시 전달될 값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmo;

	// 예비 탄약 (픽업 시 전달될 값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 ReserveAmmo;

	// ========================================
	// Initialization
	// ========================================

public:
	/**
	 * 무기 초기화 (WeaponData와 탄약 설정)
	 * @param InWeaponData - 무기 DataAsset
	 * @param InCurrentAmmo - 탄창 탄약 (0이면 만탄)
	 * @param InReserveAmmo - 예비 탄약
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void InitializeWeapon(UWeaponDataAsset* InWeaponData, int32 InCurrentAmmo = 0, int32 InReserveAmmo = 0);

	// ========================================
	// Attachment (플레이어 손에 부착)
	// ========================================

public:
	/**
	 * 무기를 캐릭터의 손에 부착
	 * @param Character - 부착할 캐릭터
	 * @param SocketName - 부착할 소켓 이름 (기본값: "hand_r_weapon")
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToCharacter(ACharacter* Character, FName SocketName = TEXT("hand_r_weapon"));

	/**
	 * 무기를 캐릭터에서 분리 (땅에 떨어뜨림)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DetachFromCharacter();

	/**
	 * 무기가 장착되어 있는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsEquipped() const { return bIsEquipped; }

protected:
	// 장착 상태 여부
	UPROPERTY()
	bool bIsEquipped;

	// ========================================
	// Getters
	// ========================================

public:
	// WeaponData 가져오기
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UWeaponDataAsset* GetWeaponData() const { return WeaponData; }

	// WeaponComponent 가져오기
	UFUNCTION(BlueprintPure, Category = "Weapon")
	class UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	// 현재 탄약 정보 가져오기
	UFUNCTION(BlueprintPure, Category = "Weapon")
	void GetAmmoInfo(int32& OutCurrentAmmo, int32& OutReserveAmmo) const
	{
		OutCurrentAmmo = CurrentAmmo;
		OutReserveAmmo = ReserveAmmo;
	}

	// WeaponMesh 가져오기
	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
