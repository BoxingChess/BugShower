// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/WeaponActor.h"
#include "Weapon/Component/WeaponComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// ========================================
	// 네트워크 복제 설정 (멀티플레이어 필수!)
	// ========================================
	bReplicates = true;  // 이 액터를 모든 클라이언트에 복제
	SetReplicateMovement(true);  // 위치/회전도 동기화

	// 충돌 컴포넌트 생성 (픽업 감지용)
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->InitSphereRadius(50.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);  // LineTrace 감지용

	// 무기 메쉬 컴포넌트 생성 (Skeletal Mesh)
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	// 물리 시뮬레이션 활성화 (땅에 떨어질 때 사용)
	WeaponMesh->SetSimulatePhysics(false); // 기본값은 false, 떨어뜨릴 때 true로 변경
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // CollisionComponent만 사용

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

///// 에디터에서 장착 중일 때 Muzzle 위치 실시간 시각화
//#if WITH_EDITOR
//	if (bIsEquipped && WeaponData && WeaponMesh && GetWorld())
//	{
//		FVector MuzzleLoc;
//		FRotator MuzzleRot;
//
//		// 소켓이 있으면 소켓의 Transform (위치 + 회전) 사용
//		if (WeaponMesh->DoesSocketExist(WeaponData->MuzzleSocketName))
//		{
//			FTransform SocketTransform = WeaponMesh->GetSocketTransform(WeaponData->MuzzleSocketName);
//			MuzzleLoc = SocketTransform.GetLocation();
//			MuzzleRot = SocketTransform.Rotator();
//
//			// 소켓의 3축 방향 시각화 (좌표계 확인용)
//			FVector Forward = SocketTransform.GetRotation().GetForwardVector();   // X축 (빨강)
//			FVector Right = SocketTransform.GetRotation().GetRightVector();       // Y축 (초록)
//			FVector Up = SocketTransform.GetRotation().GetUpVector();             // Z축 (파랑)
//
//			// X축 (Forward) - 빨강 - 총알이 나가는 방향
//			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (Forward * 30.0f), FColor::Red, false, 0.1f, 0, 3.0f);
//			// Y축 (Right) - 초록
//			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (Right * 15.0f), FColor::Green, false, 0.1f, 0, 2.0f);
//			// Z축 (Up) - 파랑
//			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (Up * 15.0f), FColor::Blue, false, 0.1f, 0, 2.0f);
//
//			// 소켓 위치 표시 (노란색 구체)
//			DrawDebugSphere(GetWorld(), MuzzleLoc, 5.0f, 12, FColor::Yellow, false, 0.1f, 0, 2.0f);
//		}
//		else
//		{
//			// 소켓 없으면 MuzzleOffset 사용 (메쉬 Scale 적용!)
//			FVector WeaponLoc = WeaponMesh->GetComponentLocation();
//			FRotator WeaponRot = WeaponMesh->GetComponentRotation();
//			FVector WeaponScale = WeaponMesh->GetComponentScale();
//
//			// MuzzleOffset에 Scale 적용
//			FVector ScaledOffset = WeaponData->MuzzleOffset * WeaponScale;
//			MuzzleLoc = WeaponLoc + WeaponRot.RotateVector(ScaledOffset);
//			MuzzleRot = WeaponRot;
//
//			// Offset 방식: 빨간 구체 + 무기 방향으로 화살표
//			DrawDebugSphere(GetWorld(), MuzzleLoc, 5.0f, 12, FColor::Red, false, 0.1f, 0, 2.0f);
//			FVector ForwardDir = WeaponRot.RotateVector(FVector::ForwardVector);
//			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (ForwardDir * 30.0f), FColor::Blue, false, 0.1f, 0, 2.0f);
//		}
//	}
//#endif
}

void AWeaponActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// WeaponData가 설정되어 있으면 메쉬만 적용 (Transform은 건드리지 않음)
	// 월드에 배치할 때는 WeaponActor 자체의 Transform 유지
	if (WeaponData && WeaponData->WeaponMesh)
	{
		WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMesh);
		// Transform은 적용하지 않음 - 에디터에서 설정한 값 유지
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponActor::OnConstruction - WeaponData or WeaponMesh is null"));
	}

	// 에디터 뷰포트에서 소켓 위치 시각화 (BP_TestWeapon 뷰포트에서 보임)
