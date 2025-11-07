// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/InventoryComponent.h"
#include "Item/ItemActor.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{

}

/*
AddItem함수를 조금 더 다듬을 필요가 있다. MaxStack처리도 아직 되지 않았고 Sort함수도 멤버 함수로 분리하는게 가독성이 더 좋아보인다.
또한 이후 Bool값을 return하도록 바꾸면 좋을듯 하다.
*/
void UInventoryComponent::AddItem(AItemActor* DroppedActor)
{
	//유효하지 않은 액터가 들어왔을 경우 종료
	if (!DroppedActor) return;

	//주울 액터에서 동적 정보와 정적 정보 가져오기
	FBS_Item& DropItem = DroppedActor->GetItemData();
	const UBSStaticItemDataAsset* DropStatic = DroppedActor->GetItemStaticData();
	//정적 정보가 없으면 return;
	if (!DropStatic) return;

	// 장비 아이템-------------------------------------------------------------------------------
	if (DropItem.ItemType == EItemType::Equipment)
	{
		if (EquipmentItem)
		{
			//DiscardItem(EquipmentItem.Get());  // 기존 장비 버리기 TODO: 함수 바꾸어야함.
		}

		//새 장비로 교체함
		EquipmentItem = DroppedActor;
		return;
	}

	// 소비 아이템--------------------------------------------------------------------------------
	if (DropItem.ItemType == EItemType::Consumable)
	{
		//인벤토리에 같은 ItemID를 가진 오브젝트가 있는지 찾는다.
		UBSItemInstance* FoundInst = nullptr;

		if (TObjectPtr<UBSItemInstance>* FoundSlot = ItemInventory.FindByPredicate(
			[&](const TObjectPtr<UBSItemInstance>& Elem)
			{
				return Elem && Elem->StaticData == DropStatic; // 같은 자산이면 같은 아이템
			}))
		{
			FoundInst = FoundSlot->Get();
		}

		//현재 들수있는 무게와 드랍된 아이템의 총 무게를 계산한다.
		const int32 AvailableWeight = MaxWeight - CurrentWeight;
		const int32 DropWeight = DropItem.Quantity * DropStatic->Weight;

		//만약 액터를 찾았을 경우 
		if (FoundInst)
		{
			//해당 아이템의 동적 / 정적 정보 재획득
			FBS_Item& FoundItem = FoundInst->GetItemData();
			const UBSStaticItemDataAsset* FoundStatic = FoundInst->GetItemStaticData();

			//이때 FFL_StaticItem가 없으면 리턴..
			if (!FoundStatic) return;

			// 현재 스택 가능한 최대치 계산
			const int32 AvailableStack = FoundStatic->MaxStackSize - FoundItem.Quantity;

			//드랍 수량, 남은 스택 수량, 무게 한도 내에서 가능한 최소 수량 계산
			const int32 AddableQuantity = FMath::Min3
			(
				DropItem.Quantity,							//드랍된 수량
				AvailableStack,								//스택 가능한 남은 공간(이미 995개 있는데 999개까지만 들수있다면? -> 4개 더 넣을수 있음)
				AvailableWeight / FoundStatic->Weight		//무게 한도(남은 무게가 20이고, 아이템 무게가 5개라면 -> 4개)
			);

			//드랍 수량, 남은 스택 수량, 무게 한도 내에서 가능한 최소 수량 계산
			if (AddableQuantity > 0)
			{
				//수량을 누적하고, 무게 추가, 드랍 아이템 수량 감소
				FoundItem.Quantity += AddableQuantity;
				CurrentWeight += AddableQuantity * FoundStatic->Weight;
				DropItem.Quantity -= AddableQuantity;
			}

		}

		//기존에 같은 ID의 아이템이 인벤토리에 없을 경우 
		else
		{
			// 무게 제한을 고려해 들 수 있는 최대 수량 계산
			int32 AddableQuantity = FMath::Min
			(
				DropItem.Quantity,
				AvailableWeight / DropStatic->Weight
			);

			// 일정 수량이라도 들 수 있다면
			if (AddableQuantity > 0)
			{
				//새 오브젝트 생성
				UBSItemInstance* NewInst = NewObject<UBSItemInstance>(this);

				// 드랍 아이템의 동적 정보 복사 + 수량은 AddableQuantity로 설정
				FBS_Item NewDyn = DropItem;
				NewDyn.Quantity = AddableQuantity;

				// 정적/동적 정보 세팅
				NewInst->StaticData = DropStatic;
				NewInst->Dynamic = NewDyn;

				//인벤토리에 추가
				ItemInventory.Add(NewInst);

				//무게 추가
				CurrentWeight += AddableQuantity * DropStatic->Weight;

				// 원본 액터(DroppedActor)의 수량을 감소시킴 → 바닥에 남은 수량 유지
				DropItem.Quantity -= AddableQuantity;
			}

			ItemInventory.Sort([](const UBSItemInstance& A, const UBSItemInstance& B)
				{
					const auto* SA = A.GetItemStaticData();
					const auto* SB = B.GetItemStaticData();
					const int32 IDA = SA ? SA->ItemID : A.Dynamic.ItemID;
					const int32 IDB = SB ? SB->ItemID : B.Dynamic.ItemID;
					return IDA < IDB;
				});
		}
	}
	for (auto& e : ItemInventory)
	{
		if (e)
		{
			int32 ItemIDValue = e->Dynamic.ItemID;
			FString ItemName = StaticEnum<EConsumableID>()->GetNameStringByValue(ItemIDValue);
			UE_LOG(LogTemp, Error, TEXT("Name : %s, Quntity : %d"), *ItemName, e->Dynamic.Quantity);
		}
	}
}

