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
	//소비템의 인벤토리 입니다.
	UPROPERTY()
	TArray<TObjectPtr<UBSItemInstance>> ItemInventory;

	//장비템 - 총 / 칼 등 
	///TODO : 이후 근접무기는 따로 빼서 원거리 무기랑은 별개로 들고다닐수 있게 한다. 지금은 무기 교체 애니메이션이 없다. 
	UPROPERTY()
	TObjectPtr<AItemActor> EquipmentItem = nullptr;

	//인벤토리내 아이템들이 가질수 있는 최대 무게
	int32 MaxWeight = 500;

	//현재 들고있는 무게
	int32 CurrentWeight = 0;
public:
	//아이템을 인벤토리 or 장비템에 추가한다.
	void AddItem(AItemActor* DroppedActor);

	//인벤토리 or 장비템을 버린다.
	void DiscardItem(UBSItemInstance* DroppedActor, int32 Count = 1);
public:
	TArray<UBSItemInstance*> GetItemInventory() { return ItemInventory; };

};
