// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BSCharacterPlayer.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

#include "Component/Movement/MovementInputComponent.h"					// 이동관련 컴포넌트
#include "Component/PickUp/PickUpDetectorComponent.h"					//	픽업 관련 컴포넌트
#include "Component/Inventory/InventoryComponent.h"						//	인벤토리 관련 컴포넌트
#include "Component/Stat/PlayerStatComponent.h"							// 스탯 컴포넌트
#include "Manager/UIManager/BSUIManager.h"								// UI 관리자
#include "Game/BSGameInstance.h"										// Game Instance
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h" // 아이템 리소스 매니저
#include "Item/BSItemInstance.h"										// 아이템 인스턴스

#include "DrawDebugHelpers.h" // ����� ���ο�
#include "Kismet/GameplayStatics.h" // ��������
// #include "Projectile/Grenade.h"  // 주석처리: Projectile 파일 없음
#include "Weapon/WeaponActor.h"
#include "Weapon/Component/WeaponComponent.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Game/BSGameModeBase.h"
#include "Animation/BSAnimInstance.h"									// 애니메이션 인스턴스
#include "Item/ItemEnum.h"												// 아이템 ID enum


ABSCharacterPlayer::ABSCharacterPlayer()
{
	//�޽��� ��ġ�� ȸ���� �����Ѵ�. - ���� ��ǥ�� �ƴ� ĸ�� ������Ʈ ������ ��� ��ġ -> �׳� �� ��� ���߿� �߱� ����
//�𸮾��� �⺻ �޽����� �������� ������ ���⶧���� -90���� �����־���
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));

	//�޽��� � ������� �ִϸ��̼��� ����� ������ ����, AnimationBlueprint - �޽ð� �ִϸ��̼� ��������Ʈ(UAnimInstance)�� ���
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	//ĳ������ SkeletalMeshComponent�� �浹 ������ "CharacterMesh"��� �̸��� �浹 �������Ϸ� �����Ѵٴ� ��. -> ������ ��ǿ� ����
	//��, ĳ������ ���̷�Ż �޽ÿ� �浹 ������ CharacterMesh�� �����Ѵٴ� �� �� ���ο� �ִ� ECC_Visibility ä�ο� ���� Block, Ignore, Overlap �� ���� ó���� �������� �Ǵ�
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

	//Playerĳ������ �޽����̷��� ������Ʈ�� ������Ʈ ���ش�.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/SciFiSoldier03/Meshes/SK_SciFiSoldier03.SK_SciFiSoldier03'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	// BS_Elegg AnimBP 사용
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(TEXT("/Game/Animation/BS_EleggAnimBP.BS_EleggAnimBP_C"));
	if (AnimBPClass.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
	}

	///������Ʈ ����--------------------------------------------------------------------------------------------------------------------------------------------------
	
	//�̵� ���� ������Ʈ ����
	MovementComponentOnGround = CreateDefaultSubobject<UMovementInputComponent>(TEXT("MovementComponentOnGround"));

	//UI ���� ������Ʈ ���� -> �̰� �÷��̾� ������ �����Ѵ�.
	//UIComponent = CreateDefaultSubobject<UUI_InGameComponent>(TEXT("UIComponent"));

	//�ݱ� ���� ������Ʈ
	PickUpDetectorComponent = CreateDefaultSubobject<UPickUpDetectorComponent>(TEXT("PickUpDetectorComponent"));

	//�κ��丮 ������Ʈ
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// 스탯 컴포넌트 (HP, 스태미나, 이동 속도, 점프 등)
	StatComponent = CreateDefaultSubobject<UPlayerStatComponent>(TEXT("StatComponent"));
	



	// ========================================
	// 수류탄 시스템 (주석처리)
	// ========================================

	/*
	//�߻� ����
	static ConstructorHelpers::FObjectFinder<UInputAction> FireActionRef(TEXT("/Game/Input/IA_Fire.IA_Fire"));
	if (FireActionRef.Succeeded())
	{
		FireAction = FireActionRef.Object;
	}

	// Set default projectile class to Grenade
	static ConstructorHelpers::FClassFinder<AGrenade> GrenadeClassRef(TEXT("/Script/BugShower.Grenade"));
	if (GrenadeClassRef.Succeeded())
	{
		ProjectileClass = GrenadeClassRef.Class;
	}
	*/


	// �������� ����
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent); // ĸ���� ����
	SpringArm->TargetArmLength = 400.0f; // ī�޶� �Ÿ�
	SpringArm->bUsePawnControlRotation = true; // ���콺 ȸ���� ���� ȸ��
	SpringArm->SocketOffset = FVector(0.f, 0.f, 100.f); //�ڿ������� ī�޶� ��ġ ����

	// 카메라 암이 Camera 채널로 충돌 검사
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeSize = 12.0f;
	SpringArm->ProbeChannel = ECollisionChannel::ECC_Camera;


	//�Ʒ��� �ε巴�� ������� ī�޶� ���� ������ ���.
	//SpringArm->bEnableCameraRotationLag = true;
	//SpringArm->CameraRotationLagSpeed = 50.0f;

	//3��Ī ī�޶� ����
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPerson_FollowCamera"));
	ThirdPersonCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // �������� ���� ����
	ThirdPersonCamera->bUsePawnControlRotation = false; // ī�޶�� ���� ȸ������ �ʰ�

	// 1��Ī ī�޶� ����
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPerson_EyeCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head")); // 'head' ���� ���� (Skeleton�� ���� �̸� �ٸ� �� ����)
	FirstPersonCamera->bUsePawnControlRotation = true; // ���콺 ȸ���� ���� ȸ��
	FirstPersonCamera->SetActive(false); // �ʱ⿣ ��Ȱ��ȭ

	// bUseControllerRotationYaw:
	// true일 때, 캐릭터의 Yaw 회전(좌우 회전)이 PlayerController의 Rotation을 따라 회전시킨다.
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

	// ========================================
	// Inventory 3D Preview Camera Setup
	// ========================================
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] Constructor START"));

	// Inventory camera arm (for rotation)
	InventoryCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("InventoryCameraArm"));
	if (InventoryCameraArm)
	{
		InventoryCameraArm->SetupAttachment(GetMesh());
		InventoryCameraArm->TargetArmLength = 400.0f; // 400cm distance (same as gameplay camera)
		InventoryCameraArm->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f)); // 90cm up (character chest height)
		InventoryCameraArm->SetRelativeRotation(FRotator(10.0f, 90.0f, 0.0f)); // Pitch=10 (look up slightly), Yaw=90 (front view)
		InventoryCameraArm->bUsePawnControlRotation = false;
		InventoryCameraArm->bInheritPitch = false;
		InventoryCameraArm->bInheritYaw = false;
		InventoryCameraArm->bInheritRoll = false;

		// CRITICAL: Disable collision test so camera doesn't get blocked by character mesh
		InventoryCameraArm->bDoCollisionTest = false;

		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] InventoryCameraArm: Distance=400cm, Height=90cm, Pitch=10, Yaw=90"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY CAMERA] Failed to create InventoryCameraArm"));
	}

	// Inventory SceneCapture2D (설정은 InventoryWidget에서 처리)
	InventoryCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("InventoryCamera"));
	if (InventoryCamera)
	{
		InventoryCamera->SetupAttachment(InventoryCameraArm, USpringArmComponent::SocketName);
		InventoryCamera->bCaptureEveryFrame = false;
		InventoryCamera->bCaptureOnMovement = false;
		InventoryCamera->FOVAngle = 50.0f;
		InventoryCamera->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] InventoryCamera created - FOV: %f"), InventoryCamera->FOVAngle);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY CAMERA] Failed to create InventoryCamera"));
	}


	// bUseControllerRotationPitch:
	// ĳ���Ͱ� Pitch ȸ��(���� ȸ��)�� ��Ʈ�ѷ��� ���� ���� ����.
	// true�� ĳ���� ��ü�� ��/�Ʒ��� �ٶ󺸰� �ǹǷ�, (�� �״�� ĳ���� �޽���ü�� ��/�Ʒ��� ���Եȴ�.)
	// 3��Ī���� false�� �ΰ�, ī�޶� Pitch�� �ݿ��ϴ� ���� �Ϲ����̴�
	bUseControllerRotationPitch = false;

	// ���� ������ ThirdPerson���� �����Ѵ�. (�ʱⰪ)
	CurrentViewMode = ECameraViewMode::ThirdPerson;
	ThirdPersonCamera->SetActive(true);
	FirstPersonCamera->SetActive(false);



	//ĳ���� ������Ʈ ����
	CharacterState = ECharacterState::Normal;

	// 무기 시스템 초기화
	CurrentWeapon = nullptr;
	DefaultWeaponData = nullptr;
}

