// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/WeaponActor.h"
#include "Weapon/Component/WeaponComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 무기 메쉬 컴포넌트 생성 (Skeletal Mesh)
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	// 물리 시뮬레이션 활성화 (땅에 떨어질 때 사용)
	WeaponMesh->SetSimulatePhysics(false); // 기본값은 false, 떨어뜨릴 때 true로 변경
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 무기 컴포넌트 생성 (발사, 재장전 로직)
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));

	// 초기 상태
	bIsEquipped = false;
	CurrentAmmo = 0;
	ReserveAmmo = 0;
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// WeaponData가 설정되어 있으면 메쉬 적용
	if (WeaponData && WeaponData->WeaponMesh)
	{
		WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMesh);

		// WeaponData의 Transform 적용
		WeaponMesh->SetRelativeLocation(WeaponData->MeshOffset);
		WeaponMesh->SetRelativeRotation(WeaponData->MeshRotation);
		WeaponMesh->SetRelativeScale3D(WeaponData->MeshScale);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponActor::OnConstruction - WeaponData or WeaponMesh is null"));
	}
}

void AWeaponActor::InitializeWeapon(UWeaponDataAsset* InWeaponData, int32 InCurrentAmmo, int32 InReserveAmmo)
{
	if (!InWeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponActor::InitializeWeapon - InWeaponData is null!"));
		return;
	}

	WeaponData = InWeaponData;

	// 탄약 설정 (0이면 만탄으로 시작)
	CurrentAmmo = (InCurrentAmmo > 0) ? InCurrentAmmo : WeaponData->MagSize;
	ReserveAmmo = InReserveAmmo;

	// 메쉬 적용
	if (WeaponData->WeaponMesh && WeaponMesh)
	{
		WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMesh);

		// WeaponData의 Transform 적용
		WeaponMesh->SetRelativeLocation(WeaponData->MeshOffset);
		WeaponMesh->SetRelativeRotation(WeaponData->MeshRotation);
		WeaponMesh->SetRelativeScale3D(WeaponData->MeshScale);
	}

	// WeaponComponent에 WeaponData 전달
	if (WeaponComponent)
	{
		WeaponComponent->EquipWeapon(WeaponData, ReserveAmmo);
		// WeaponComponent의 탄약도 동기화
		// (WeaponComponent 내부에서 CurrentAmmo와 ReserveAmmo를 관리하므로)
	}

	UE_LOG(LogTemp, Log, TEXT("WeaponActor::InitializeWeapon - Initialized %s with %d/%d ammo"),
		*WeaponData->WeaponName.ToString(), CurrentAmmo, ReserveAmmo);
}

void AWeaponActor::AttachToCharacter(ACharacter* Character, FName SocketName)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponActor::AttachToCharacter - Character is null!"));
		return;
	}

	// 캐릭터의 메쉬에 소켓이 있는지 확인
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!CharacterMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponActor::AttachToCharacter - Character mesh is null!"));
		return;
	}

	if (!CharacterMesh->DoesSocketExist(SocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponActor::AttachToCharacter - Socket '%s' does not exist on character mesh! Creating default attachment."),
			*SocketName.ToString());
	}

	// 무기를 캐릭터 손에 부착
	// Location/Rotation은 소켓에 맞추되, Scale은 WeaponData에 설정된 값 유지
	// KeepRelative: 부모(소켓)의 스케일 영향을 받지 않고 상대 스케일 유지
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget,  // Location - 소켓 위치에 스냅
		EAttachmentRule::SnapToTarget,  // Rotation - 소켓 회전에 스냅
		EAttachmentRule::KeepRelative,  // Scale - 상대 스케일 유지 (WeaponData의 MeshScale 값 유지)
		false
	);
	AttachToComponent(CharacterMesh, AttachRules, SocketName);

	// 부착 후 WeaponData의 Transform 다시 적용 (소켓 스냅으로 인해 초기화되었을 수 있음)
	if (WeaponData)
	{
		WeaponMesh->SetRelativeLocation(WeaponData->MeshOffset);
		WeaponMesh->SetRelativeRotation(WeaponData->MeshRotation);
		WeaponMesh->SetRelativeScale3D(WeaponData->MeshScale);
	}

	// 물리 시뮬레이션 비활성화 (손에 들고 있을 때)
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bIsEquipped = true;

	UE_LOG(LogTemp, Log, TEXT("WeaponActor::AttachToCharacter - Weapon attached to %s at socket %s"),
		*Character->GetName(), *SocketName.ToString());
}

void AWeaponActor::DetachFromCharacter()
{
	// 캐릭터에서 분리
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	DetachFromActor(DetachRules);

	// 물리 시뮬레이션 활성화 (땅에 떨어짐)
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	bIsEquipped = false;

	UE_LOG(LogTemp, Log, TEXT("WeaponActor::DetachFromCharacter - Weapon detached and dropped"));
}
