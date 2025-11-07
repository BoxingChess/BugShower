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

	UE_LOG(LogTemp, Warning, TEXT("UPickUpDetectorComponent::BeginPlay"));

	///�̺κ� ���� �� ���ذ� �ʿ���. -> ��������� �������� �ʾ���..
	// 	// ����(����/�������� ���� ��Ʈ)������ ���α� ->���������� ��Ŀ�� ������ ������ �ʿ䰡 ������
	// 	if (GetNetMode() != NM_Client) 
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("!NM_Client"));
	// 
	// 		SetComponentTickEnabled(false);
	// 		return;
	// 	}

		// Ŭ��: ���÷� ���� ���� Pawn�� ���� ��
	if (const auto P = Cast<APawn>(GetOwner());
		//�� pawn�� ���� ���� �������ΰ�?
		P && P->IsLocallyControlled())
	{
		PrimaryComponentTick.bCanEverTick = true;
		SetComponentTickEnabled(true);
		SetComponentTickInterval(0.1f); //�ʴ� 2���� ƽ ������
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
	AItemActor* NewFocus = nullptr;
	if (bHit)
	{
		NewFocus = Cast<AItemActor>(Hit.GetActor());  // ������ �ƴϸ� nullptr
	}

	// ��Ŀ���� �ٲ���� ���� ��������Ʈ ȣ��
	if (FocusedItem.Get() != NewFocus)
	{
		FocusedItem = NewFocus;
		OnFocusItemChanged.Broadcast(NewFocus);   //���⼭ UI�� �̺�Ʈ ����

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
					// Update pickup prompt with item data (or hide if no item)
					UIManager->UpdatePickupPrompt(NewFocus);
				}
			}
		}

		return true;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// ����� �ð�ȭ
	const FColor LineColor = NewFocus ? FColor::Red : (bHit ? FColor::Yellow : FColor::Cyan);
	DrawDebugLine(GetWorld(), ViewLoc, End, LineColor, /*bPersistent*/false, /*LifeTime*/0.05f, 0, /*Thickness*/0.1f);

	if (bHit)
	{
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, /*Size*/10.f, LineColor, false, 0.05f);

		const FString NameStr = (NewFocus && NewFocus->GetItemStaticData())
			? NewFocus->GetItemStaticData()->DisplayName.ToString()
			: TEXT("<invalid>");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *NameStr);
	}
#endif

	// ��Ŀ�� ���� �� ��������Ʈ ��ε�ĳ��Ʈ
	if (FocusedItem.Get() != NewFocus)
	{
		FocusedItem = NewFocus;
		return true; // �����
	}


	return false;
}

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

	// ��ȿ ������ Ŭ����(Ȥ�ö� ���� �� ������)
	NearbyItems.RemoveAll([](const TWeakObjectPtr<AActor>& P) { return !P.IsValid(); });


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

void UPickUpDetectorComponent::TryPickupFocused()
{
	UE_LOG(LogTemp, Warning, TEXT("TryPickupFocused"));

	// ���⼱ �����۸� ����
	if (AItemActor* Item = FocusedItem.Get())
	{
		ServerTryPickup(Item);  // ���� RPC
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
	UE_LOG(LogTemp, Warning, TEXT("ChangeState - Toggling inventory UI"));

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

	// Toggle inventory widget
	UIManager->ToggleWidget(FName("Inventory"), PC);

	// Update internal state
	bIsInventoryOpen = !bIsInventoryOpen;
	bIsLineTrace = !bIsInventoryOpen;

	UE_LOG(LogTemp, Log, TEXT("PickUpDetectorComponent::ChangeState - Inventory is now %s"),
		bIsInventoryOpen ? TEXT("OPEN") : TEXT("CLOSED"));
}

