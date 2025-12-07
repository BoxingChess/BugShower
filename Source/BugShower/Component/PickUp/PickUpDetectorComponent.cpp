// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PickUp/PickUpDetectorComponent.h"

#include "Component/Movement/MovementInputComponent.h"
#include "Component/Inventory/InventoryComponent.h"

#include "Engine/World.h"             // UWorld::OverlapMultiByObjectType
#include "Engine/OverlapResult.h"
#include "Engine/EngineTypes.h"       // FCollisionQueryParams, FCollisionObjectQueryParams
#include "CollisionQueryParams.h"   // FCollisionQueryParams
#include "CollisionShape.h"           // FCollisionShape
#include "DrawDebugHelpers.h"
#include "Manager/UIManager/BSUIManager.h"
#include "GameFramework/PlayerController.h"

#include "Player/BSCharacterPlayer.h"
#include "Item/ItemActor.h"
#include "Weapon/WeaponActor.h"




// Sets default values for this component's properties
UPickUpDetectorComponent::UPickUpDetectorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ItemActorClass = AItemActor::StaticClass();	//�⺻��
	// ...
}


// Called when the game starts
void UPickUpDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("PickUpDetectorComponent::BeginPlay - Initializing"));

	// 데디케이티드 서버 최적화: 서버에서는 UI/입력 관련 로직 불필요
	// Dedicated Server Optimization: No need for UI/input on server
	if (GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Log, TEXT("PickUpDetectorComponent: Running on dedicated server, disabling tick"));
		SetComponentTickEnabled(false);
		return; // Early exit for dedicated server
	}

	// 클라이언트 전용: 로컬로 제어되는 Pawn에서만 Tick 활성화
	// Client-only: Enable tick only for locally controlled Pawn
	if (const auto P = Cast<APawn>(GetOwner());
		P && P->IsLocallyControlled())
	{
		PrimaryComponentTick.bCanEverTick = true;
		SetComponentTickEnabled(true);
		SetComponentTickInterval(0.1f); // Tick every 0.1 seconds (10 Hz)
	}
	else
	{
		SetComponentTickEnabled(false);
	}

	//���� ���ʿ� ���� MovementInputComponent ã��
	if (auto* MoveInput = GetOwner()->FindComponentByClass<UMovementInputComponent>())
	{
		UE_LOG(LogTemp, Warning, TEXT("FindComponentByClass<UMovementInputComponent>"));

		//��ε�ĳ��Ʈ�� ���ε�
		MoveInput->OnInventoryToggleRequested.AddUniqueDynamic
		(
			this, &UPickUpDetectorComponent::ChangeState
		);

		//��ε�ĳ��Ʈ�� ���ε�
		MoveInput->OnKeyToggleRequested.AddUniqueDynamic
		(
			this, &UPickUpDetectorComponent::TryPickupFocused
		);

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("!FindComponentByClass<UMovementInputComponent>"));

	}

	if (ABSCharacterPlayer* P = Cast<ABSCharacterPlayer>(GetOwner()))
	{
		OwnerChar = P;             // OK
	}
	else
	{
		OwnerChar = nullptr;       // Ÿ���� �ٸ��� ��ȿȭ
	}
}


// Called every frame
void UPickUpDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// ���� ����
	const auto Pawn = Cast<APawn>(GetOwner());
	if (GetNetMode() == NM_DedicatedServer || !Pawn || !Pawn->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("NM_DedicatedServer"));

		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("TickComponent"));



	//�κ��丮�� �������� ��� ��� �Ұ�����?
	if (bIsInventoryOpen)
	{
		//UE_LOG(LogTemp, Warning, TEXT("bIsInventoryOpen"));

		//�ֺ��� �����۵��� Ž���Ѵ�.
		RefreshNearbyList();
	}
	//������ ������ ���� ���� Ʈ���̽��� �������?
	else
	{
		//ī�޶� �������� ���� �� �ϳ��� �������� Ž���Ѵ�.
		LineTraceFocus();
	}
}