// ========================================
// 네트워크 리플리케이션
// ========================================

void ABSCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// CurrentWeapon 리플리케이션
	DOREPLIFETIME(ABSCharacterPlayer, CurrentWeapon);
}

/**
 * CurrentWeapon 복제 시 호출되는 콜백
 * 클라이언트에서 무기가 복제되었을 때 무기 장착 상태를 업데이트하고
 * 무기를 캐릭터 손에 부착 (Transform 적용)
 */
void ABSCharacterPlayer::OnRep_CurrentWeapon()
{
	// 무기 장착 상태 업데이트
	SetIsArmed(CurrentWeapon != nullptr);

	// 클라이언트에서 무기 부착 처리 (Transform 적용)
	if (CurrentWeapon)
	{
		// 무기의 Owner 설정
		CurrentWeapon->SetOwner(this);

		// 무기를 캐릭터 손에 부착 (Transform 적용됨)
		CurrentWeapon->AttachToCharacter(this, TEXT("hand_r_weapon"));

		UE_LOG(LogTemp, Log, TEXT("[Client] OnRep_CurrentWeapon - Attached weapon: %s"),
			*CurrentWeapon->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[Client] OnRep_CurrentWeapon - Weapon cleared"));
	}
}

/**
 * BeginPlay - 게임 시작 시 호출
 *
 * 테스트용 무기 자동 장착:
 * 1. TestWeaponData가 설정되어 있는지 확인
 * 2. WeaponActor 스폰
 * 3. InitializeWeapon()으로 DataAsset 설정
 * 4. EquipWeapon()으로 캐릭터에 장착
 */
void ABSCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - Character spawned"));

	// ========================================
	// Inventory Camera: Add Player Mesh to ShowOnlyList
	// ========================================
	UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] BeginPlay - Adding Mesh to ShowOnlyList"));

	if (!InventoryCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY CAMERA] InventoryCamera is NULL in BeginPlay"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] InventoryCamera found: %s"), *InventoryCamera->GetName());
	}

	if (!GetMesh())
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY CAMERA] GetMesh() returned NULL"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] Mesh found: %s"), *GetMesh()->GetName());
		if (GetMesh()->GetSkeletalMeshAsset())
		{
			UE_LOG(LogTemp, Warning, TEXT("[INVENTORY CAMERA] SkeletalMeshAsset: %s"), *GetMesh()->GetSkeletalMeshAsset()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[INVENTORY CAMERA] SkeletalMeshAsset is NULL"));
		}
	}

	// InventoryCamera 설정은 InventoryWidget에서 처리

	// ========================================
	// 스탯 컴포넌트 초기화
	// ========================================
	if (StatComponent)
	{
		// 플레이어 초기 스탯 설정
		// HP: 100, 에블라 입자: 100, 걷기 속도: 600, 달리기 속도: 900, 더블 점프, 점프력: 600
		StatComponent->InitializeStats(100.f, 100.f, 600.f, 900.f, 2, 600.f);

		// HP 변경 이벤트 바인딩 (UI 업데이트용)
		StatComponent->OnHPChanged.AddDynamic(this, &ABSCharacterPlayer::OnPlayerHPChanged);

		// 에블라 입자 변경 이벤트 바인딩 (UI 업데이트용)
		StatComponent->OnAblaParticleChanged.AddDynamic(this, &ABSCharacterPlayer::OnPlayerAblaParticleChanged);

		// 사망 이벤트 바인딩
		StatComponent->OnPlayerDeath.AddDynamic(this, &ABSCharacterPlayer::OnPlayerDied);

		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - StatComponent initialized"));
	}

	// ========================================
	// 인벤토리 컴포넌트 이벤트 바인딩
	// ========================================
	if (InventoryComponent)
	{
		// 아이템 사용 이벤트 바인딩 (아이템 효과 적용용)
		InventoryComponent->OnItemUsed.AddDynamic(this, &ABSCharacterPlayer::OnItemUsed);

		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - InventoryComponent event binding complete"));
	}

	// 기본 무기 자동 장착 (bAutoEquipDefaultWeapon이 true이고 DefaultWeaponData가 설정되어 있을 때만)
	if (bAutoEquipDefaultWeapon && DefaultWeaponData)
	{
		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - DefaultWeaponData is set, spawning default weapon..."));

		// WeaponActor 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AWeaponActor* DefaultWeapon = GetWorld()->SpawnActor<AWeaponActor>(
			AWeaponActor::StaticClass(),
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams
		);

		if (DefaultWeapon)
		{
			// WeaponData 설정 (탄창 30발, 예비 탄약 90발)
			DefaultWeapon->InitializeWeapon(DefaultWeaponData, 30, 90);

			// 캐릭터에 장착
			EquipWeapon(DefaultWeapon);

			UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - Default weapon equipped successfully!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ABSCharacterPlayer::BeginPlay - Failed to spawn default weapon!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - No TestWeaponData set, starting with empty hands"));
	}

	// ========================================
	// Load Selected Items from Game Instance (Client-Initiated)
	// ========================================
	// 클라이언트가 자신의 GameInstance 확인 후 서버로 전송
	UBSGameInstance* GameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (GameInstance && InventoryComponent)
	{
		const TArray<UBSItemInstance*>& SelectedItems = GameInstance->GetSelecedItems();

		if (SelectedItems.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[Client] ABSCharacterPlayer::BeginPlay - Found %d selected items in GameInstance"),
				SelectedItems.Num());

			// Convert UBSItemInstance array to FBS_Item array for RPC
			TArray<FBS_Item> ItemDataArray;
			for (UBSItemInstance* ItemInstance : SelectedItems)
			{
				if (ItemInstance)
				{
					ItemDataArray.Add(ItemInstance->Dynamic);
				}
			}

			// Send to server via RPC
			if (ItemDataArray.Num() > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("[Client] Sending %d items to server via RPC"), ItemDataArray.Num());
				ServerLoadSelectedItems(ItemDataArray);

				// Clear selected items from client's GameInstance after sending
				GameInstance->ClearSelectedItems();
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[Client] ABSCharacterPlayer::BeginPlay - No selected items in GameInstance"));
		}
	}
	else
	{
		if (!GameInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Client] ABSCharacterPlayer::BeginPlay - GameInstance is null!"));
		}
		if (!InventoryComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Client] ABSCharacterPlayer::BeginPlay - InventoryComponent is null!"));
		}
	}
}

void ABSCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Initing..."));
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// MovementComponent 입력 바인딩
	MovementComponentOnGround->FL_SetupPlayerInputComponent(PlayerInputComponent);

	// EnhancedInputComponent로 캐스팅
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogTemp, Error, TEXT("SetupPlayerInputComponent - Failed to cast to EnhancedInputComponent!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SetupPlayerInputComponent - EnhancedInput successfully casted"));

	// ========================================
	// 무기 입력 바인딩
	// ========================================

	// 무기 발사 (좌클릭)
	// Started: 마우스 버튼 누르는 순간 → StartFireWeapon() 호출
	// Completed: 마우스 버튼 떼는 순간 → StopFireWeapon() 호출
	if (IA_FireWeapon)
	{
		EnhancedInput->BindAction(IA_FireWeapon, ETriggerEvent::Started, this, &ABSCharacterPlayer::StartFireWeapon);
		EnhancedInput->BindAction(IA_FireWeapon, ETriggerEvent::Completed, this, &ABSCharacterPlayer::StopFireWeapon);
		UE_LOG(LogTemp, Log, TEXT("SetupPlayerInputComponent - IA_FireWeapon bound (Started/Completed)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent - IA_FireWeapon is not set! Please set it in Blueprint."));
	}

	// 무기 재장전 (R키)
	// Triggered: 키를 누르는 순간 → ReloadWeapon() 호출
	if (IA_ReloadWeapon)
	{
		EnhancedInput->BindAction(IA_ReloadWeapon, ETriggerEvent::Triggered, this, &ABSCharacterPlayer::ReloadWeapon);
		UE_LOG(LogTemp, Log, TEXT("SetupPlayerInputComponent - IA_ReloadWeapon bound (Triggered)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent - IA_ReloadWeapon is not set! Please set it in Blueprint."));
	}

	// ========================================
	// 수류탄 입력 바인딩 (주석처리)
	// ========================================

	/*
	// 수류탄 발사 (우클릭)
	if (FireAction)
	{
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ABSCharacterPlayer::Fire);
		UE_LOG(LogTemp, Log, TEXT("SetupPlayerInputComponent - FireAction (Grenade) bound"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent - FireAction (Grenade) is not set!"));
	}
	*/
}

void ABSCharacterPlayer::SetCurrentViewMode(ECameraViewMode _newMode)
{
	CurrentViewMode = _newMode;
}

void ABSCharacterPlayer::SetCharacterState(ECharacterState _newState)
{
	CharacterState = _newState;
}

ECameraViewMode ABSCharacterPlayer::GetCurrentViewMode()
{
	return CurrentViewMode;
}

ECharacterState ABSCharacterPlayer::GetCharacterState()
{
	return CharacterState;
}

void ABSCharacterPlayer::ToggleViewMode()
{
	if (CurrentViewMode == ECameraViewMode::FirstPerson)
	{
		SetCameraViewMode(ECameraViewMode::ThirdPerson);
	}
	else
	{
		SetCameraViewMode(ECameraViewMode::FirstPerson);
	}
}

