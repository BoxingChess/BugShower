#pragma once
#include "CoreMinimal.h"

///ĳ������ ���� Ÿ�Կ� ���� Enum���� ��Ƶ� ���.

//���� ��ȯ�� ���� ĳ���� ���� ENUM
UENUM(BlueprintType)
enum class ECameraViewMode : uint8
{
    ThirdPerson,
    FirstPerson
};


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),     //����
    Armed       UMETA(DisplayName = "Armed"),       //���⸦ �������
    Reloading   UMETA(DisplayName = "Reloading")    //��������
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Gun         UMETA(DisplayName = "Gun"),         //��
    Melee       UMETA(DisplayName = "Melee"),       //��������
    Throwing    UMETA(DisplayName = "Throwing")      //��ô����
};


UENUM(BlueprintType)
enum class ECharacterStance : uint8
{
    Standing,               //���ִ� �⺻ �ڼ�
    Crouching,              //��ũ�� �ڼ�(����, ���� ��ֹ� �����)
    Prone                   //���帰 �ڼ�
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    Normal         UMETA(DisplayName = "Normal"),       // ����
    InVehicle      UMETA(DisplayName = "In Vehicle"),   // ���� ž�� ��
    Stunned        UMETA(DisplayName = "Stunned"),      // ���� ����
    Dead           UMETA(DisplayName = "Dead")          // ���
};

UENUM(BlueprintType)
enum class EUIState : uint8
{
    OpenInventory       UMETA(DisplayName = "Open Inventory"),
    CloseInventory      UMETA(DisplayName = "Close Inventory")
};