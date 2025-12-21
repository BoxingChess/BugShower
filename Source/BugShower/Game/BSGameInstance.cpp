// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BSGameInstance.h"
#include "Manager/UIManager/BSUIManager.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"
#include "Save/BSPlayerSaveGame.h"
#include "Item/BSItemInstance.h"
#include "Kismet/GameplayStatics.h"

void UBSGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - Game Instance initialized"));

	// Verify that all subsystems are available
	// 모든 서브시스템이 사용 가능한지 확인

	// Check UIManager Subsystem
	// UIManager 서브시스템 확인
	UBSUIManager* UIManager = GetSubsystem<UBSUIManager>();
	if (UIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - UIManager Subsystem is available"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::Init - UIManager Subsystem NOT found!"));
	}

	// Check ItemResourceManager Subsystem
	// ItemResourceManager 서브시스템 확인
	UItemResourceManager* ItemResourceManager = GetSubsystem<UItemResourceManager>();
	if (ItemResourceManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BSGameInstance::Init - ItemResourceManager Subsystem is available"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::Init - ItemResourceManager Subsystem NOT found!"));
	}

	// PlayerID를 빈 문자열로 설정하여 기본 SaveSlot "PlayerSaveSlot" 사용
	// 각 클라이언트는 자신의 로컬 PC에 SaveGame 저장 → 슬롯 이름이 같아도 충돌 없음
	CurrentPlayerID = TEXT("");
	UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::Init - Using default save slot: %s"), *GetSaveSlotName());

	// 게임 시작 시 한 번만 SaveData 로드
	// ServerTravel 후에는 메모리에 있는 RuntimeItemInventory 유지
	LoadPlayerSaveData();
	UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::Init - SaveData loaded at startup"));
}

void UBSGameInstance::SetPlayerID(const FString& InPlayerID)
{
	CurrentPlayerID = InPlayerID;
	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::SetPlayerID - PlayerID set to: %s"), *CurrentPlayerID);
}

FString UBSGameInstance::GetSaveSlotName() const
{
	// PlayerID가 설정되어 있으면 플레이어별 세이브 슬롯 사용
	if (!CurrentPlayerID.IsEmpty())
	{
		return FString::Printf(TEXT("PlayerSave_%s"), *CurrentPlayerID);
	}

	// 기본 슬롯 (싱글플레이어 또는 ID 미설정 시)
	return TEXT("PlayerSaveSlot");
}

bool UBSGameInstance::LoadPlayerSaveData()
{
	// 플레이어별 세이브 슬롯 이름 가져오기
	FString SaveSlotName = GetSaveSlotName();

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::LoadPlayerSaveData - Attempting to load from slot: %s"), *SaveSlotName);

	// 세이브 파일이 존재하는지 확인
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		// 세이브 파일 로드
		CurrentSaveGame = Cast<UBSPlayerSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

		if (CurrentSaveGame)
		{
			UE_LOG(LogTemp, Log, TEXT("BSGameInstance::LoadPlayerSaveData - Save data loaded successfully!"));
			UE_LOG(LogTemp, Log, TEXT("  - Saved Items Count: %d"), CurrentSaveGame->SavedItems.Num());

			// 저장된 아이템을 RuntimeItemInventory로 변환
			// FBS_Item -> UBSItemInstance 변환이 필요
			RuntimeItemInventory.Empty();

			UItemResourceManager* ItemResourceManager = GetSubsystem<UItemResourceManager>();
			if (!ItemResourceManager)
			{
				UE_LOG(LogTemp, Error, TEXT("BSGameInstance::LoadPlayerSaveData - ItemResourceManager not found!"));
				return false;
			}

			for (const FBS_Item& SavedItem : CurrentSaveGame->SavedItems)
			{
				// ItemResourceManager를 통해 StaticData 가져오기
				const UBSStaticItemDataAsset* StaticData = ItemResourceManager->GetStaticItem(
					SavedItem.ItemType, SavedItem.ItemID);

				if (StaticData)
				{
					// UBSItemInstance 생성
					UBSItemInstance* NewInstance = NewObject<UBSItemInstance>(this);
					NewInstance->StaticData = StaticData;
					NewInstance->Dynamic = SavedItem;

					RuntimeItemInventory.Add(NewInstance);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::LoadPlayerSaveData - Failed to find StaticData for ItemType=%d, ItemID=%d"),
						static_cast<int32>(SavedItem.ItemType), SavedItem.ItemID);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("BSGameInstance::LoadPlayerSaveData - Runtime inventory populated with %d items"),
				RuntimeItemInventory.Num());

			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("BSGameInstance::LoadPlayerSaveData - Failed to cast save data!"));
			return false;
		}
	}
	else
	{
		// 세이브 파일이 없으면 새로 생성
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::LoadPlayerSaveData - No save file found. Creating new save data."));
		CurrentSaveGame = Cast<UBSPlayerSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UBSPlayerSaveGame::StaticClass()));

		if (CurrentSaveGame)
		{
			CurrentSaveGame->SaveSlotName = SaveSlotName;
			CurrentSaveGame->UserIndex = UserIndex;
			return true;
		}

		return false;
	}
}

