// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/InventoryComponent.h"
#include "Item/ItemActor.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemEnum.h"
#include "Game/BSGameInstance.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Enable replication for this component
	SetIsReplicatedByDefault(true);
}

// ========================================
// Replication Setup
// ========================================
void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate ReplicatedItemData (FBS_Item 구조체 배열) to all clients
	DOREPLIFETIME(UInventoryComponent, ReplicatedItemData);
}

// ========================================
// 서버: ItemInventory → ReplicatedItemData 동기화
// ========================================
void UInventoryComponent::SyncReplicatedData()
{
	// 서버에서만 실행
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// ReplicatedItemData 초기화
	ReplicatedItemData.Empty();
	ReplicatedItemData.Reserve(ItemInventory.Num());

	// ItemInventory → ReplicatedItemData 변환
	for (const TObjectPtr<UBSItemInstance>& ItemInstance : ItemInventory)
	{
		if (ItemInstance)
		{
			// CRITICAL FIX: Dynamic과 StaticData의 ItemID 불일치 수정
			if (ItemInstance->StaticData && ItemInstance->Dynamic.ItemID != ItemInstance->StaticData->ItemID)
			{
				UE_LOG(LogTemp, Error, TEXT("[Server] SyncReplicatedData - ⚠️ ID MISMATCH in inventory! Dynamic.ID=%d, Static.ID=%d, Name=%s - FIXING!"),
					ItemInstance->Dynamic.ItemID, ItemInstance->StaticData->ItemID, *ItemInstance->StaticData->DisplayName.ToString());

				// StaticData의 ID로 수정
				ItemInstance->Dynamic.ItemID = ItemInstance->StaticData->ItemID;
				ItemInstance->Dynamic.ItemType = ItemInstance->StaticData->ItemType;
			}

			ReplicatedItemData.Add(ItemInstance->Dynamic);

			// 디버그: 복제되는 데이터 확인
			UE_LOG(LogTemp, Warning, TEXT("[Server] SyncReplicatedData - Adding Item: Type=%d, ID=%d, Qty=%d, StaticName=%s"),
				static_cast<int32>(ItemInstance->Dynamic.ItemType),
				ItemInstance->Dynamic.ItemID,
				ItemInstance->Dynamic.Quantity,
				ItemInstance->StaticData ? *ItemInstance->StaticData->DisplayName.ToString() : TEXT("NULL"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Server] SyncReplicatedData - Synced %d items to ReplicatedItemData"), ReplicatedItemData.Num());
}

// ========================================
// 클라이언트: ReplicatedItemData → ItemInventory 재구성
// ========================================
void UInventoryComponent::OnRep_ReplicatedItemData()
{
	UE_LOG(LogTemp, Log, TEXT("[Client] OnRep_ReplicatedItemData called - Received %d items from server"), ReplicatedItemData.Num());

	// 기존 ItemInventory 초기화
	ItemInventory.Empty();
	ItemInventory.Reserve(ReplicatedItemData.Num());

	// GameInstance에서 ItemResourceManager 가져오기
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UBSGameInstance* BSGameInstance = Cast<UBSGameInstance>(GameInstance);
	if (!BSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[Client] OnRep_ReplicatedItemData - Failed to get BSGameInstance!"));
		return;
	}

	UItemResourceManager* ItemResourceManager = BSGameInstance->GetSubsystem<UItemResourceManager>();
	if (!ItemResourceManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[Client] OnRep_ReplicatedItemData - Failed to get ItemResourceManager!"));
		return;
	}

	// ReplicatedItemData → ItemInventory 변환
	for (const FBS_Item& ItemData : ReplicatedItemData)
	{
		// 디버그: 수신된 데이터 확인
		UE_LOG(LogTemp, Warning, TEXT("[Client] OnRep - Received Item: Type=%d, ID=%d, Qty=%d"),
			static_cast<int32>(ItemData.ItemType), ItemData.ItemID, ItemData.Quantity);

		// StaticData 가져오기 (ItemType과 ItemID 사용)
		const UBSStaticItemDataAsset* StaticData = ItemResourceManager->GetStaticItem(ItemData.ItemType, ItemData.ItemID);
		if (!StaticData)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Client] OnRep_ReplicatedItemData - Failed to find StaticData for ItemType: %d, ItemID: %d"),
				static_cast<int32>(ItemData.ItemType), ItemData.ItemID);
			continue;
		}

		// 디버그: 찾은 StaticData 확인
		UE_LOG(LogTemp, Warning, TEXT("[Client] OnRep - Found StaticData: Name=%s, StaticID=%d"),
			*StaticData->DisplayName.ToString(), StaticData->ItemID);

		// UBSItemInstance 생성
		UBSItemInstance* NewInstance = NewObject<UBSItemInstance>(this);
		NewInstance->StaticData = StaticData;
		NewInstance->Dynamic = ItemData;

		ItemInventory.Add(NewInstance);
	}

	UE_LOG(LogTemp, Log, TEXT("[Client] OnRep_ReplicatedItemData - Reconstructed %d ItemInstances"), ItemInventory.Num());

	// UI 업데이트
	OnInventoryChanged.Broadcast();
}

