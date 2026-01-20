// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Item/ItemActor.h"
#include "Game/BSGameInstance.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"
#include "Player/BSCharacterPlayer.h"
#include "Subsystems/PoolingSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Component/Inventory/InventoryComponent.h"
#include "Logging/BugShowerLog.h"
#include "Net/UnrealNetwork.h"


void AItemActor::Spawn(const FVector pos)
{
	// DEBUG: Log this actor being spawned with current lifespan and hidden state
	float CurrentLifeSpan = GetLifeSpan();
	bool bIsCurrentlyHidden = IsHidden();
	UE_LOG(LogTemp, Warning, TEXT("[ItemActor::Spawn] 🟢 Actor %p spawning at %s, CurrentLifeSpan: %.2f, WasHidden: %d"),
		this, *pos.ToString(), CurrentLifeSpan, bIsCurrentlyHidden);

	// IMPORTANT: Clear lifespan FIRST to prevent immediate expiration!
	SetLifeSpan(0.0f);

	// For physics-enabled actors, need special handling
	if (MeshComponent && MeshComponent->IsSimulatingPhysics())
	{
		// Temporarily disable physics, set location, then re-enable
		MeshComponent->SetSimulatePhysics(false);
		SetActorLocation(pos);
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);  // Reset velocity
		MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);  // Reset rotation
	}
	else
	{
		// Standard activation
		Activate(this, pos);
	}

	// Set data from StaticItemInfo if available
	if (StaticItemInfo)
	{
		ItemInformation.ItemID = StaticItemInfo->ItemID;
		ItemInformation.ItemType = StaticItemInfo->ItemType;
		ItemInformation.Quantity = StaticItemInfo->SpawnQuntity;

		UE_LOG(LogTemp, Log, TEXT("Spawn() 호출: ItemID=%d, Type=%d, Quantity=%d, Position=%s"),
			ItemInformation.ItemID, (int32)ItemInformation.ItemType, ItemInformation.Quantity, *pos.ToString());
	}
	else
	{
		// StaticItemInfo will be set later via InitializeItemBS_Item()
		UE_LOG(LogTemp, Warning, TEXT("Spawn() 호출: StaticItemInfo is null (will be initialized later), Position=%s"), *pos.ToString());
	}

	// 클라이언트에 활성화 상태 리플리케이트
	SetPoolActive(true);
}

void AItemActor::ReturnPool()
{
	// 클라이언트에 비활성화 상태 리플리케이트
	SetPoolActive(false);

	Deactivate(this);

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPoolByClass(this);
		}
	}
}

void AItemActor::LifeSpanExpired()
{
	DeSpawn();
}

void AItemActor::DeSpawn()
{
	// DEBUG: Log this actor being despawned
	UE_LOG(LogTemp, Warning, TEXT("[ItemActor::DeSpawn] 🟠 Actor %p despawning"), this);

	// 클라이언트에 비활성화 상태 리플리케이트
	SetPoolActive(false);

	Deactivate(this);

	// Return to pool via subsystem
	if (UWorld* World = GetWorld())
	{
		if (UPoolingSubsystem* PoolSys = World->GetSubsystem<UPoolingSubsystem>())
		{
			PoolSys->ReturnToPoolByClass(this);
		}
	}
}

 

// Sets default values
AItemActor::AItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Enable replication(pos,life span, type)
	bReplicates = true;
	SetReplicateMovement(true);

	// Create mesh component as root (required for physics simulation)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Enable physics simulation
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetEnableGravity(true);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);  // Both query and physics

	// Set collision responses
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);    // Block ground/walls
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);   // Block dynamic objects
	MeshComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);   // Ignore other items (no item-item collision)
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);          // Ignore players (pass through!)
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);     // Block visibility trace


	///이제 아래것을 Use Complex Collision As Simple설정 처럼 바꿀거라 주석처리를 하겠음.
	// Create collision component (sphere for pickup detection)