// 클라이언트 전용: 카메라 라인트레이스로 아이템 포커스 감지
// Client-only: Detect item focus using camera line trace
bool UPickUpDetectorComponent::LineTraceFocus()
{
	if (!OwnerChar.IsValid()) return false;

	// ī�޶� ��ġ/���� ��������
	FVector ViewLoc;
	FRotator ViewRot;
	if (AController* PlayerConroller = OwnerChar->GetController())
	{
		// Returns Player's Point of View
		PlayerConroller->GetPlayerViewPoint(ViewLoc, ViewRot);
	}
	else
	{
		//Returns Pawn's eye location, Get the view rotation of the Pawn
		OwnerChar->GetActorEyesViewPoint(ViewLoc, ViewRot);
	}

	//���� ���� ���
	const FVector End = ViewLoc + (ViewRot.Vector() * FocusTraceDistance);

	// �浹 �Ķ���� ���� (�ڱ� �ڽ� ����)
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PickupFocus), false, GetOwner());
	Params.AddIgnoredActor(OwnerChar.Get());


	// ����ĳ��Ʈ ����
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel
	(
		Hit, ViewLoc, End, FocusTraceChannel, Params
	);

	// ���� ���Ͱ� ���������� �Ǻ�
	AItemActor* NewFocusItem = nullptr;
	AWeaponActor* NewFocusWeapon = nullptr;

	if (bHit)
	{
		// 아이템인지 확인
		NewFocusItem = Cast<AItemActor>(Hit.GetActor());
		// 무기인지 확인
		if (!NewFocusItem)
		{
			NewFocusWeapon = Cast<AWeaponActor>(Hit.GetActor());
		}
	}

	// ��Ŀ���� �ٲ���� ���� ��������Ʈ ȣ��
	bool bChanged = (FocusedItem.Get() != NewFocusItem) || (FocusedWeapon.Get() != NewFocusWeapon);
	if (bChanged)
	{
		FocusedItem = NewFocusItem;
		FocusedWeapon = NewFocusWeapon;
		OnFocusItemChanged.Broadcast(NewFocusItem);   //���⼭ UI�� �̺�Ʈ ����

		// Update LineTrace UI through UIManager
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn)
		{
			APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
			if (PC && PC->GetGameInstance())
			{
				UBSUIManager* UIManager = PC->GetGameInstance()->GetSubsystem<UBSUIManager>();
				if (UIManager)
				{
					// Update pickup prompt with item data (or hide if no item/weapon)
					if (NewFocusItem)
					{
						UIManager->UpdatePickupPrompt(NewFocusItem);
					}
					else if (NewFocusWeapon && NewFocusWeapon->GetWeaponData())
					{
						// 무기 UI 표시 (무기 이름)
						// TODO: UpdatePickupPrompt를 무기도 받을 수 있도록 수정하거나 별도 함수 추가
						// 임시로 로그만 출력
						UE_LOG(LogTemp, Log, TEXT("Focused Weapon: %s"), *NewFocusWeapon->GetWeaponData()->WeaponName.ToString());
					}
					else
					{
						UIManager->UpdatePickupPrompt(nullptr);  // UI 숨기기
					}
				}
			}
		}

		return true;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// ����� �ð�ȭ
	const bool bHasValidTarget = (NewFocusItem != nullptr) || (NewFocusWeapon != nullptr);
	const FColor LineColor = bHasValidTarget ? FColor::Red : (bHit ? FColor::Yellow : FColor::Cyan);
	DrawDebugLine(GetWorld(), ViewLoc, End, LineColor, /*bPersistent*/false, /*LifeTime*/0.05f, 0, /*Thickness*/0.1f);

	if (bHit)
	{
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, /*Size*/10.f, LineColor, false, 0.05f);

		FString NameStr = TEXT("<invalid>");
		if (NewFocusItem && NewFocusItem->GetItemStaticData())
		{
			NameStr = NewFocusItem->GetItemStaticData()->DisplayName.ToString();
		}
		else if (NewFocusWeapon && NewFocusWeapon->GetWeaponData())
		{
			NameStr = NewFocusWeapon->GetWeaponData()->WeaponName.ToString();
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *NameStr);
	}
#endif

	return false;
}

