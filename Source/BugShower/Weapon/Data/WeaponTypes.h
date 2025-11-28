// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

/**
 * 무기 종류
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rifle       UMETA(DisplayName = "Assault Rifle"),    // 돌격소총 (M416, AK47)
	Sniper      UMETA(DisplayName = "Sniper Rifle"),     // 저격소총 (AWM)
	Shotgun     UMETA(DisplayName = "Shotgun"),          // 산탄총 (여러 발 동시 발사)
	Pistol      UMETA(DisplayName = "Pistol"),           // 권총
	SMG         UMETA(DisplayName = "Submachine Gun")    // 기관단총
};

/**
 * 발사 모드
 */
UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single      UMETA(DisplayName = "Single Shot"),      // 단발 (클릭당 1발)
	Auto        UMETA(DisplayName = "Full Auto"),        // 연발 (누르고 있으면 계속 발사)
	Burst       UMETA(DisplayName = "3-Round Burst")     // 점사 (클릭당 3발 연속)
};

/**
 * 탄약 타입
 */
UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Ammo_556mm  UMETA(DisplayName = "5.56mm"),           // M416, M16
	Ammo_762mm  UMETA(DisplayName = "7.62mm"),           // AK47, AWM
	Ammo_9mm    UMETA(DisplayName = "9mm"),              // 권총, 기관단총
	Ammo_12Gauge UMETA(DisplayName = "12 Gauge"),        // 산탄총
	Ammo_45ACP  UMETA(DisplayName = ".45 ACP")           // 권총
};

/**
 * 발사 시작점 (총알이 어디서 나가는지)
 */
UENUM(BlueprintType)
enum class EFireOrigin : uint8
{
	Camera      UMETA(DisplayName = "Camera Center"),    // 카메라 중심 (FPS 정확도 높음)
	Muzzle      UMETA(DisplayName = "Weapon Muzzle")     // 총구 끝 (TPS 시각적으로 정확)
};