/*
AddItem Function Notes:
- TODO: Implement MaxStack handling
- TODO: Separate Sort functionality
- TODO: Consider returning bool for success/failure
*/
void UInventoryComponent::AddItem(AItemActor* DroppedActor)
{
	// DEBUG: Log function call with actor pointer
	UE_LOG(LogTemp, Warning, TEXT("[AddItem] 🟣 Called with Actor: %p"), DroppedActor);

	// Validate input
	if (!DroppedActor) return;

	// 데디서버 환경: 서버에서만 인벤토리 수정 가능
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::AddItem - Called on client! Ignoring. Use server authority."));
		return;
	}

	// Get item data (dynamic) and static data asset
	FBS_Item& DropItem = DroppedActor->GetItemData();
	const UBSStaticItemDataAsset* DropStatic = DroppedActor->GetItemStaticData();

	// Validate static data
	if (!DropStatic)
	{
		UE_LOG(LogTemp, Error, TEXT("AddItem Failed: DropStatic is null!"));
		return;
	}

	// ========================================
	// CRITICAL FIX: Dynamic.ItemID와 StaticData->ItemID 불일치 수정
	// ItemActor의 ItemInformation.ItemID가 StaticItemInfo->ItemID와 다를 경우 동기화
	// ========================================
	if (DropItem.ItemID != DropStatic->ItemID)
	{
		UE_LOG(LogTemp, Error, TEXT("[AddItem] ⚠️ ID MISMATCH! Dynamic.ItemID=%d, StaticData->ItemID=%d, StaticName=%s - FIXING!"),
			DropItem.ItemID, DropStatic->ItemID, *DropStatic->DisplayName.ToString());

		// StaticData의 ItemID가 정확한 값이므로 Dynamic을 수정
		DropItem.ItemID = DropStatic->ItemID;
		DropItem.ItemType = DropStatic->ItemType;
	}

	// Debug: Log item info
	UE_LOG(LogTemp, Log, TEXT("[AddItem Start] ItemID: %d, Quantity: %d, Weight: %d, Type: %d"),
		DropItem.ItemID, DropItem.Quantity, DropStatic->Weight, (int32)DropItem.ItemType);

	// DEBUG: Check if quantity is 0
	if (DropItem.Quantity <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[AddItem] ❌ Quantity is 0! Item not initialized properly. Ignoring."));
		return;
	}

	// ========================================
	// Equipment Item Handling (Weapon, Armor, etc.)
	// ========================================
	if (DropItem.ItemType == EItemType::Equipment)
	{
		// If equipment slot is occupied, drop existing equipment
		if (EquipmentItem)
		{
			// Find ground position below player and drop equipment
			FVector DropLocation = GetOwner()->GetActorLocation();

			// Line trace to find ground position below player
			FVector Start = DropLocation + FVector(0, 0, 50);   // Slightly above player
			FVector End = DropLocation - FVector(0, 0, 500);     // 500 units below

			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(GetOwner());  // Ignore player

			// Check ground collision
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult, Start, End, ECC_Visibility, Params);

			// Use hit point if ground found, otherwise use player location
			DropLocation = bHit ? HitResult.ImpactPoint : DropLocation;
			DropLocation += FVector(0, 100, 0); // Move 100 units forward

			// Place existing equipment in world and clear owner (allow re-pickup)
			EquipmentItem->SetActorLocation(DropLocation);
			EquipmentItem->SetOwner(nullptr);
		}

		// Equip new item
		EquipmentItem = DroppedActor;

		// Log equipment acquisition
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] Equipment acquired! ItemID: %d (Equipped in slot)"),
			DropItem.ItemID);

		// Equipment always takes full item, so remove from world
		// Note: Equipment items don't use pooling in current design
		// They are kept as actors and attached to player

		return;
	}

	// ========================================
	// Consumable & Material Item Handling (스택 가능한 아이템)
	// ========================================
	if (DropItem.ItemType == EItemType::Consumable || DropItem.ItemType == EItemType::Material)
	{
		// Find existing slot with same ItemID
		UBSItemInstance* FoundInst = nullptr;

		if (TObjectPtr<UBSItemInstance>* FoundSlot = ItemInventory.FindByPredicate(
			[&](const TObjectPtr<UBSItemInstance>& Elem)
			{
				return Elem && Elem->StaticData == DropStatic; // Same static data = same item type
			}))
		{
			FoundInst = FoundSlot->Get();
		}

		// Calculate available weight capacity
		const int32 AvailableWeight = MaxWeight - CurrentWeight;
		const int32 DropWeight = DropItem.Quantity * DropStatic->Weight;

		UE_LOG(LogTemp, Log, TEXT("  💼 무게 계산: AvailableWeight=%d, DropWeight=%d, ItemWeight=%d"),
			AvailableWeight, DropWeight, DropStatic->Weight);

		// Case 1: Existing slot found with same ItemID
		if (FoundInst)
		{
			// Get dynamic and static data of found slot
			FBS_Item& FoundItem = FoundInst->GetItemData();
			const UBSStaticItemDataAsset* FoundStatic = FoundInst->GetItemStaticData();

			// Validate static data
			if (!FoundStatic) return;

			// Calculate available stack space
			const int32 AvailableStack = FoundStatic->MaxStackSize - FoundItem.Quantity;

			// Calculate addable quantity (minimum of: drop quantity, stack space, weight capacity)
			const int32 AddableQuantity = FMath::Min3
			(
				DropItem.Quantity,							// Dropped quantity
				AvailableStack,								// Available stack space (e.g., 995 existing + 999 dropped = only 4 can be added)
				AvailableWeight / FoundStatic->Weight		// Weight capacity (e.g., 20 available weight, 5 weight per item = max 4 items)
			);

			UE_LOG(LogTemp, Log, TEXT("  AddableQuantity (existing slot): Drop=%d, Stack=%d, Weight=%d -> Result=%d"),
				DropItem.Quantity, AvailableStack, AvailableWeight / FoundStatic->Weight, AddableQuantity);

			// If can add items
			if (AddableQuantity > 0)
			{
				// Add to existing slot, update weight, decrease dropped quantity
				FoundItem.Quantity += AddableQuantity;
				CurrentWeight += AddableQuantity * FoundStatic->Weight;
				DropItem.Quantity -= AddableQuantity;

				// Success log (added to existing slot)
				UE_LOG(LogTemp, Warning, TEXT("[Pickup Success] Item added! ItemID: %d, Added: %d, Weight: %d/%d"),
					DropItem.ItemID, AddableQuantity, CurrentWeight, MaxWeight);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Pickup Failed] AddableQuantity = 0 (Inventory full or weight exceeded)"));
			}

		}

		// Case 2: No existing slot found - create new slot
		else
		{
			// Calculate addable quantity based on weight capacity
			int32 AddableQuantity = FMath::Min
			(
				DropItem.Quantity,
				AvailableWeight / DropStatic->Weight
			);

			UE_LOG(LogTemp, Log, TEXT("  AddableQuantity (new slot): Drop=%d, Weight=%d -> Result=%d"),
				DropItem.Quantity, AvailableWeight / DropStatic->Weight, AddableQuantity);

			// If can add items
			if (AddableQuantity > 0)
			{
				// Create new instance
				UBSItemInstance* NewInst = NewObject<UBSItemInstance>(this);

				// Copy dynamic data and set quantity to addable amount
				FBS_Item NewDyn = DropItem;
				NewDyn.Quantity = AddableQuantity;

				// Set static/dynamic data
				NewInst->StaticData = DropStatic;
				NewInst->Dynamic = NewDyn;

				// Add to inventory
				ItemInventory.Add(NewInst);

				// Update weight
				CurrentWeight += AddableQuantity * DropStatic->Weight;

				// Decrease dropped actor quantity (or destroy if 0)
				DropItem.Quantity -= AddableQuantity;

				// Success log (new slot created)
				UE_LOG(LogTemp, Warning, TEXT("[Pickup Success] New item added! ItemID: %d, Quantity: %d, Weight: %d/%d"),
					DropItem.ItemID, AddableQuantity, CurrentWeight, MaxWeight);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Pickup Failed] AddableQuantity = 0 (Inventory full or weight exceeded)"));
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

	// ========================================
	// Remove Item from World if Fully Picked Up
	// ========================================
	if (DropItem.Quantity <= 0)
	{
		// All quantity moved to inventory - remove from world
		if (DroppedActor->IsPooled())
		{
			// DEBUG: Log actor pointer when returning to pool
			UE_LOG(LogTemp, Warning, TEXT("[AddItem] 🔴 Returning actor to pool: %p (ItemID=%d)"),
				DroppedActor, DropItem.ItemID);

			// Pooled item → return to pool
			DroppedActor->DeSpawn();
			UE_LOG(LogTemp, Log, TEXT("[AddItem] ✅ Pooled item returned to pool (ItemID=%d)"), DropItem.ItemID);
		}
		else
		{
			// Map-placed item → destroy
			DroppedActor->Destroy();
			UE_LOG(LogTemp, Log, TEXT("[AddItem] ✅ Map-placed item destroyed (ItemID=%d)"), DropItem.ItemID);
		}
	}
	else
	{
		// Partial pickup - item remains in world with reduced quantity
		UE_LOG(LogTemp, Log, TEXT("[AddItem] Partial pickup - %d items remaining in world"), DropItem.Quantity);
	}

	// Debug: Print inventory contents (Development only)
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (auto& e : ItemInventory)
	{
		if (e)
		{
			int32 ItemIDValue = e->Dynamic.ItemID;
			FString ItemName = StaticEnum<EConsumableID>()->GetNameStringByValue(ItemIDValue);
			UE_LOG(LogTemp, Log, TEXT("  [Inventory] Item: %s, Quantity: %d"), *ItemName, e->Dynamic.Quantity);
		}
	}
#endif

	// 서버: ItemInventory → ReplicatedItemData 동기화 (클라이언트로 복제)
	SyncReplicatedData();

	// Broadcast inventory changed event
	OnInventoryChanged.Broadcast();
}