void ABSCharacterPlayer::SetCameraViewMode(ECameraViewMode NewMode)
{
	CurrentViewMode = NewMode;
	// ī�޶� ��ȯ
	if (NewMode == ECameraViewMode::FirstPerson)
	{
		FirstPersonCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);

		// �޽� ����� ����
		//GetMesh()->HideBoneByName("head", EPhysBodyOp::PBO_None);
		GetMesh()->SetOwnerNoSee(true);

		//�׷��� �׸��ڴ� �׸��� �ؾ��Ѵ�.
		GetMesh()->SetCastShadow(true);		//�� �޽��� �׸��ڸ� ������ΰ�?
		GetMesh()->bCastHiddenShadow = true; // �޽��� ī�޶� ������ �ʾƵ� �׸��ڸ� ������ ���ΰ�?

	}
	else
	{
		ThirdPersonCamera->SetActive(true);
		FirstPersonCamera->SetActive(false);

		//GetMesh()->UnHideBoneByName("head");
		GetMesh()->SetOwnerNoSee(false);

	}
}

void ABSCharacterPlayer::StartThirdPersonZoom()
{
	ThirdPersonCamera->SetFieldOfView(85.0f);
	SpringArm->TargetArmLength = 350.0f;
	// �ΰ����� ���̷��� ���� ���� �ΰ� ����

	// ���� ��� ī�޶� ��ȯ ��
	SpringArm->SocketOffset = FVector(0.f, 100.f, 100.f); //ī�޶� ĳ���� ���������� �ű��.
}

void ABSCharacterPlayer::EndThirdPersonZoom()
{
	ThirdPersonCamera->SetFieldOfView(90.0f);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 100.f); //�ڿ������� ī�޶� ��ġ ����
}


// ĳ���Ͱ� ���ο� ��Ʈ�ѷ��� ���� ����(Possess)�� �� ȣ��Ǵ� �Լ�.
// ���������� ȣ���.
// NewController�� �ٴ� �����̹Ƿ�, MovementComponent ������ OwnerController �����͸� �������ش�.
// �̷��� �ϸ� ��Ʈ�ѷ��� ��ü�Ǿ��� ��(��: UI ���� ��Ʈ�ѷ��� ����) �Է� ó�� ������ ��� �ݿ��� �� �ִ�.
void ABSCharacterPlayer::PossessedBy(AController* NewController)
{
	
	Super::PossessedBy(NewController);
	if (MovementComponentOnGround)
	{
		MovementComponentOnGround->RefreshControllerCache();
	}
	
}


// Controller ������ ��Ʈ��ũ�� ���� ����(Replicate)�Ǿ� Ŭ���̾�Ʈ���� ����Ǿ��� �� ȣ��Ǵ� �Լ�.
// ��Ƽ�÷��� ȯ�濡�� Ŭ���̾�Ʈ�� ����ó�� PossessedBy()�� ȣ����� �ʱ� ������,
// OnRep_Controller()�� ���� ������ ���� �۾��� �����ؾ� �Ѵ�.
// ���⼭�� MovementComponent�� OwnerController�� ���� �����Ͽ�
// UI ���� ��Ʈ�ѷ��� �پ��� �� �Է��� ���ų�, ���ο� ��Ʈ�ѷ��� �´� ������ ������ �� �ִ�.
void ABSCharacterPlayer::OnRep_Controller()
{
	
	Super::OnRep_Controller();
	if (MovementComponentOnGround)
	{
		MovementComponentOnGround->RefreshControllerCache();
	}
	
}

/*
// ========================================
// 수류탄 발사 함수 (주석처리)
// ========================================

void ABSCharacterPlayer::Fire()
{
	// Check if projectile class is set
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectileClass is not set! Cannot fire."));
		return;
	}

	// Get spawn location (from hand socket or camera)
	FVector SpawnLocation = GetMesh()->GetSocketLocation(TEXT("hand_r"));

	// Get spawn rotation (from camera forward vector)
	FRotator SpawnRotation = ThirdPersonCamera->GetForwardVector().Rotation();

	// Set spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn projectile
	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (Projectile)
	{
		// Initialize projectile velocity
		FVector LaunchDirection = ThirdPersonCamera->GetForwardVector();
		Projectile->InitVelocity(LaunchDirection);

		UE_LOG(LogTemp, Log, TEXT("Projectile fired! Class: %s"), *ProjectileClass->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn projectile!"));
	}

	// OLD LINE TRACE CODE - Keep for reference if needed
	// FVector Start = GetMesh()->GetSocketLocation(TEXT("hand_r"));
	// FVector End = Start + (ThirdPersonCamera->GetForwardVector() * 10000.0f);
	//
	// FCollisionQueryParams Params;
	// Params.AddIgnoredActor(this);
	//
	// FHitResult HitResult;
	// bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	//
	// FColor LineColor = bHit ? FColor::Red : FColor::Green;
	// FVector LineEnd = bHit ? HitResult.ImpactPoint : End;
	// DrawDebugLine(GetWorld(), Start, LineEnd, LineColor, false, 1.0f, 0, 1.0f);
	//
	// if (bHit && HitResult.GetActor())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
	// 	UGameplayStatics::ApplyDamage(HitResult.GetActor(), 10.0f, GetController(), this, nullptr);
	// }
}
*/

// ========================================
// 무기 시스템 구현
// ========================================

/**
 * 무기 장착 서버 RPC
 *
 * 클라이언트가 무기를 주울 때 이 함수를 호출하면
 * 서버에서 EquipWeapon()이 실행되고 자동으로 모든 클라이언트에 리플리케이트됨
 */
void ABSCharacterPlayer::ServerEquipWeapon_Implementation(AWeaponActor* Weapon)
{
	// 서버에서 실제 장착 로직 실행
	EquipWeapon(Weapon);
}

/**
 * 무기 장착 (로컬 실행)
 *
 * 동작:
 * 1. 기존 무기가 있으면 먼저 해제
 * 2. 새 무기를 CurrentWeapon에 저장
 * 3. WeaponActor->AttachToCharacter()로 손에 부착
 * 4. WeaponActor의 Owner를 this(캐릭터)로 설정
 * 5. StatComponent의 bIsArmed를 true로 설정 (자동 리플리케이트)
 *
 * 주의: 데디서버 환경에서는 ServerEquipWeapon을 호출해야 함!
 */