// 	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
// 	CollisionComponent->SetupAttachment(RootComponent);
// 	CollisionComponent->InitSphereRadius(50.f);
// 	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
// 	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
// 	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

// ========================================
// 물리 설정 및 시뮬레이션 활성화 (생성자에서 이동)
// ========================================
// 생성자에서 물리 관련 함수 호출 시 CDO 문제 발생:
// - SetSimulatePhysics() → 물리 엔진 등록 시도
// - SetMassOverrideInKg(), SetLinearDamping(), SetAngularDamping()
//   → 내부적으로 GetSimplePhysicalMaterial() 호출 → GEngine 필요
// BeginPlay()는 실제 월드에 스폰될 때만 호출되므로 안전
	if (MeshComponent)
	{
		// 물리 속성 설정
		MeshComponent->SetMassOverrideInKg(NAME_None, 0.5f);  // Light weight (0.5kg)
		MeshComponent->SetLinearDamping(2.0f);   // Damping to stop rolling quickly
		MeshComponent->SetAngularDamping(2.0f);  // Angular damping to stop spinning

		// 물리 시뮬레이션 활성화
		MeshComponent->SetSimulatePhysics(true);
	}


	if (HasAuthority())
	{
		//CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnOverlapBegin);
		MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnOverlapBegin);


		// StaticItemInfo가 설정되어 있는데 ItemInformation이 초기화 안된 경우
		// (에디터에서 배치된 아이템 등, Spawn() 함수가 호출되지 않은 경우)
		if (StaticItemInfo && ItemInformation.ItemID == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("🔧 BeginPlay에서 ItemInformation 초기화 (Spawn 미호출)"));

			ItemInformation.ItemID = StaticItemInfo->ItemID;
			ItemInformation.ItemType = StaticItemInfo->ItemType;
			ItemInformation.Quantity = StaticItemInfo->SpawnQuntity;

			UE_LOG(LogTemp, Log, TEXT("  ✅ 초기화 완료: ItemID=%d, Type=%d, Quantity=%d"),
				ItemInformation.ItemID, (int32)ItemInformation.ItemType, ItemInformation.Quantity);
		}
	}
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ========================================
// 리플리케이션 설정
// ========================================
void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemActor, bPoolActive);
	DOREPLIFETIME(AItemActor, ItemInformation);
}

// ========================================
// 클라이언트에서 아이템 정보 수신 시 호출
// StaticItemInfo를 로드하여 UI에서 사용 가능하게 함
// ========================================
void AItemActor::OnRep_ItemInformation()
{
	// 클라이언트에서 StaticItemInfo 로드
	if (ItemInformation.ItemID != 0 && !StaticItemInfo)
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			UItemResourceManager* ResourceManager = GI->GetSubsystem<UItemResourceManager>();
			if (ResourceManager)
			{
				StaticItemInfo = ResourceManager->GetStaticItem(ItemInformation.ItemType, ItemInformation.ItemID);

				// 메시와 스케일도 적용
				if (StaticItemInfo && StaticItemInfo->WorldMesh && MeshComponent)
				{
					MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);
					MeshComponent->SetWorldScale3D(StaticItemInfo->WorldMeshScale);
				}

				UE_LOG(LogTemp, Log, TEXT("[ItemActor::OnRep_ItemInformation] Client loaded StaticItemInfo: ID=%d, Type=%d"),
					ItemInformation.ItemID, (int32)ItemInformation.ItemType);
			}
		}
	}
}