// ========================================
// Add UBSItemInstance Directly (For Loading from Save/Selected Items)
// ========================================
void UInventoryComponent::AddItemInstances(const TArray<UBSItemInstance*>& ItemInstances)
{
	// Server-only function check
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("[AddItemInstances] This function can only be called on server!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[AddItemInstances] Adding %d items from selected items"), ItemInstances.Num());

	for (UBSItemInstance* ItemInstance : ItemInstances)
	{
		if (!ItemInstance || !ItemInstance->StaticData)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AddItemInstances] Invalid item instance or static data, skipping"));
			continue;
		}

		const FBS_Item& ItemData = ItemInstance->Dynamic;
		const UBSStaticItemDataAsset* StaticData = ItemInstance->StaticData;

		// Only handle Consumable items (Equipment is handled differently)
		if (ItemData.ItemType != EItemType::Consumable)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AddItemInstances] Item type is not Consumable (Type=%d), skipping"),
				(int32)ItemData.ItemType);
			continue;
		}

		// Check if same item already exists in inventory
		UBSItemInstance* FoundInst = nullptr;
		for (UBSItemInstance* ExistingItem : ItemInventory)
		{
			if (ExistingItem && ExistingItem->StaticData == StaticData)
			{
				FoundInst = ExistingItem;
				break;
			}
		}

		// Calculate available weight
		const int32 AvailableWeight = MaxWeight - CurrentWeight;
		const int32 ItemWeight = ItemData.Quantity * StaticData->Weight;

		// If same item exists, merge quantities
		if (FoundInst)
		{
			const int32 AvailableStack = StaticData->MaxStackSize - FoundInst->Dynamic.Quantity;
			const int32 AddableQuantity = FMath::Min3(
				ItemData.Quantity,
				AvailableStack,
				AvailableWeight / StaticData->Weight
			);

			if (AddableQuantity > 0)
			{
				FoundInst->Dynamic.Quantity += AddableQuantity;
				CurrentWeight += AddableQuantity * StaticData->Weight;

				UE_LOG(LogTemp, Log, TEXT("[AddItemInstances] Merged item (ID=%d, Added=%d, Total=%d)"),
					ItemData.ItemID, AddableQuantity, FoundInst->Dynamic.Quantity);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[AddItemInstances] Cannot add item (ID=%d) - inventory full or weight limit"),
					ItemData.ItemID);
			}
		}
		// Create new inventory slot
		else
		{
			const int32 AddableQuantity = FMath::Min(
				ItemData.Quantity,
				AvailableWeight / StaticData->Weight
			);

			if (AddableQuantity > 0)
			{
				// Create new instance
				UBSItemInstance* NewInst = NewObject<UBSItemInstance>(this);
				NewInst->StaticData = StaticData;
				NewInst->Dynamic = ItemData;
				NewInst->Dynamic.Quantity = AddableQuantity;

				ItemInventory.Add(NewInst);
				CurrentWeight += AddableQuantity * StaticData->Weight;

				UE_LOG(LogTemp, Log, TEXT("[AddItemInstances] Created new slot (ID=%d, Quantity=%d)"),
					ItemData.ItemID, AddableQuantity);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[AddItemInstances] Cannot add item (ID=%d) - weight limit exceeded"),
					ItemData.ItemID);
			}
		}
	}

	// Sort inventory by ItemID
	ItemInventory.Sort([](const UBSItemInstance& A, const UBSItemInstance& B)
	{
		const auto* SA = A.GetItemStaticData();
		const auto* SB = B.GetItemStaticData();
		const int32 IDA = SA ? SA->ItemID : A.Dynamic.ItemID;
		const int32 IDB = SB ? SB->ItemID : B.Dynamic.ItemID;
		return IDA < IDB;
	});

	UE_LOG(LogTemp, Log, TEXT("[AddItemInstances] Finished adding items. Total inventory: %d items, Weight: %d/%d"),
		ItemInventory.Num(), CurrentWeight, MaxWeight);

	// 서버: ItemInventory → ReplicatedItemData 동기화 (클라이언트로 복제)
	SyncReplicatedData();

	// Broadcast inventory changed event
	OnInventoryChanged.Broadcast();
}