void ABSCharacterPlayer::EquipWeapon(AWeaponActor* Weapon)
{
	// nullptr 체크
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABSCharacterPlayer::EquipWeapon - Weapon is nullptr!"));
		return;
	}

	// 기존 무기가 있으면 먼저 해제
	if (CurrentWeapon)
	{
		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::EquipWeapon - Unequipping old weapon first"));
		UnequipWeapon();
	}

	// 새 무기 설정
	CurrentWeapon = Weapon;

	// WeaponActor의 Owner를 캐릭터로 설정 (먼저 설정해야 WeaponComponent가 올바르게 작동)
	CurrentWeapon->SetOwner(this);

	// 서버/클라이언트 구분
	FString NetRole = HasAuthority() ? TEXT("[Server]") : TEXT("[Client]");

	// WeaponData가 설정되어 있으면 초기화 (픽업한 무기의 경우)
	// 이미 InitializeWeapon이 호출된 경우 (BeginPlay에서 스폰한 무기) 중복 호출되지만 문제없음
	if (UWeaponDataAsset* WeaponData = CurrentWeapon->GetWeaponData())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s ========================================"), *NetRole);
		UE_LOG(LogTemp, Warning, TEXT("%s BSCharacterPlayer::EquipWeapon - START"), *NetRole);
		UE_LOG(LogTemp, Warning, TEXT("%s   Weapon: %s"), *NetRole, *WeaponData->WeaponName.ToString());

		// 인벤토리 상태 확인
		if (InventoryComponent)
		{
			TArray<UBSItemInstance*> Items = InventoryComponent->GetItemInventory();
			UE_LOG(LogTemp, Warning, TEXT("%s   InventoryComponent found: %d items"), *NetRole, Items.Num());
			for (int32 i = 0; i < Items.Num(); i++)
			{
				if (Items[i] && Items[i]->StaticData)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s     [%d] ItemID: %d, Quantity: %d"),
						*NetRole, i, Items[i]->StaticData->ItemID, Items[i]->Dynamic.Quantity);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s   InventoryComponent is NULL!"), *NetRole);
		}

		// 인벤토리에서 탄약 개수 조회
		int32 CurrentAmmo = 0;  // 탄창은 비어있는 상태로 시작
		int32 ReserveAmmo = 0;

		if (InventoryComponent && WeaponData)
		{
			// AmmoType을 ItemID로 변환
			uint8 AmmoItemID = 0;
			switch (WeaponData->AmmoType)
			{
				case EAmmoType::Ammo_9mm:
					AmmoItemID = static_cast<uint8>(EConsumableID::Ammunition_9mm);  // 201
					break;
				case EAmmoType::Ammo_556mm:
					AmmoItemID = static_cast<uint8>(EConsumableID::Ammunition_5_56mm);  // 202
					break;
				case EAmmoType::Ammo_762mm:
					AmmoItemID = static_cast<uint8>(EConsumableID::Ammunition_7_62mm);  // 203
					break;
				default:
					UE_LOG(LogTemp, Warning, TEXT("%s ABSCharacterPlayer::EquipWeapon - Unknown AmmoType: %d"), *NetRole, static_cast<int32>(WeaponData->AmmoType));
					break;
			}

			// 인벤토리에서 해당 ItemID의 총 개수 가져오기
			if (AmmoItemID != 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s   Looking for AmmoItemID: %d"), *NetRole, AmmoItemID);
				ReserveAmmo = InventoryComponent->GetItemCountByID(AmmoItemID);
				UE_LOG(LogTemp, Warning, TEXT("%s   Found ReserveAmmo: %d"), *NetRole, ReserveAmmo);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("%s ========================================"), *NetRole);

		// 무기 초기화 (CurrentAmmo=0, ReserveAmmo=인벤토리 탄약 개수)
		CurrentWeapon->InitializeWeapon(WeaponData, CurrentAmmo, ReserveAmmo);
		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::EquipWeapon - Initialized weapon with %s"), *WeaponData->WeaponName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ABSCharacterPlayer::EquipWeapon - Weapon has no WeaponData!"));
	}

	// WeaponActor를 캐릭터 손에 부착
	// "hand_r_weapon" 소켓이 캐릭터 스켈레톤에 있어야 함 (언리얼 에디터에서 추가 필요)
	CurrentWeapon->AttachToCharacter(this, TEXT("hand_r_weapon"));

	// StatComponent의 bIsArmed를 true로 설정 (AnimInstance가 이 값을 읽어감)
	SetIsArmed(true);

	UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::EquipWeapon - Equipped weapon: %s"), *CurrentWeapon->GetName());
}

/**
 * 무기 해제 서버 RPC
 *
 * 클라이언트가 무기를 버릴 때 이 함수를 호출하면
 * 서버에서 UnequipWeapon()이 실행되고 자동으로 모든 클라이언트에 리플리케이트됨
 */
void ABSCharacterPlayer::ServerUnequipWeapon_Implementation()
{
	// 서버에서 실제 해제 로직 실행
	UnequipWeapon();
}

/**
 * 무기 해제 (로컬 실행)
 *
 * 동작:
 * 1. 현재 무기가 있는지 확인
 * 2. 발사 중지 (혹시라도 발사 중이면)
 * 3. WeaponActor->DetachFromCharacter()로 손에서 분리
 * 4. StatComponent의 bIsArmed를 false로 설정 (자동 리플리케이트)
 * 5. CurrentWeapon을 nullptr로 초기화 (빈손 상태)
 *
 * 참고: WeaponActor는 파괴하지 않고 월드에 떨어뜨림
 *       만약 파괴하려면 CurrentWeapon->Destroy() 호출
 *
 * 주의: 데디서버 환경에서는 ServerUnequipWeapon을 호출해야 함!
 */
