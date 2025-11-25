// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ItemDropStructs.generated.h"

/**
 * Single item drop entry with weight and conditions
 */
USTRUCT(BlueprintType)
struct FItemDropEntry
{
	GENERATED_BODY()

	// Item class to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TSubclassOf<class AItemBase> ItemClass;

	// Drop weight for weighted random selection (higher = more likely)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropWeight = 1.0f;

	// Minimum number of items to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MinCount = 1;

	// Maximum number of items to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MaxCount = 1;

	// Required gameplay tags for this drop (quest, event, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Condition")
	FGameplayTagContainer RequiredTags;

	// If true, this item always drops (ignores drop chance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Condition")
	bool bIsGuaranteedDrop = false;
};

/**
 * Drop condition for conditional drops
 */
USTRUCT(BlueprintType)
struct FDropCondition
{
	GENERATED_BODY()

	// Gameplay tag query for complex conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FGameplayTagQuery ConditionQuery;

	// Required quest IDs (if any quest is active, condition is met)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TArray<FName> RequiredQuestIDs;

	// Minimum player level required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	int32 MinPlayerLevel = 1;

	// Check if condition is met
	bool IsMet(const FGameplayTagContainer& ActiveTags, int32 PlayerLevel) const
	{
		// Check level
		if (PlayerLevel < MinPlayerLevel)
			return false;

		// Check tag query
		if (!ConditionQuery.IsEmpty() && !ConditionQuery.Matches(ActiveTags))
			return false;

		return true;
	}
};

/**
 * Monster drop configuration - DataTable row
 * Contains all drop information for a specific monster type
 */
USTRUCT(BlueprintType)
struct FMonsterDropConfig : public FTableRowBase
{
	GENERATED_BODY()

	// ========== Layer 1: Common Drops (Always checked) ==========
	// Items that always have a chance to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Common")
	TArray<FItemDropEntry> CommonDrops;

	// ========== Layer 2: Rare/Unique Drops ==========
	// Special items unique to this monster
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Unique")
	TArray<FItemDropEntry> UniqueDrops;

	// ========== Layer 3: Guaranteed Drops ==========
	// Items that always drop (ignores drop chance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Guaranteed")
	TArray<FItemDropEntry> GuaranteedDrops;

	// ========== Drop Settings ==========
	// Base chance for any drop to occur (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Settings")
	float BaseDropChance = 0.5f;

	// Minimum total items to drop (if any drop occurs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Settings")
	int32 MinTotalDropCount = 1;

	// Maximum total items to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Settings")
	int32 MaxTotalDropCount = 3;

	// Spread radius for dropped items
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Settings")
	float DropSpreadRadius = 100.0f;
};