// ========================================
// Find Item Index in Inventory
// ========================================
int32 UInventoryComponent::FindItemIndex(UBSItemInstance* ItemInstance) const
{
	if (!ItemInstance) return INDEX_NONE;

	for (int32 i = 0; i < ItemInventory.Num(); ++i)
	{
		if (ItemInventory[i] == ItemInstance)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

// ========================================
// Discard Item By Index
// ========================================
void UInventoryComponent::DiscardItemByIndex(int32 ItemIndex, int32 Count /*= 1*/)
{
	// 데디서버 환경: 서버에서만 인벤토리 수정 가능
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::DiscardItemByIndex - Called on client! Ignoring. Use ServerDiscardItem RPC."));
		return;
	}

	// Validate index
	if (!ItemInventory.IsValidIndex(ItemIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] Invalid index: %d (Inventory size: %d)"), ItemIndex, ItemInventory.Num());
		return;
	}

	UBSItemInstance* DroppedItem = ItemInventory[ItemIndex];
	if (!DroppedItem)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] Item at index %d is null"), ItemIndex);
		return;
	}

	// Validate quantity
	if (Count <= 0 || DroppedItem->Dynamic.Quantity < Count)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] Invalid count: %d (Item has: %d)"), Count, DroppedItem->Dynamic.Quantity);
		return;
	}

	// Get owner location
	const FVector ActorLocation = GetOwner()->GetActorLocation();

	// Line trace to find ground (50 units up, 500 units down)
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

	// Use hit point if ground found, otherwise use actor location
	// Spawn above ground so item can fall with physics
	FVector SpawnLocation = bHit ? HitResult.ImpactPoint : ActorLocation;
	SpawnLocation.Z += 100.0f;  // Spawn 100 units above ground (let it fall with physics)

	// Add random horizontal offset for variety
	float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
	float RandomDistance = FMath::FRandRange(20.0f, 50.0f);  // Smaller spread (items will roll naturally)
	SpawnLocation.X += FMath::Cos(RandomAngle) * RandomDistance;
	SpawnLocation.Y += FMath::Sin(RandomAngle) * RandomDistance;

	// Get item actor class from DataAsset
	if (!DroppedItem->StaticData)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] StaticData is null - ItemID: %d"),
			DroppedItem->Dynamic.ItemID);
		return;
	}

	TSubclassOf<AItemActor> ActorClass = DroppedItem->StaticData->GetItemActorClass();

	// Fallback: Use default ItemActor class if not set in DataAsset
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DiscardItemByIndex] ActorClass not set in DataAsset (Item: %s, ID: %d) - Using AItemActor as fallback"),
			*DroppedItem->StaticData->DisplayName.ToString(),
			DroppedItem->Dynamic.ItemID);

		ActorClass = AItemActor::StaticClass();
	}

	// Get world
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] World is null"));
		return;
	}

	// ========================================
	// Use PoolingSubsystem to get item from pool
	// ========================================
	UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>();
	if (!PoolSys)
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] PoolingSubsystem not found!"));
		return;
	}

	// DEBUG: Before getting from pool
	UE_LOG(LogTemp, Warning, TEXT("[DiscardItemByIndex] ⚪ About to get actor from pool..."));

	// Get item from pool
	TScriptInterface<ISpawnable> SpawnedObj = PoolSys->SpawnFromClass(ActorClass, SpawnLocation);
	AItemActor* SpawnedActor = Cast<AItemActor>(SpawnedObj.GetObject());

	if (SpawnedActor)
	{
		// DEBUG: Log actor pointer to track reuse
		UE_LOG(LogTemp, Warning, TEXT("[DiscardItemByIndex] 🔵 Got actor from pool: %p, Location: %s, Hidden: %d"),
			SpawnedActor, *SpawnLocation.ToString(), SpawnedActor->IsHidden());

		// Set item data (discarded quantity)
		FBS_Item NewItemData = DroppedItem->Dynamic;
		NewItemData.Quantity = Count;

		SpawnedActor->InitializeItemBS_Item(NewItemData);
		SpawnedActor->SetOwner(nullptr);
		SpawnedActor->SetPooled(true);  // Mark as pooled item

		UE_LOG(LogTemp, Log, TEXT("[DiscardItemByIndex] ✅ Item spawned from pool: %s (ItemID: %d, Quantity: %d) at %s"),
			*ActorClass->GetName(),
			DroppedItem->Dynamic.ItemID,
			Count,
			*SpawnLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] ❌❌❌ POOL EXHAUSTED! Failed to spawn item! ❌❌❌"));
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] Item NOT removed from inventory (ItemID: %d, Count: %d)"),
			DroppedItem->Dynamic.ItemID, Count);
		UE_LOG(LogTemp, Error, TEXT("[DiscardItemByIndex] Solution: Increase pool size in GameMode or wait for items to be picked up"));

		// Do NOT decrease inventory quantity - spawn failed!
		// Broadcast to update UI (to show item is still there)
		SyncReplicatedData();  // 서버: 복제 데이터 동기화
		OnInventoryChanged.Broadcast();
		return;
	}

	// Decrease inventory quantity and weight
	DroppedItem->Dynamic.Quantity -= Count;

	// Calculate and decrease weight
	int32 DiscardedWeight = Count * DroppedItem->StaticData->Weight;
	CurrentWeight -= DiscardedWeight;

	UE_LOG(LogTemp, Log, TEXT("[DiscardItemByIndex] Weight decreased: -%d (New total: %d/%d)"),
		DiscardedWeight, CurrentWeight, MaxWeight);

	// Remove from inventory if quantity reaches 0
	if (DroppedItem->Dynamic.Quantity <= 0)
	{
		ItemInventory.Remove(DroppedItem);
		DroppedItem->MarkAsGarbage(); // Mark for garbage collection
	}

	// 서버: ItemInventory → ReplicatedItemData 동기화 (클라이언트로 복제)
	SyncReplicatedData();

	// Broadcast inventory changed event
	OnInventoryChanged.Broadcast();
}