void UInventoryComponent::DiscardItem(UBSItemInstance* DroppedItem, int32 Count /*= 1*/)
{
	if (!DroppedItem || !GetOwner()) return;

	// 드랍 수량이 유효한지 체크
	if (Count <= 0 || DroppedItem->Dynamic.Quantity < Count) return;

	// 현재 소유자 액터의 위치
	const FVector ActorLocation = GetOwner()->GetActorLocation();

	// 위로 50, 아래로 500 라인트레이스
	FVector Start = ActorLocation + FVector(0, 0, 50);
	FVector End = ActorLocation - FVector(0, 0, 500);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	// 땅을 맞췄으면 그 위치, 아니면 캐릭터 발밑
	FVector SpawnLocation = bHit ? HitResult.ImpactPoint : ActorLocation;

	// 스폰 파라미터
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// DataAsset에서 지정된 아이템 액터 클래스 사용
	if (!DroppedItem->StaticData)
	{
		UE_LOG(LogTemp, Log, TEXT("5"));

		return;
	}
	TSubclassOf<AItemActor> ActorClass = DroppedItem->StaticData->GetItemActorClass();
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Log, TEXT("6"));
		return;
	}

	AItemActor* SpawnedActor = GetWorld()->SpawnActor<AItemActor>(
		ActorClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (SpawnedActor)
	{
		UE_LOG(LogTemp, Log, TEXT("2"));

		// 새로운 액터에 데이터 세팅 (버리는 수량만큼)
		FBS_Item NewItemData = DroppedItem->Dynamic;
		NewItemData.Quantity = Count;

		SpawnedActor->InitializeItemBS_Item(NewItemData);
		SpawnedActor->SetOwner(nullptr);
	}

	// 인벤토리 내부 수량 차감
	DroppedItem->Dynamic.Quantity -= Count;

	// 만약 수량이 0 이하가 되면 인벤토리에서 제거
	if (DroppedItem->Dynamic.Quantity <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("3"));

		ItemInventory.Remove(DroppedItem);
		DroppedItem->MarkAsGarbage(); // GC 대상 처리
	}
}
