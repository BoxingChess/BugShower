// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Item/BSItemInstance.h"

#include "InventoryComponent.generated.h"

// 인벤토리 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUGSHOWER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

	// Replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Consumable item inventory (Replicated to all clients)
	UPROPERTY(ReplicatedUsing = OnRep_ItemInventory)
	TArray<TObjectPtr<UBSItemInstance>> ItemInventory;

	// Called when ItemInventory is replicated to client
	UFUNCTION()
	void OnRep_ItemInventory();

	// Equipment slot - Weapon / Tool
	// TODO: Implement equipment system with mesh attachment and animations
	UPROPERTY()
	TObjectPtr<AItemActor> EquipmentItem = nullptr;

	// Maximum weight capacity
	int32 MaxWeight = 500;

	// Current weight
	int32 CurrentWeight = 0;
public:
	// Add item to inventory or equipment slot
	void AddItem(AItemActor* DroppedActor);

	/**
	 * Add UBSItemInstance directly to inventory (for loading from save or selected items)
	 * Server-only function
	 *
	 * @param ItemInstances - Array of item instances to add
	 */
	void AddItemInstances(const TArray<UBSItemInstance*>& ItemInstances);

	// Discard item from inventory by index (Internal function - Do not call from client)
	void DiscardItemByIndex(int32 ItemIndex, int32 Count = 1);

	// [Server RPC] Discard item from inventory by index. Automatically synced to all clients
	UFUNCTION(Server, Reliable)
	void ServerDiscardItem(int32 ItemIndex, int32 Count = 1);

	// Find item index in inventory
	int32 FindItemIndex(UBSItemInstance* ItemInstance) const;

	/**
	 * 특정 아이템 ID의 총 개수 조회 (탄약용)
	 * @param ItemID 아이템 ID (201=9mm, 202=5.56mm, 203=7.62mm 등)
	 * @return 인벤토리에 있는 해당 아이템의 총 개수
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCountByID(uint8 ItemID) const;

	/**
	 * 인벤토리에서 아이템 소비 (탄약용)
	 * @param ItemID 아이템 ID
	 * @param Amount 소비할 개수
	 * @return 실제로 소비된 개수 (인벤토리에 부족하면 가능한 만큼만)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 ConsumeItemByID(uint8 ItemID, int32 Amount);

public:
	TArray<UBSItemInstance*> GetItemInventory() { return ItemInventory; };

	// Delegate broadcast when inventory changes
	UPROPERTY(BlueprintAssignable)
	FOnInventoryChanged OnInventoryChanged;
};