// ========================================
// [Dedicated Server] ServerDiscardItem RPC Implementation
// ========================================
void UInventoryComponent::ServerDiscardItem_Implementation(int32 ItemIndex, int32 Count /*= 1*/)
{
	UE_LOG(LogTemp, Log, TEXT("[Server] ServerDiscardItem_Implementation called - ItemIndex: %d, Count: %d"), ItemIndex, Count);

	// Executed on server - calls DiscardItemByIndex to handle actual logic
	// Item spawned on server is automatically replicated to all clients
	// Inventory change is replicated via OnRep_ItemInventory
	DiscardItemByIndex(ItemIndex, Count);
}

// ========================================
// 아이템 관리 함수 (탄약용)
// ========================================

/**
 * 특정 아이템 ID의 총 개수 조회
 */
int32 UInventoryComponent::GetItemCountByID(uint8 ItemID) const
{
	if (ItemID == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::GetItemCountByID - Invalid ItemID: 0"));
		return 0;
	}

	// 서버/클라이언트 구분
	FString NetRole = GetOwner() ? (GetOwner()->HasAuthority() ? TEXT("[Server]") : TEXT("[Client]")) : TEXT("[Unknown]");

	int32 TotalCount = 0;
	int32 MatchedItems = 0;

	UE_LOG(LogTemp, Log, TEXT("%s GetItemCountByID - Searching for ItemID: %d in %d inventory slots"),
		*NetRole, ItemID, ItemInventory.Num());

	// 인벤토리에서 해당 ItemID의 총 개수 합산
	for (const UBSItemInstance* Item : ItemInventory)
	{
		if (Item && Item->StaticData && Item->StaticData->ItemID == ItemID)
		{
			TotalCount += Item->Dynamic.Quantity;
			MatchedItems++;
			UE_LOG(LogTemp, Log, TEXT("%s   - Found match: Quantity=%d"), *NetRole, Item->Dynamic.Quantity);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s GetItemCountByID - ItemID: %d, Matched %d stacks, TotalCount: %d"),
		*NetRole, ItemID, MatchedItems, TotalCount);

	return TotalCount;
}

/**
 * 인벤토리에서 아이템 소비 (탄약용)
 */
int32 UInventoryComponent::ConsumeItemByID(uint8 ItemID, int32 Amount)
{
	// 데디서버 환경: 서버에서만 인벤토리 수정 가능 (Replicated ItemInventory가 클라이언트로 전파됨)
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::ConsumeItemByID - Called on client! Ignoring. Use server authority."));
		return 0;
	}

	if (Amount <= 0)
	{
		return 0;
	}

	if (ItemID == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::ConsumeItemByID - Invalid ItemID: 0"));
		return 0;
	}

	int32 RemainingToConsume = Amount;

	// 인벤토리에서 해당 아이템을 찾아서 소비 (역순으로 순회 - 삭제를 위해)
	for (int32 i = ItemInventory.Num() - 1; i >= 0; i--)
	{
		UBSItemInstance* Item = ItemInventory[i];

		if (!Item || !Item->StaticData || Item->StaticData->ItemID != ItemID)
		{
			continue;  // 다른 아이템은 건너뜀
		}

		int32 ConsumedFromThisStack = FMath::Min(RemainingToConsume, Item->Dynamic.Quantity);
		Item->Dynamic.Quantity -= ConsumedFromThisStack;
		RemainingToConsume -= ConsumedFromThisStack;

		UE_LOG(LogTemp, Log, TEXT("UInventoryComponent::ConsumeItemByID - Consumed %d from stack (remaining in stack: %d)"),
			ConsumedFromThisStack, Item->Dynamic.Quantity);

		// 수량이 0이 되면 아이템 삭제
		if (Item->Dynamic.Quantity <= 0)
		{
			ItemInventory.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("UInventoryComponent::ConsumeItemByID - Removed empty item stack at index %d"), i);
		}

		// 필요한 만큼 소비했으면 종료
		if (RemainingToConsume <= 0)
		{
			break;
		}
	}

	// 서버: ItemInventory → ReplicatedItemData 동기화 (클라이언트로 복제)
	SyncReplicatedData();

	// 인벤토리 변경 알림
	OnInventoryChanged.Broadcast();

	// 실제로 소비된 개수 반환
	int32 ConsumedAmount = Amount - RemainingToConsume;
	UE_LOG(LogTemp, Log, TEXT("UInventoryComponent::ConsumeItemByID - ItemID: %d, Requested: %d, Consumed: %d"),
		ItemID, Amount, ConsumedAmount);

	return ConsumedAmount;
}