#if WITH_EDITOR
	if (WeaponData && WeaponMesh && GetWorld())
	{
		// 이전 디버그 라인 모두 삭제 (중복 방지)
		FlushPersistentDebugLines(GetWorld());

		FVector MuzzleLoc;

		// 소켓이 있으면 소켓의 Transform 시각화
		if (WeaponMesh->DoesSocketExist(WeaponData->MuzzleSocketName))
		{
			FTransform SocketTransform = WeaponMesh->GetSocketTransform(WeaponData->MuzzleSocketName);
			MuzzleLoc = SocketTransform.GetLocation();

			// 소켓의 3축 좌표계 시각화
			FVector Forward = SocketTransform.GetRotation().GetForwardVector();   // X축 (빨강)
			FVector Right = SocketTransform.GetRotation().GetRightVector();       // Y축 (초록)
			FVector Up = SocketTransform.GetRotation().GetUpVector();             // Z축 (파랑)

			// X축 (Forward) - 빨강 - 총알이 나가는 방향이어야 함
			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (Forward * 30.0f), FColor::Red, true, -1.0f, 0, 3.0f);
			// Y축 (Right) - 초록
			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (Right * 15.0f), FColor::Green, true, -1.0f, 0, 2.0f);
			// Z축 (Up) - 파랑
			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (Up * 15.0f), FColor::Blue, true, -1.0f, 0, 2.0f);

			// 소켓 위치 표시 (노란색 구체)
			DrawDebugSphere(GetWorld(), MuzzleLoc, 5.0f, 12, FColor::Yellow, true, -1.0f, 0, 2.0f);
		}
		else
		{
			// 소켓 없으면 MuzzleOffset 위치 표시 (메쉬 Scale 적용!)
			FVector WeaponLoc = WeaponMesh->GetComponentLocation();
			FRotator WeaponRot = WeaponMesh->GetComponentRotation();
			FVector WeaponScale = WeaponMesh->GetComponentScale();

			// MuzzleOffset에 Scale 적용
			FVector ScaledOffset = WeaponData->MuzzleOffset * WeaponScale;
			MuzzleLoc = WeaponLoc + WeaponRot.RotateVector(ScaledOffset);

			// 빨간 구체 + 무기 방향
			DrawDebugSphere(GetWorld(), MuzzleLoc, 5.0f, 12, FColor::Red, true, -1.0f, 0, 2.0f);
			FVector ForwardDir = WeaponRot.RotateVector(FVector::ForwardVector);
			DrawDebugLine(GetWorld(), MuzzleLoc, MuzzleLoc + (ForwardDir * 30.0f), FColor::Blue, true, -1.0f, 0, 2.0f);
		}
	}
#endif
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

	// 메쉬 적용 (Transform은 적용하지 않음 - AttachToCharacter에서 적용됨)
	if (WeaponData->WeaponMesh && WeaponMesh)
	{
		WeaponMesh->SetSkeletalMesh(WeaponData->WeaponMesh);
		// Transform은 적용하지 않음 - 월드 배치 시에는 WeaponActor 자체의 Transform 유지
		// 장착 시에만 AttachToCharacter()에서 WeaponData의 Transform 적용
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

	// 이전 위치에 남아있던 디버그 라인 모두 삭제 (무기를 주울 때)
#if WITH_EDITOR
	if (GetWorld())
	{
		FlushPersistentDebugLines(GetWorld());
	}
#endif

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

	// 장착 시 충돌 비활성화 (픽업 불가능)
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	// 분리 시 충돌 활성화 (다시 픽업 가능)
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// 물리 시뮬레이션 활성화 (땅에 떨어짐)
	// WeaponMesh는 NoCollision 유지 (CollisionComponent만 사용)
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bIsEquipped = false;

	UE_LOG(LogTemp, Log, TEXT("WeaponActor::DetachFromCharacter - Weapon detached and dropped"));
}