bool UBSGameInstance::SavePlayerData()
{
	if (!CurrentSaveGame)
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::SavePlayerData - CurrentSaveGame is NULL!"));
		return false;
	}

	// 플레이어별 세이브 슬롯 이름 가져오기
	FString SaveSlotName = GetSaveSlotName();

	// RuntimeItemInventory를 FBS_Item 배열로 변환하여 저장
	CurrentSaveGame->SavedItems.Empty();

	for (const UBSItemInstance* ItemInstance : RuntimeItemInventory)
	{
		if (ItemInstance)
		{
			CurrentSaveGame->SavedItems.Add(ItemInstance->Dynamic);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::SavePlayerData - Saving to slot: %s"), *SaveSlotName);

	// 디스크에 저장
	bool bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, UserIndex);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("BSGameInstance::SavePlayerData - Save successful! Saved %d items."),
			CurrentSaveGame->SavedItems.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BSGameInstance::SavePlayerData - Save failed!"));
	}

	return bSuccess;
}

void UBSGameInstance::AddItemsToRuntimeInventory(const TArray<UBSItemInstance*>& ItemsToAdd)
{
	for (UBSItemInstance* Item : ItemsToAdd)
	{
		if (Item)
		{
			// 중복 체크: 같은 아이템이 이미 있으면 수량 합산
			bool bFoundExisting = false;

			for (UBSItemInstance* ExistingItem : RuntimeItemInventory)
			{
				if (ExistingItem &&
					ExistingItem->Dynamic.ItemType == Item->Dynamic.ItemType &&
					ExistingItem->Dynamic.ItemID == Item->Dynamic.ItemID)
				{
					// 같은 아이템 발견 - 수량 합산
					ExistingItem->Dynamic.Quantity += Item->Dynamic.Quantity;
					bFoundExisting = true;

					UE_LOG(LogTemp, Log, TEXT("BSGameInstance::AddItemsToRuntimeInventory - Merged item (ID=%d, NewQuantity=%d)"),
						Item->Dynamic.ItemID, ExistingItem->Dynamic.Quantity);
					break;
				}
			}

			// 새로운 아이템이면 추가
			if (!bFoundExisting)
			{
				// 복사본 생성 (GameInstance가 소유하도록)
				UBSItemInstance* NewInstance = NewObject<UBSItemInstance>(this);
				NewInstance->StaticData = Item->StaticData;
				NewInstance->Dynamic = Item->Dynamic;

				RuntimeItemInventory.Add(NewInstance);

				UE_LOG(LogTemp, Log, TEXT("BSGameInstance::AddItemsToRuntimeInventory - Added new item (ID=%d, Quantity=%d)"),
					Item->Dynamic.ItemID, Item->Dynamic.Quantity);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::AddItemsToRuntimeInventory - Total items in inventory: %d"),
		RuntimeItemInventory.Num());

	// 인벤토리 변경 알림
	OnStorageChanged.Broadcast(RuntimeItemInventory);
}

const TArray<UBSItemInstance*>& UBSGameInstance::GetPlayerItems() const
{
	return RuntimeItemInventory;
}

void UBSGameInstance::ResetSaveData()
{
	// 런타임 인벤토리 초기화
	RuntimeItemInventory.Empty();

	// 플레이어별 세이브 슬롯 이름 가져오기
	FString SaveSlotName = GetSaveSlotName();

	// 세이브 데이터 초기화
	if (CurrentSaveGame)
	{
		CurrentSaveGame->SavedItems.Empty();
		CurrentSaveGame->PlayerLevel = 1;
		CurrentSaveGame->PlayerExperience = 0;
		CurrentSaveGame->PlayerGold = 0;

		// 디스크에 빈 데이터 저장
		UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, UserIndex);

		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::ResetSaveData - All save data has been reset for slot: %s"), *SaveSlotName);
	}
}

void UBSGameInstance::AddItemsToInventoryForGame(UBSItemInstance* AddData, int32 Amount)
{
	if (!AddData || Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::AddItemsToInventoryForGame - Invalid AddData or Amount"));
		return;
	}

	int32 ItemID = AddData->Dynamic.ItemID;

	// 1. RuntimeItemInventory에서 차감 가능한지 확인
	UBSItemInstance* SourceItem = nullptr;
	for (UBSItemInstance* Item : RuntimeItemInventory)
	{
		if (Item && Item->Dynamic.ItemID == ItemID)
		{
			SourceItem = Item;
			break;
		}
	}

	if (!SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::AddItemsToInventoryForGame - Item not found in RuntimeInventory (ID=%d)"), ItemID);
		return;
	}

	if (SourceItem->Dynamic.Quantity < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::AddItemsToInventoryForGame - Not enough items (Have=%d, Want=%d)"),
			SourceItem->Dynamic.Quantity, Amount);
		return;
	}

	// 2. SelectedItemsForGame에서 같은 아이템 찾기
	UBSItemInstance* FoundSelected = nullptr;
	for (UBSItemInstance* Item : SelectedItemsForGame)
	{
		if (Item && Item->Dynamic.ItemID == ItemID)
		{
			FoundSelected = Item;
			break;
		}
	}

	// 3-1. 이미 선택된 아이템이면 수량만 증가
	if (FoundSelected)
	{
		FoundSelected->Dynamic.Quantity += Amount;
		UE_LOG(LogTemp, Log, TEXT("✅ Updated existing selected item (ID=%d, NewQuantity=%d)"),
			ItemID, FoundSelected->Dynamic.Quantity);
	}
	// 3-2. 새로운 아이템이면 복사본 생성 후 추가
	else
	{
		UBSItemInstance* NewInstance = NewObject<UBSItemInstance>(this);
		NewInstance->StaticData = AddData->StaticData;
		NewInstance->Dynamic = AddData->Dynamic;
		NewInstance->Dynamic.Quantity = Amount;
		SelectedItemsForGame.Add(NewInstance);

		UE_LOG(LogTemp, Log, TEXT("✅ Added new selected item (ID=%d, Quantity=%d)"), ItemID, Amount);
	}

	// 4. RuntimeItemInventory에서 차감
	SourceItem->Dynamic.Quantity -= Amount;
	UE_LOG(LogTemp, Log, TEXT("📉 Deducted from RuntimeInventory (ID=%d, Remaining=%d)"),
		ItemID, SourceItem->Dynamic.Quantity);

	// 5. 수량이 0이 되면 배열에서 제거
	if (SourceItem->Dynamic.Quantity <= 0)
	{
		RuntimeItemInventory.Remove(SourceItem);
		UE_LOG(LogTemp, Log, TEXT("🗑️ Removed depleted item from RuntimeInventory (ID=%d)"), ItemID);
	}

	// 인벤토리 변경 알림
	OnSelectedItemsChanged.Broadcast(SelectedItemsForGame);
	OnStorageChanged.Broadcast(RuntimeItemInventory);
}