// ========================================
// 소비 아이템 사용 (힐, 에블라 아이템 등)
// ========================================

bool UInventoryComponent::UseItem(int32 ItemIndex)
{
	// 클라이언트에서 호출 시 서버 RPC로 전달
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerUseItem(ItemIndex);
		return true;
	}

	// 서버에서 실행되는 로직
	if (!ItemInventory.IsValidIndex(ItemIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UseItem] Invalid item index: %d"), ItemIndex);
		return false;
	}

	UBSItemInstance* ItemInstance = ItemInventory[ItemIndex];
	if (!ItemInstance || !ItemInstance->StaticData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UseItem] Item or StaticData is null at index: %d"), ItemIndex);
		return false;
	}

	// 아이템 수량 체크
	if (ItemInstance->Dynamic.Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UseItem] Item quantity is 0 at index: %d"), ItemIndex);
		return false;
	}

	const UBSStaticItemDataAsset* ItemData = ItemInstance->StaticData;
	const FText& ItemName = ItemData->DisplayName;
	const uint8 ItemID = ItemData->ItemID;

	UE_LOG(LogTemp, Log, TEXT("[UseItem] Using item: %s (ID: %d)"), *ItemName.ToString(), ItemID);

	// 델리게이트 발행 - Character가 구독해서 효과 적용
	OnItemUsed.Broadcast(ItemData);

	// 아이템 수량 감소
	ItemInstance->Dynamic.Quantity -= 1;
	CurrentWeight -= ItemInstance->StaticData->Weight;

	UE_LOG(LogTemp, Log, TEXT("[UseItem] Item consumed. Remaining: %d, Weight: %d/%d"),
		ItemInstance->Dynamic.Quantity, CurrentWeight, MaxWeight);

	// 수량이 0이 되면 인벤토리에서 제거
	if (ItemInstance->Dynamic.Quantity <= 0)
	{
		ItemInventory.RemoveAt(ItemIndex);
		ItemInstance->MarkAsGarbage();
		UE_LOG(LogTemp, Log, TEXT("[UseItem] Item removed from inventory (quantity reached 0)"));
	}

	// 서버: ItemInventory → ReplicatedItemData 동기화 (클라이언트로 복제)
	SyncReplicatedData();

	// 인벤토리 변경 브로드캐스트
	OnInventoryChanged.Broadcast();

	return true;
}

// ========================================
// [Server RPC] UseItem
// ========================================
void UInventoryComponent::ServerUseItem_Implementation(int32 ItemIndex)
{
	UE_LOG(LogTemp, Log, TEXT("[Server] ServerUseItem_Implementation called - ItemIndex: %d"), ItemIndex);
	UseItem(ItemIndex);
}
