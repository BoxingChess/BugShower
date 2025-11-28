// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/InventoryComponent.h"
#include "Item/ItemActor.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{

}

/*
AddItem�Լ��� ���� �� �ٵ��� �ʿ䰡 �ִ�. MaxStackó���� ���� ���� �ʾҰ� Sort�Լ��� ��� �Լ��� �и��ϴ°� �������� �� ���ƺ��δ�.
���� ���� Bool���� return�ϵ��� �ٲٸ� ������ �ϴ�.
*/
void UInventoryComponent::AddItem(AItemActor* DroppedActor)
{
	//��ȿ���� ���� ���Ͱ� ������ ��� ����
	if (!DroppedActor) return;

	//�ֿ� ���Ϳ��� ���� ������ ���� ���� ��������
	FBS_Item& DropItem = DroppedActor->GetItemData();
	const UBSStaticItemDataAsset* DropStatic = DroppedActor->GetItemStaticData();
	//���� ������ ������ return;
	if (!DropStatic) return;

	// 장비 아이템 처리 (무기, 방어구 등)-------------------------------------------------------------------------------
	if (DropItem.ItemType == EItemType::Equipment)
	{
		// 기존 장비가 있으면 바닥에 드롭 (수정: 아이템 손실 버그 해결)
		if (EquipmentItem)
		{
			// 플레이어의 현재 위치에서 바닥을 찾아 장비를 떨어뜨림
			FVector DropLocation = GetOwner()->GetActorLocation();

			// Line Trace로 플레이어 발 밑 바닥 위치 찾기
			FVector Start = DropLocation + FVector(0, 0, 50);   // 플레이어 위치에서 약간 위
			FVector End = DropLocation - FVector(0, 0, 500);     // 아래로 500 유닛

			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(GetOwner());  // 플레이어 자신은 무시

			// 바닥 충돌 체크
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult, Start, End, ECC_Visibility, Params);

			// 바닥을 찾았으면 그 위치에, 못 찾았으면 플레이어 위치에 배치
			DropLocation = bHit ? HitResult.ImpactPoint : DropLocation;
			DropLocation += FVector(0, 100, 0); // 플레이어 앞쪽으로 100 유닛 이동

			// 기존 장비를 월드에 배치하고 주인 해제 (다시 주울 수 있게 함)
			EquipmentItem->SetActorLocation(DropLocation);
			EquipmentItem->SetOwner(nullptr);
		}

		// 새 장비로 교체
		EquipmentItem = DroppedActor;
		return;
	}

	// �Һ� ������--------------------------------------------------------------------------------
	if (DropItem.ItemType == EItemType::Consumable)
	{
		//�κ��丮�� ���� ItemID�� ���� ������Ʈ�� �ִ��� ã�´�.
		UBSItemInstance* FoundInst = nullptr;

		if (TObjectPtr<UBSItemInstance>* FoundSlot = ItemInventory.FindByPredicate(
			[&](const TObjectPtr<UBSItemInstance>& Elem)
			{
				return Elem && Elem->StaticData == DropStatic; // ���� �ڻ��̸� ���� ������
			}))
		{
			FoundInst = FoundSlot->Get();
		}

		//���� ����ִ� ���Կ� ����� �������� �� ���Ը� ����Ѵ�.
		const int32 AvailableWeight = MaxWeight - CurrentWeight;
		const int32 DropWeight = DropItem.Quantity * DropStatic->Weight;

		//���� ���͸� ã���� ��� 
		if (FoundInst)
		{
			//�ش� �������� ���� / ���� ���� ��ȹ��
			FBS_Item& FoundItem = FoundInst->GetItemData();
			const UBSStaticItemDataAsset* FoundStatic = FoundInst->GetItemStaticData();

			//�̶� FFL_StaticItem�� ������ ����..
			if (!FoundStatic) return;

			// ���� ���� ������ �ִ�ġ ���
			const int32 AvailableStack = FoundStatic->MaxStackSize - FoundItem.Quantity;

			//��� ����, ���� ���� ����, ���� �ѵ� ������ ������ �ּ� ���� ���
			const int32 AddableQuantity = FMath::Min3
			(
				DropItem.Quantity,							//����� ����
				AvailableStack,								//���� ������ ���� ����(�̹� 995�� �ִµ� 999�������� ����ִٸ�? -> 4�� �� ������ ����)
				AvailableWeight / FoundStatic->Weight		//���� �ѵ�(���� ���԰� 20�̰�, ������ ���԰� 5����� -> 4��)
			);

			//��� ����, ���� ���� ����, ���� �ѵ� ������ ������ �ּ� ���� ���
			if (AddableQuantity > 0)
			{
				//������ �����ϰ�, ���� �߰�, ��� ������ ���� ����
				FoundItem.Quantity += AddableQuantity;
				CurrentWeight += AddableQuantity * FoundStatic->Weight;
				DropItem.Quantity -= AddableQuantity;
			}

		}

		//������ ���� ID�� �������� �κ��丮�� ���� ��� 
		else
		{
			// ���� ������ ������ �� �� �ִ� �ִ� ���� ���
			int32 AddableQuantity = FMath::Min
			(
				DropItem.Quantity,
				AvailableWeight / DropStatic->Weight
			);

			// ���� �����̶� �� �� �ִٸ�
			if (AddableQuantity > 0)
			{
				//�� ������Ʈ ����
				UBSItemInstance* NewInst = NewObject<UBSItemInstance>(this);

				// ��� �������� ���� ���� ���� + ������ AddableQuantity�� ����
				FBS_Item NewDyn = DropItem;
				NewDyn.Quantity = AddableQuantity;

				// ����/���� ���� ����
				NewInst->StaticData = DropStatic;
				NewInst->Dynamic = NewDyn;

				//�κ��丮�� �߰�
				ItemInventory.Add(NewInst);

				//���� �߰�
				CurrentWeight += AddableQuantity * DropStatic->Weight;

				// ���� ����(DroppedActor)�� ������ ���ҽ�Ŵ �� �ٴڿ� ���� ���� ����
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

	// 디버그: 인벤토리 내용 출력 (개발 중에만 사용)
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// for (auto& e : ItemInventory)
	// {
	// 	if (e)
	// 	{
	// 		int32 ItemIDValue = e->Dynamic.ItemID;
	// 		FString ItemName = StaticEnum<EConsumableID>()->GetNameStringByValue(ItemIDValue);
	// 		UE_LOG(LogTemp, Log, TEXT("Inventory Item - Name: %s, Quantity: %d"), *ItemName, e->Dynamic.Quantity);
	// 	}
	// }
#endif
}

void UInventoryComponent::DiscardItem(UBSItemInstance* DroppedItem, int32 Count /*= 1*/)
{
	if (!DroppedItem || !GetOwner()) return;

	// ��� ������ ��ȿ���� üũ
	if (Count <= 0 || DroppedItem->Dynamic.Quantity < Count) return;

	// ���� ������ ������ ��ġ
	const FVector ActorLocation = GetOwner()->GetActorLocation();

	// ���� 50, �Ʒ��� 500 ����Ʈ���̽�
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

	// ���� �������� �� ��ġ, �ƴϸ� ĳ���� �߹�
	FVector SpawnLocation = bHit ? HitResult.ImpactPoint : ActorLocation;

	// ���� �Ķ����
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// DataAsset���� ������ ������ ���� Ŭ���� ���
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

		// ���ο� ���Ϳ� ������ ���� (������ ������ŭ)
		FBS_Item NewItemData = DroppedItem->Dynamic;
		NewItemData.Quantity = Count;

		SpawnedActor->InitializeItemBS_Item(NewItemData);
		SpawnedActor->SetOwner(nullptr);
	}

	// �κ��丮 ���� ���� ����
	DroppedItem->Dynamic.Quantity -= Count;

	// ���� ������ 0 ���ϰ� �Ǹ� �κ��丮���� ����
	if (DroppedItem->Dynamic.Quantity <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("3"));

		ItemInventory.Remove(DroppedItem);
		DroppedItem->MarkAsGarbage(); // GC ��� ó��
	}
}
