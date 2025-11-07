#pragma once

#include "CoreMinimal.h"
#include "PoolingType.generated.h"

UENUM()
enum class EPoolType : uint8
{
	Monster UMETA(DisplayName = "Monster"),
	Item UMETA(DisplayName = "Item"),
	Bullet UMETA(DisplayName = "Bullet"),
};