// ========================================
// 클라이언트에서 풀 활성화 상태 변경 시 호출
// ========================================
void AItemActor::OnRep_PoolActive()
{
	if (bPoolActive)
	{
		// 활성화
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		SetActorTickEnabled(true);

		if (MeshComponent)
		{
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
	else
	{
		// 비활성화
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);

		if (MeshComponent)
		{
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

// ========================================
// 풀 활성화 상태 설정 (서버에서 호출)
// ========================================
void AItemActor::SetPoolActive(bool bActive)
{
	if (HasAuthority())
	{
		bPoolActive = bActive;

		// 서버에서도 즉시 적용 (OnRep은 클라이언트에서만 호출됨)
		OnRep_PoolActive();
	}
}

void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (StaticItemInfo && StaticItemInfo->WorldMesh)
	{
		MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);

		MeshComponent->SetWorldScale3D(StaticItemInfo->WorldMeshScale);

		UE_LOG(LogTemp, Log, TEXT("OnConstruction: StaticItemInfo 로드 완료 (ItemID=%d)"), StaticItemInfo->ItemID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnConstruction: StaticItemInfo 또는 WorldMesh가 null!"));
	}
}

// Initialize item with dynamic data
// 동적 아이템 데이터로 초기화
void AItemActor::InitializeItemBS_Item(FBS_Item& _item)
{
	// Set dynamic item data (quantity, durability, etc.)
	// 동적 아이템 데이터 설정 (수량, 내구도 등)
	ItemInformation = _item;

	// Load static item data from ItemResourceManager Subsystem
	// ItemResourceManager 서브시스템에서 정적 아이템 데이터 로드
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		UItemResourceManager* ResourceManager = GI->GetSubsystem<UItemResourceManager>();
		if (ResourceManager)
		{
			// Get static data (mesh, icon, description, etc.)
			// 정적 데이터 가져오기 (메시, 아이콘, 설명 등)
			StaticItemInfo = ResourceManager->GetStaticItem(_item.ItemType, _item.ItemID);

			if (StaticItemInfo)
			{
				// Update mesh if available
				// 메시가 있으면 적용
				if (StaticItemInfo->WorldMesh && MeshComponent)
				{
					MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);

					// 스케일 설정 (버그 수정: 아이템 크기 줄어듦 문제)
					// OnConstruction은 풀링된 아이템에서 호출 안 되므로 여기서 설정
					MeshComponent->SetWorldScale3D(StaticItemInfo->WorldMeshScale);
				}

				// Set LifeSpan to 0 for dropped items (infinite - stay forever)
				// 바닥에 버린 아이템은 무제한 (0 = 영구)
				SetLifeSpan(0.0f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ItemActor::InitializeItemBS_Item - Failed to find static data for item type %d, ID %d"),
					static_cast<uint8>(_item.ItemType), _item.ItemID);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ItemActor::InitializeItemBS_Item - ItemResourceManager Subsystem not found!"));
		}
	}
}

FBS_Item& AItemActor::GetItemData()
{
	return ItemInformation; 
}

void AItemActor::OnPickup(AActor* PickupActor)
{
	if (!HasAuthority())
		return;

	LOG_LOGIC_INFO(TEXT("Item %s picked up by %s"), *(StaticItemInfo->DisplayName.ToString()), *PickupActor->GetName());

	// NOTE: This function is currently not called anywhere
	// Pickup is handled by PickUpDetectorComponent -> InventoryComponent::AddItem()
	// DeSpawn() is called inside AddItem() when quantity reaches 0
}

void AItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 	if (!HasAuthority())
		return;

	// Check if overlapping actor is a player
	if (OtherActor && OtherActor->IsA(ABSCharacterPlayer::StaticClass()))
	{
		//test if it's player controller
		APawn* Pawn = Cast<APawn>(OtherActor);

		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			// Find InventoryComponent and add item
			if (UInventoryComponent* InvComp = Pawn->FindComponentByClass<UInventoryComponent>())
			{
				//InvComp->AddItem(this);  // Commented out: pickup handled by key press, not automatic
				UE_LOG(LogTemp, Log, TEXT("[ItemActor] Player overlapping item: ItemID=%d, Quantity=%d"),
					ItemInformation.ItemID, ItemInformation.Quantity);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ItemActor] Player has no InventoryComponent!"));
			}
		}
	}
}