void ABSCharacterPlayer::UnequipWeapon()
{
	// 무기가 없으면 무시
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABSCharacterPlayer::UnequipWeapon - No weapon equipped!"));
		return;
	}

	// 혹시라도 발사 중이면 중지
	StopFireWeapon();

	// 무기를 손에서 분리 (땅에 떨어뜨림)
	CurrentWeapon->DetachFromCharacter();

	UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::UnequipWeapon - Unequipped weapon: %s"), *CurrentWeapon->GetName());

	// StatComponent의 bIsArmed를 false로 설정 (빈손 상태)
	SetIsArmed(false);

	// 빈손 상태로
	CurrentWeapon = nullptr;
}

/**
 * 발사 시작 (좌클릭 누름)
 *
 * 동작:
 * 1. 무기가 장착되어 있는지 확인
 * 2. 클라이언트면 Server RPC 호출
 * 3. 서버면 직접 실행 + Multicast로 애니메이션 전파
 *
 * WeaponComponent에서:
 * - 단발: 1발만 발사
 * - 연발: Tick에서 계속 발사 (StopFire 호출 전까지)
 * - 점사: BurstCount만큼만 발사
 */
void ABSCharacterPlayer::StartFireWeapon()
{
	// 무기가 없으면 무시
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ABSCharacterPlayer::StartFireWeapon - No weapon equipped"));
		return;
	}

	// 클라이언트에서는 서버로 전달
	if (!HasAuthority())
	{
		ServerStartFireWeapon();

		// 로컬에서도 카메라/이동 설정 적용 (즉각적인 반응성)
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->DisableMovement();
		return;
	}

	// 서버에서 실행
	UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ABSCharacterPlayer::StartFireWeapon - WeaponComponent is nullptr!"));
		return;
	}

	// 발사 중에는 카메라 방향으로 회전
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// 발사 중에는 이동 불가
	GetCharacterMovement()->DisableMovement();

	// 발사 시작 (서버에서 실행)
	WeaponComp->StartFire();

	// 모든 클라이언트에게 발사 애니메이션 전파
	MulticastPlayFireAnimation();
}

/**
 * 발사 시작 Server RPC 구현
 */
void ABSCharacterPlayer::ServerStartFireWeapon_Implementation()
{
	StartFireWeapon();
}

/**
 * 발사 중지 (좌클릭 뗌)
 *
 * 동작:
 * 1. 무기가 장착되어 있는지 확인
 * 2. 클라이언트면 Server RPC 호출
 * 3. 서버면 직접 실행
 *
 * 연발 무기의 경우 발사를 멈춤
 */
void ABSCharacterPlayer::StopFireWeapon()
{
	// 무기가 없으면 무시
	if (!CurrentWeapon)
	{
		return;
	}

	// 클라이언트에서는 서버로 전달
	if (!HasAuthority())
	{
		ServerStopFireWeapon();

		// 로컬에서도 카메라/이동 설정 복구 (즉각적인 반응성)
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		return;
	}

	// 서버에서 실행
	UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent();
	if (!WeaponComp)
	{
		return;
	}

	// 발사 중지
	WeaponComp->StopFire();

	// 이동 복구
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// 발사 중지 시 이동 방향으로 회전 복귀
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

/**
 * 발사 중지 Server RPC 구현
 */
void ABSCharacterPlayer::ServerStopFireWeapon_Implementation()
{
	StopFireWeapon();
}

/**
 * 재장전 (R키)
 *
 * 동작:
 * 1. 무기가 장착되어 있는지 확인
 * 2. 클라이언트면 Server RPC 호출
 * 3. 서버면 직접 실행 + Multicast로 애니메이션 전파
 *
 * WeaponComponent에서:
 * - 예비 탄약이 있는지 확인
 * - 재장전 중인지 확인
 * - 재장전 타이머 시작 (ReloadTime 후 OnReloadComplete 호출)
 */
void ABSCharacterPlayer::ReloadWeapon()
{
	// 무기가 없으면 무시
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ABSCharacterPlayer::ReloadWeapon - No weapon equipped"));
		return;
	}

	// 클라이언트에서는 서버로 전달
	if (!HasAuthority())
	{
		ServerReloadWeapon();
		return;
	}

	// 서버에서 실행
	UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ABSCharacterPlayer::ReloadWeapon - WeaponComponent is nullptr!"));
		return;
	}

	// 재장전 시작 (서버에서 실행)
	WeaponComp->Reload();

	// 모든 클라이언트에게 재장전 애니메이션 전파
	MulticastPlayReloadAnimation();
}

/**
 * 재장전 Server RPC 구현
 */
void ABSCharacterPlayer::ServerReloadWeapon_Implementation()
{
	ReloadWeapon();
}

/**
 * 발사 애니메이션 Multicast RPC 구현
 * 모든 클라이언트에서 발사 애니메이션 재생
 */
void ABSCharacterPlayer::MulticastPlayFireAnimation_Implementation()
{
	// AnimInstance에서 발사 몽타주 재생
	if (UBSAnimInstance* AnimInst = Cast<UBSAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->PlayFireMontage();
	}
}

/**
 * 재장전 애니메이션 Multicast RPC 구현
 * 모든 클라이언트에서 재장전 애니메이션 재생
 */
void ABSCharacterPlayer::MulticastPlayReloadAnimation_Implementation()
{
	// AnimInstance에서 재장전 몽타주 재생
	if (UBSAnimInstance* AnimInst = Cast<UBSAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->PlayReloadMontage();
	}
}

// ========================================
// 데미지 시스템 구현
// ========================================

/**
 * TakeDamage 오버라이드
 *
 * 언리얼 엔진의 데미지 시스템에서 호출되는 함수
 * Bullet, Grenade 등에서 ApplyDamage를 호출하면 이 함수가 호출됨
 *
 * @param DamageAmount - 받은 데미지 양
 * @param DamageEvent - 데미지 이벤트 정보 (타입, 히트 위치 등)
 * @param EventInstigator - 데미지를 가한 컨트롤러
 * @param DamageCauser - 데미지를 준 액터 (총알, 수류탄 등)
 * @return 실제 적용된 데미지 양
 */
float ABSCharacterPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                      AController* EventInstigator, AActor* DamageCauser)
{
	// 부모 클래스의 TakeDamage 호출
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// StatComponent가 있으면 데미지 적용
	if (StatComponent)
	{
		StatComponent->ApplyDamage(DamageAmount);

		UE_LOG(LogTemp, Log, TEXT("Player TakeDamage: %.1f from %s"),
		       DamageAmount,
		       DamageCauser ? *DamageCauser->GetName() : TEXT("Unknown"));
	}

	return ActualDamage;
}

/**
 * HP 변경 시 호출되는 콜백 함수
 * StatComponent의 OnHPChanged 델리게이트에 바인딩됨
 *
 * @param CurrentHP - 현재 HP
 * @param MaxHP - 최대 HP
 */
void ABSCharacterPlayer::OnPlayerHPChanged(float CurrentHP, float MaxHP)
{
	UE_LOG(LogTemp, Log, TEXT("Player HP Changed: %.1f / %.1f (%.1f%%)"),
	       CurrentHP, MaxHP, (CurrentHP / MaxHP) * 100.0f);

	// 서버에서만 클라이언트에게 UI 업데이트 명령 전송
	if (HasAuthority())
	{
		ClientUpdateHealthUI(CurrentHP, MaxHP);
	}
}

void ABSCharacterPlayer::ClientUpdateHealthUI_Implementation(float CurrentHP, float MaxHP)
{
	// 클라이언트에서만 실행됨 (서버가 호출)
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBSUIManager* UIManager = GameInstance->GetSubsystem<UBSUIManager>())
		{
			UIManager->UpdateHealthUI(CurrentHP, MaxHP);
			UE_LOG(LogTemp, Log, TEXT("[Client] HP UI Updated: %.1f / %.1f"), CurrentHP, MaxHP);
		}
	}
}

/**
 * 에블라 입자 변경 시 호출되는 콜백 함수
 * StatComponent의 OnAblaParticleChanged 델리게이트에 바인딩됨
 *
 * @param CurrentAblaParticle - 현재 에블라 입자
 * @param MaxAblaParticle - 최대 에블라 입자
 */
void ABSCharacterPlayer::OnPlayerAblaParticleChanged(float CurrentAblaParticle, float MaxAblaParticle)
{
	// UE_LOG(LogTemp, Log, TEXT("Player Abla Particle Changed: %.2f / %.2f (%.1f%%)"),
	//        CurrentAblaParticle, MaxAblaParticle, (CurrentAblaParticle / MaxAblaParticle) * 100.0f);

	// 서버에서만 클라이언트에게 UI 업데이트 명령 전송
	if (HasAuthority())
	{
		ClientUpdateAblaParticleUI(CurrentAblaParticle, MaxAblaParticle);
	}
}

void ABSCharacterPlayer::ClientUpdateAblaParticleUI_Implementation(float CurrentAblaParticle, float MaxAblaParticle)
{
	// 클라이언트에서만 실행됨 (서버가 호출)
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBSUIManager* UIManager = GameInstance->GetSubsystem<UBSUIManager>())
		{
			UIManager->UpdateAblaParticleUI(CurrentAblaParticle, MaxAblaParticle);
			// UE_LOG(LogTemp, Log, TEXT("[Client] Abla UI Updated: %.1f / %.1f"), CurrentAblaParticle, MaxAblaParticle);
		}
	}
}

/**
 * 플레이어 사망 시 호출되는 콜백 함수
 * StatComponent의 OnPlayerDeath 델리게이트에 바인딩됨
 */
void ABSCharacterPlayer::OnPlayerDied()
{
	UE_LOG(LogTemp, Warning, TEXT("Player %s has died!"), *GetName());

	// TODO: 사망 처리
	// 1. 입력 비활성화
	// DisableInput(Cast<APlayerController>(GetController()));

	// 2. 사망 애니메이션 재생
	// if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	// {
	//     AnimInstance->Montage_Play(DeathMontage);
	// }

	// 3. 래그돌 활성화 또는 사망 이펙트
	// GetMesh()->SetSimulatePhysics(true);
	// GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	// 4. 리스폰 타이머 시작
	// GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this,
	//     &ABSCharacterPlayer::Respawn, 5.0f, false);

	auto GameMode = GetWorld()->GetAuthGameMode<ABSGameModeBase>();
	if (GameMode)
	{
		GameMode->OnPlayerDied(GetController());
	}

}

/**
 * 아이템 사용 시 호출되는 콜백 함수
 * InventoryComponent의 OnItemUsed 델리게이트로부터 호출됨
 * DataAsset 기반으로 효과 적용
 */
void ABSCharacterPlayer::OnItemUsed(const UBSStaticItemDataAsset* ItemData)
{
	if (!StatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnItemUsed] StatComponent is null!"));
		return;
	}

	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnItemUsed] ItemData is null!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[OnItemUsed] Item used: %s (ID: %d)"),
		*ItemData->DisplayName.ToString(), ItemData->ItemID);

	// ========================================
	// 힐 효과
	// ========================================
	if (ItemData->HealAmount > 0.f)
	{
		StatComponent->Heal(ItemData->HealAmount);
		UE_LOG(LogTemp, Log, TEXT("[OnItemUsed] Healed %.1f HP"), ItemData->HealAmount);
	}

	// ========================================
	// 에블라 정화제 효과 (에블라 감소)
	// ========================================
	if (ItemData->AblaReductionAmount > 0.f)
	{
		float CurrentAbla = StatComponent->GetCurrentAblaParticle();
		float ActualReduction = FMath::Min(CurrentAbla, ItemData->AblaReductionAmount);

		if (ActualReduction > 0.f)
		{
			StatComponent->ConsumeAblaParticle(ActualReduction);
			UE_LOG(LogTemp, Log, TEXT("[OnItemUsed] Reduced Abla Particle by %.1f (%.1f -> %.1f)"),
				ActualReduction, CurrentAbla, CurrentAbla - ActualReduction);
		}
	}

	// ========================================
	// 에블라 억제제 효과 (증가 속도 감소)
	// ========================================
	if (ItemData->AblaGainRateMultiplier < 1.0f)
	{
		const float BaseRate = 0.1f; // 기본 증가율
		float NewRate = BaseRate * ItemData->AblaGainRateMultiplier;
		StatComponent->SetAblaParticleGainRate(NewRate);

		UE_LOG(LogTemp, Log, TEXT("[OnItemUsed] Abla gain rate changed: %.3f (Multiplier: %.2f)"),
			NewRate, ItemData->AblaGainRateMultiplier);

		// 지속 시간이 설정되어 있으면 타이머로 원래대로 복구
		if (ItemData->AblaEffectDuration > 0.f)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[this, BaseRate]()
				{
					if (StatComponent)
					{
						StatComponent->SetAblaParticleGainRate(BaseRate);
						UE_LOG(LogTemp, Log, TEXT("[OnItemUsed] Abla gain rate restored to %.3f"), BaseRate);
					}
				},
				ItemData->AblaEffectDuration,
				false
			);

			UE_LOG(LogTemp, Log, TEXT("[OnItemUsed] Effect will last for %.1f seconds"), ItemData->AblaEffectDuration);
		}
	}
}