void UBSGameInstance::AddItemsToStarage(UBSItemInstance* AddData, int32 Amount)
{
	if (!AddData || Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::AddItemsToStarage - Invalid AddData or Amount"));
		return;
	}

	int32 ItemID = AddData->Dynamic.ItemID;

	// 1. SelectedItemsForGame에서 차감 가능한지 확인
	UBSItemInstance* SourceItem = nullptr;
	for (UBSItemInstance* Item : SelectedItemsForGame)
	{
		if (Item && Item->Dynamic.ItemID == ItemID)
		{
			SourceItem = Item;
			break;
		}
	}

	if (!SourceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::AddItemsToStarage - Item not found in SelectedItemsForGame (ID=%d)"), ItemID);
		return;
	}

	if (SourceItem->Dynamic.Quantity < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSGameInstance::AddItemsToStarage - Not enough items (Have=%d, Want=%d)"),
			SourceItem->Dynamic.Quantity, Amount);
		return;
	}

	// 2. RuntimeItemInventory에서 같은 아이템 찾기
	UBSItemInstance* FoundSelected = nullptr;
	for (UBSItemInstance* Item : RuntimeItemInventory)
	{
		if (Item && Item->Dynamic.ItemID == ItemID)
		{
			FoundSelected = Item;
			break;
		}
	}

	// 3-1. 이미 선택된 아이템이면 수량만 증가
	if (FoundSelected)
	{
		FoundSelected->Dynamic.Quantity += Amount;
		UE_LOG(LogTemp, Log, TEXT("✅ Updated existing selected item (ID=%d, NewQuantity=%d)"),
			ItemID, FoundSelected->Dynamic.Quantity);
	}
	// 3-2. 새로운 아이템이면 복사본 생성 후 추가
	else
	{
		UBSItemInstance* NewInstance = NewObject<UBSItemInstance>(this);
		NewInstance->StaticData = AddData->StaticData;
		NewInstance->Dynamic = AddData->Dynamic;
		NewInstance->Dynamic.Quantity = Amount;
		RuntimeItemInventory.Add(NewInstance);

		UE_LOG(LogTemp, Log, TEXT("✅ Added new RuntimeItemInventory item (ID=%d, Quantity=%d)"), ItemID, Amount);
	}

	// 4. SelectedItemsForGame에서 차감
	SourceItem->Dynamic.Quantity -= Amount;
	UE_LOG(LogTemp, Log, TEXT("📉 Deducted from SelectedItems (ID=%d, Remaining=%d)"),
		ItemID, SourceItem->Dynamic.Quantity);

	// 5. 수량이 0이 되면 배열에서 제거
	if (SourceItem->Dynamic.Quantity <= 0)
	{
		SelectedItemsForGame.Remove(SourceItem);
		UE_LOG(LogTemp, Log, TEXT("🗑️ Removed depleted item from SelectedItems (ID=%d)"), ItemID);
	}

	// 인벤토리 변경 알림
	OnSelectedItemsChanged.Broadcast(SelectedItemsForGame);
	OnStorageChanged.Broadcast(RuntimeItemInventory);
}

const TArray<UBSItemInstance*>& UBSGameInstance::GetSelecedItems() const
{
	return SelectedItemsForGame;
}

void UBSGameInstance::ClearSelectedItems()
{
	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::ClearSelectedItems - Clearing %d selected items"), SelectedItemsForGame.Num());

	SelectedItemsForGame.Empty();

	// 인벤토리 변경 알림
	OnSelectedItemsChanged.Broadcast(SelectedItemsForGame);

	UE_LOG(LogTemp, Log, TEXT("BSGameInstance::ClearSelectedItems - Selected items cleared"));
}
