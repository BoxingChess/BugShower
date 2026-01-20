#pragma once
#include "CoreMinimal.h"

//아이템 타입
UENUM(BlueprintType)
enum class EItemType : uint8
{
    None							UMETA(DisplayName = "아이템 타입이 없음"),
    Consumable						UMETA(DisplayName = "아이템 타입 - 소비"),
    Equipment						UMETA(DisplayName = "아이템 타입 - 장비"),
    Quest							UMETA(DisplayName = "아이템 타입 - 퀘스트"),
	Material						UMETA(DisplayName = "아이템 타입 - 기타")
};

//소비템 관련 Enum
///이제는 AllItem으로 관리하기에 모든 아이템이 고유의 ID값을 갖도록하였음.
UENUM(BlueprintType)
enum class EConsumableID : uint8
{
	//1~100까지는 체력 관련 아이템
	Emergency_Treatment_Kit = 0     UMETA(DisplayName = "응급처치키트"),
	Aid_Kit                         UMETA(DisplayName = "구급상자"),
	Bandages                        UMETA(DisplayName = "붕대"),
	Adrenaline_Syringe              UMETA(DisplayName = "아드레날린 주사기"),
	Painkillers                     UMETA(DisplayName = "진통제"),
	Energy_Drinks                   UMETA(DisplayName = "에너지드링크"),
	Abla_Suppressor                 UMETA(DisplayName = "에블라 억제제"),
	Abla_Purifier                   UMETA(DisplayName = "에블라 정화제"),

	//101~200까지는 투척류 아이템
	Grenades = 101					UMETA(DisplayName = "수류탄"),
	Smoke_Grenades                  UMETA(DisplayName = "연막탄"),
	Flash_Grenades                  UMETA(DisplayName = "섬광탄"),

	//201~255까지는 총알류
	Ammunition_9mm = 201			UMETA(DisplayName = "9mm 탄약"),
	Ammunition_5_56mm               UMETA(DisplayName = "5.56mm 탄약"),
	Ammunition_7_62mm               UMETA(DisplayName = "7.62mm 탄약"),
};

//장비템 관련 Enum
UENUM(BlueprintType)
enum class EEquipmentID : uint8
{
	//0 ~ 100 원거리 무기
	M416 = 0						UMETA(DisplayName = "총 - M416"), 
	AK47							UMETA(DisplayName = "총 - AK47"), 
	AWM								UMETA(DisplayName = "총 - AWM"),

	//101~200 근접 무기
	Knife = 101						UMETA(DisplayName = "근접무기 - 칼"), 
	///TODO : 이후 더 추가 예정. 
};