// ========================================
// 스탯 Getter 함수 구현 (Coupling 방지)
// ========================================

/**
 * 최대 점프 횟수 가져오기
 *
 * MovementComponent가 StatComponent를 직접 참조하지 않도록
 * BSCharacterPlayer가 중간 레이어 역할을 함
 *
 * 이점:
 * - Loose coupling: MovementComponent는 PlayerStatComponent를 몰라도 됨
 * - Encapsulation: StatComponent 구현을 숨김
 * - Flexibility: 나중에 StatComponent를 다른 걸로 바꿔도 MovementComponent 수정 불필요
 */
int32 ABSCharacterPlayer::GetMaxJumpCount() const
{
	if (StatComponent)
	{
		return StatComponent->GetMaxJumpCount();
	}
	return 2; // 기본값: 더블 점프
}

/**
 * 점프력 가져오기
 */
float ABSCharacterPlayer::GetJumpPower() const
{
	if (StatComponent)
	{
		return StatComponent->GetJumpPower();
	}
	return 600.f; // 기본값
}

/**
 * 걷기 속도 가져오기
 */
float ABSCharacterPlayer::GetWalkSpeed() const
{
	if (StatComponent)
	{
		return StatComponent->GetWalkSpeed();
	}
	return 600.f; // 기본값
}

/**
 * 달리기 속도 가져오기
 */
float ABSCharacterPlayer::GetSprintSpeed() const
{
	if (StatComponent)
	{
		return StatComponent->GetSprintSpeed();
	}
	return 900.f; // 기본값
}

/**
 * 무기 장착 상태 가져오기
 */
bool ABSCharacterPlayer::GetIsArmed() const
{
	if (StatComponent)
	{
		return StatComponent->IsArmed();
	}
	return false; // 기본값: 빈손
}

/**
 * 무기 장착 상태 설정
 */
void ABSCharacterPlayer::SetIsArmed(bool bNewIsArmed)
{
	if (StatComponent)
	{
		StatComponent->SetIsArmed(bNewIsArmed);
	}
}

// ========================================
// 인벤토리 시스템 - 멀티플레이어
// ========================================

/**
 * ServerLoadSelectedItems RPC Implementation
 *
 * 서버에서 실행되며, 클라이언트로부터 받은 선택된 아이템 배열을 인벤토리에 추가
 *
 * 동작:
 * 1. FBS_Item 배열을 받음 (클라이언트의 GameInstance에서 선택한 아이템)
 * 2. ItemResourceManager를 통해 StaticData 찾기
 * 3. UBSItemInstance로 변환
 * 4. InventoryComponent->AddItemInstances() 호출
 * 5. 자동으로 모든 클라이언트에 리플리케이트됨 (InventoryComponent의 Replicated 속성)
 */
void ABSCharacterPlayer::ServerLoadSelectedItems_Implementation(const TArray<FBS_Item>& SelectedItems)
{
	UE_LOG(LogTemp, Log, TEXT("[Server] ServerLoadSelectedItems - Received %d items from client"), SelectedItems.Num());

	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ServerLoadSelectedItems - InventoryComponent is null!"));
		return;
	}

	// Get ItemResourceManager to retrieve StaticData
	UBSGameInstance* GameInstance = Cast<UBSGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ServerLoadSelectedItems - GameInstance is null!"));
		return;
	}

	UItemResourceManager* ItemResourceManager = GameInstance->GetSubsystem<UItemResourceManager>();
	if (!ItemResourceManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ServerLoadSelectedItems - ItemResourceManager not found!"));
		return;
	}

	// Convert FBS_Item array to UBSItemInstance array
	TArray<UBSItemInstance*> ItemInstances;
	for (const FBS_Item& ItemData : SelectedItems)
	{
		// Get StaticData from ItemResourceManager
		const UBSStaticItemDataAsset* StaticData = ItemResourceManager->GetStaticItem(
			ItemData.ItemType, ItemData.ItemID);

		if (StaticData)
		{
			// Create UBSItemInstance
			UBSItemInstance* NewInstance = NewObject<UBSItemInstance>(this);
			NewInstance->StaticData = StaticData;
			NewInstance->Dynamic = ItemData;

			ItemInstances.Add(NewInstance);

			UE_LOG(LogTemp, Log, TEXT("[Server] Created item instance (Type=%d, ID=%d, Quantity=%d)"),
				(int32)ItemData.ItemType, ItemData.ItemID, ItemData.Quantity);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Server] Failed to find StaticData for ItemType=%d, ItemID=%d"),
				(int32)ItemData.ItemType, ItemData.ItemID);
		}
	}

	// Add to inventory (will automatically replicate to all clients)
	if (ItemInstances.Num() > 0)
	{
		InventoryComponent->AddItemInstances(ItemInstances);
		UE_LOG(LogTemp, Log, TEXT("[Server] Successfully added %d items to inventory"), ItemInstances.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] No valid items to add to inventory"));
	}
}
