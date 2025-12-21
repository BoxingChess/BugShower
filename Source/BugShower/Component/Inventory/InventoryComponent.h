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
<<<<<<< HEAD
	//�Һ����� �κ��丮 �Դϴ�.
	UPROPERTY()
	TArray<TObjectPtr<UBSItemInstance>> ItemInventory;

	//����� - �� / Į �� 
	///TODO : ���� ��������� ���� ���� ���Ÿ� ������� ������ ����ٴҼ� �ְ� �Ѵ�. ������ ���� ��ü �ִϸ��̼��� ����. 
	UPROPERTY()
	TObjectPtr<AItemActor> EquipmentItem = nullptr;

	//�κ��丮�� �����۵��� ������ �ִ� �ִ� ����
	int32 MaxWeight = 500;

	//���� ����ִ� ����
	int32 CurrentWeight = 0;
public:
	//�������� �κ��丮 or ����ۿ� �߰��Ѵ�.
	void AddItem(AItemActor* DroppedActor);

	//�κ��丮 or ������� ������.
	void DiscardItem(UBSItemInstance* DroppedActor, int32 Count = 1);

	/**
	 * UBSItemInstance를 직접 인벤토리에 추가
	 * ItemActor 없이 바로 추가 (로비 선택 아이템용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemInstance(UBSItemInstance* ItemInstance);

=======
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

	// Discard item from inventory by index (Internal function - Do not call from client)
	void DiscardItemByIndex(int32 ItemIndex, int32 Count = 1);

	// [Server RPC] Discard item from inventory by index. Automatically synced to all clients
	UFUNCTION(Server, Reliable)
	void ServerDiscardItem(int32 ItemIndex, int32 Count = 1);

	// Find item index in inventory
	int32 FindItemIndex(UBSItemInstance* ItemInstance) const;
>>>>>>> development
public:
	TArray<UBSItemInstance*> GetItemInventory() { return ItemInventory; };

	// Delegate broadcast when inventory changes
	UPROPERTY(BlueprintAssignable)
	FOnInventoryChanged OnInventoryChanged;
};
