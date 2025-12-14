// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/InventoryComponent.h"
#include "Item/ItemActor.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Net/UnrealNetwork.h"

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

	// Replicate ItemInventory to all clients
	DOREPLIFETIME(UInventoryComponent, ItemInventory);
}

// Called automatically when ItemInventory is replicated (on client)
void UInventoryComponent::OnRep_ItemInventory()
{
	UE_LOG(LogTemp, Log, TEXT("[Client] OnRep_ItemInventory called - Inventory changed on server, syncing to client"));

	// Broadcast delegate to update UI
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

	// Get item data (dynamic) and static data asset
	FBS_Item& DropItem = DroppedActor->GetItemData();
	const UBSStaticItemDataAsset* DropStatic = DroppedActor->GetItemStaticData();

	// Validate static data
	if (!DropStatic)
	{
		UE_LOG(LogTemp, Error, TEXT("AddItem Failed: DropStatic is null!"));
		return;
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
	// Consumable Item Handling
	// ========================================
	if (DropItem.ItemType == EItemType::Consumable)
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
	// Validate index
	if (!ItemInventory.IsValidIndex(ItemIndex) || !GetOwner())
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