// 클라이언트 전용: 주변 아이템 목록 새로고침 (구체 오버랩 사용)
// Client-only: Refresh nearby item list using sphere overlap
void UPickUpDetectorComponent::RefreshNearbyList(bool isForced /*= false*/)
{
	//UE_LOG(LogTemp, Warning, TEXT("call RefreshNearbyList"));


	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// ���ø� ������ �ְ�
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (!Pawn->IsLocallyControlled())
		{
			return;
		}
	}

	UWorld* World = Owner->GetWorld();
	if (!World)
	{
		return;
	}

	//����� ���� �迭, ��ĵ�� �߽���ǥ
	TArray<FOverlapResult> Overlaps;
	const FVector Center = Owner->GetActorLocation();

	//�� ����
	FCollisionShape Sphere = FCollisionShape::MakeSphere(NearbyRadius);

	//���� �Ķ����
	// SCENE_QUERY_STAT: �������ϸ� �±�(������/�������Ϸ����� �̸����� ����)
	// bTraceComplex=false: �ܼ� �浹�� ��� (��� ����)
	// Owner: ������ ����(�ڱ� �ڽ� ����)
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(nullptr), /*bTraceComplex*/false, Owner);
	QueryParams.bReturnPhysicalMaterial = false; // ���� ��Ƽ���� ���ʿ� �� ��� ����

	//������Ʈ ä�� ��� ����(������ ���� ä�θ� ��ȸ)
	//������ ECC_Visibility�� ��� ���� ��ĥ��. TODO
	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(NearbyChannel);

	//������ ����
	const bool bAny = World->OverlapMultiByObjectType
	(
		Overlaps, 			 //out���
		Center, 			 //������ġ
		FQuat::Identity, 	 //ȸ��X
		ObjParams, 			 //������Ʈ ä�� ��� ���� TODO : ���� �ٲܰ�.
		Sphere, 			 //�� ����
		QueryParams			 //���� �Ķ����
	);

	DrawDebugSphere(
		GetWorld(),
		Center,
		NearbyRadius,
		16,
		FColor::Cyan,
		/*bPersistentLines*/false,
		/*LifeTime*/0.1f,
		0,
		/*Thickness*/1.0f);


	// ���� �ڷᱸ�� �ʱ�ȭ
	NearbyItems.Reset();

	if (bAny)
	{
		//�ʿ� ������ŭ �Ҵ�
		NearbyItems.Reserve(Overlaps.Num());

		//��Ʈ�� ���� ��ȸ
		for (const FOverlapResult& R : Overlaps)
		{
			AActor* HitActor = R.GetActor();

			if (!HitActor || HitActor == Owner)
			{
				continue;	//�ڱ��ڽ� ����
			}

			if (!IsItemActor(HitActor))
			{
				continue;	//������ Ŭ��������?
			}


			NearbyItems.AddUnique(HitActor);
		}

		// �Ÿ��� ���� -> ���� Enum������ ��ĥ��.
// 		NearbyItems.Sort([&Center](AActor* const& A, AActor* const& B)
// 			{
// 				if (!A || !B) return !!A; // ��ȿ�� �� �켱
// 				return FVector::DistSquared(A->GetActorLocation(), Center)
// 					< FVector::DistSquared(B->GetActorLocation(), Center);
// 			});
	}

	// 무효한 포인터 제거 (nullptr인 항목 삭제)
	// 수정: TWeakObjectPtr -> AActor* 타입으로 수정 (타입 미스매치 버그 해결)
	NearbyItems.RemoveAll([](const AActor* P) { return !P; });


	bool bChanged = false;

	// 1) ���� ��
	if (NearbyItems.Num() != PreviousNearbyItems.Num())
	{
		bChanged = true;
	}
	else
	{
		// 2) �׸� ��
		for (int32 i = 0; i < NearbyItems.Num(); i++)
		{
			if (NearbyItems[i] != PreviousNearbyItems[i])
			{
				bChanged = true;
				break;
			}
		}
	}

	if (bChanged || isForced)
	{
		// �α� Ȯ�ο�
		UE_LOG(LogTemp, Warning, TEXT("Nearby items changed! Count = %d"), NearbyItems.Num());

		//���!
		OnRefreshNearbyList.Broadcast();

		// ���� �� ������Ʈ
		PreviousNearbyItems = NearbyItems;
	}

}

// 클라이언트 시작: F키 입력으로 포커스된 아이템 줍기 시도 -> 서버 RPC 호출
// Client initiates: Try pickup focused item with F key -> calls Server RPC
void UPickUpDetectorComponent::TryPickupFocused()
{
	UE_LOG(LogTemp, Warning, TEXT("TryPickupFocused"));

	// 아이템 픽업 시도
	if (AItemActor* Item = FocusedItem.Get())
	{
		ServerTryPickup(Item);  // 서버 RPC
	}
	// 무기 픽업 시도
	else if (AWeaponActor* Weapon = FocusedWeapon.Get())
	{
		// 무기 장착 (플레이어 캐릭터의 EquipWeapon 호출)
		if (ABSCharacterPlayer* Player = Cast<ABSCharacterPlayer>(GetOwner()))
		{
			Player->EquipWeapon(Weapon);
			UE_LOG(LogTemp, Log, TEXT("TryPickupFocused - Equipped weapon: %s"),
				Weapon->GetWeaponData() ? *Weapon->GetWeaponData()->WeaponName.ToString() : TEXT("Unknown"));
		}
	}

}

