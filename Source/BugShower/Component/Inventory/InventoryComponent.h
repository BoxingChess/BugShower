// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Item/BSItemInstance.h"

#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUGSHOWER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();
private:
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

public:
	TArray<UBSItemInstance*> GetItemInventory() { return ItemInventory; };

};