void UPickUpDetectorComponent::ServerTryPickup_Implementation(class AItemActor* Item)
{
	///���⼭���ʹ� �������� ����ȴ�.

	//�������� ���ų� �ı������� ���ų� ���� �ı�������?
	if (!IsValid(Item) || Item->IsActorBeingDestroyed() || Item->IsPendingKillPending()) return;

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	// �κ��丮 ������Ʈ ã��
	if (UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		// ���� AddItem�� DroppedActor�� �޾� ���ο��� ����/����/�ı����� ó����
		Inv->AddItem(Item);

		//����������� ������ 0�� ��� -> ������ �����ȰŴ� ���͸� ����
		if (Item->GetItemData().Quantity == 0)
		{
			Item->Destroy(); // ������ ������
		}

		// AddItem���� ���� 0�̸� Destroy()���� ó���ϹǷ�
		// ���⼭ ���� ���Ŵ� ���ʿ�. ��Ŀ���� ������ �ִ� �� ���.
		if (!IsValid(Item))
		{
			FocusedItem = nullptr;
		}
	}
}

void UPickUpDetectorComponent::ServerTryPickupPartial_Implementation(class AItemActor* Item, int32 Quantity)
{
	if (!IsValid(Item)) return;
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	if (UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		FBS_Item& ItemData = Item->GetItemData();
		const UBSStaticItemDataAsset* Static = Item->GetItemStaticData();
		if (!Static) return;

		int32 TakeAmount = FMath::Clamp(Quantity, 1, ItemData.Quantity);

		// �ӽ� ���纻�� ���� TakeAmount ��ŭ�� ������ �� ItemActor ����
		// �� Inv->AddItem �� AItemActor* �� �ʿ��ϴϱ�
		AItemActor* TempActor = GetWorld()->SpawnActor<AItemActor>(Item->GetClass());
		FBS_Item PartialData = ItemData;
		PartialData.Quantity = TakeAmount;
		TempActor->InitializeItemBS_Item(PartialData);

		Inv->AddItem(TempActor);   // ���� AddItem ���
		TempActor->Destroy();      // �ӽ� ���ʹ� �κ��丮 ó�� �� ����

		ItemData.Quantity -= TakeAmount;
		Item->InitializeItemBS_Item(ItemData);
		if (ItemData.Quantity <= 0)
		{
			Item->Destroy();
			FocusedItem = nullptr;
		}
	}
}

bool UPickUpDetectorComponent::IsItemActor(AActor* Actor) const
{
	return Actor && Actor->IsA(AItemActor::StaticClass());
}

bool UPickUpDetectorComponent::PickupInternal(AActor* Item)
{
	return false;
}

void UPickUpDetectorComponent::ChangeState()
{
	UE_LOG(LogTemp, Warning, TEXT("ChangeState - Current state: %s"),
		bIsInventoryOpen ? TEXT("OPEN") : TEXT("CLOSED"));

	// Get UIManager from GameInstance
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("PickUpDetectorComponent::ChangeState - Owner is not a Pawn"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("PickUpDetectorComponent::ChangeState - No PlayerController found"));
		return;
	}

	UGameInstance* GameInstance = PC->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("PickUpDetectorComponent::ChangeState - No GameInstance found"));
		return;
	}

	UBSUIManager* UIManager = GameInstance->GetSubsystem<UBSUIManager>();
	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("PickUpDetectorComponent::ChangeState - No UIManager found"));
		return;
	}

	// Toggle state first
	bIsInventoryOpen = !bIsInventoryOpen;
	bIsLineTrace = !bIsInventoryOpen;

	// Then explicitly call Show or Hide based on the new state
	if (bIsInventoryOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChangeState - Opening inventory (calling ShowWidget)"));
		UIManager->ShowWidget(FName("Inventory"), PC);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChangeState - Closing inventory (calling HideWidget)"));
		UIManager->HideWidget(FName("Inventory"), PC);

		// FORCE GameOnly mode when closing inventory (ignore LineTraceUI)
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		UE_LOG(LogTemp, Error, TEXT("!!! FORCED GameOnly mode after closing inventory !!!"));
	}

	UE_LOG(LogTemp, Log, TEXT("PickUpDetectorComponent::ChangeState - Inventory is now %s"),
		bIsInventoryOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

