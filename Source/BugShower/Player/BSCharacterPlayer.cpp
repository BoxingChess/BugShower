// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BSCharacterPlayer.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/Movement/MovementInputComponent.h"					//��ü������Ʈ
#include "Component/PickUp/PickUpDetectorComponent.h"					//��ü������Ʈ
#include "Component/Inventory/InventoryComponent.h"						//��ü������Ʈ

#include "DrawDebugHelpers.h" // ����� ���ο�
#include "Kismet/GameplayStatics.h" // ��������
// #include "Projectile/Grenade.h"  // 주석처리: Projectile 파일 없음
#include "Weapon/WeaponActor.h"
#include "Weapon/Component/WeaponComponent.h"
#include "Weapon/Data/WeaponDataAsset.h"

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

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(TEXT("/Game/Animation/FL_PlayerAnimBP.FL_PlayerAnimBP_C"));
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
	// true�� ���, ĳ������ Yaw ȸ��(�¿� ȸ��)�� PlayerController�� Rotation�� ���� ȸ����Ų��.
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); // ȸ�� �ӵ� ������


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
	TestWeaponData = nullptr;
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

	// 테스트용 무기 자동 장착 (TestWeaponData가 설정되어 있을 때만)
	if (TestWeaponData)
	{
		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - TestWeaponData is set, spawning test weapon..."));

		// WeaponActor 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AWeaponActor* TestWeapon = GetWorld()->SpawnActor<AWeaponActor>(
			AWeaponActor::StaticClass(),
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams
		);

		if (TestWeapon)
		{
			// WeaponData 설정 (탄창 30발, 예비 탄약 90발)
			TestWeapon->InitializeWeapon(TestWeaponData, 30, 90);

			// 캐릭터에 장착
			EquipWeapon(TestWeapon);

			UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - Test weapon equipped successfully!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ABSCharacterPlayer::BeginPlay - Failed to spawn test weapon!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::BeginPlay - No TestWeaponData set, starting with empty hands"));
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
 * 무기 장착
 *
 * 동작:
 * 1. 기존 무기가 있으면 먼저 해제
 * 2. 새 무기를 CurrentWeapon에 저장
 * 3. WeaponActor->AttachToCharacter()로 손에 부착
 * 4. WeaponActor의 Owner를 this(캐릭터)로 설정
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

	// WeaponActor를 캐릭터 손에 부착
	// "hand_r_weapon" 소켓이 캐릭터 스켈레톤에 있어야 함 (언리얼 에디터에서 추가 필요)
	CurrentWeapon->AttachToCharacter(this, TEXT("hand_r_weapon"));

	// WeaponActor의 Owner를 캐릭터로 설정
	// 이렇게 하면 WeaponComponent에서 GetOwnerCharacter()로 캐릭터 접근 가능
	CurrentWeapon->SetOwner(this);

	UE_LOG(LogTemp, Log, TEXT("ABSCharacterPlayer::EquipWeapon - Equipped weapon: %s"), *CurrentWeapon->GetName());
}

/**
 * 무기 해제
 *
 * 동작:
 * 1. 현재 무기가 있는지 확인
 * 2. 발사 중지 (혹시라도 발사 중이면)
 * 3. WeaponActor->DetachFromCharacter()로 손에서 분리
 * 4. CurrentWeapon을 nullptr로 초기화 (빈손 상태)
 *
 * 참고: WeaponActor는 파괴하지 않고 월드에 떨어뜨림
 *       만약 파괴하려면 CurrentWeapon->Destroy() 호출
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

	// 빈손 상태로
	CurrentWeapon = nullptr;
}

/**
 * 발사 시작 (좌클릭 누름)
 *
 * 동작:
 * 1. 무기가 장착되어 있는지 확인
 * 2. WeaponComponent->StartFire() 호출
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

	// WeaponComponent 가져오기
	UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ABSCharacterPlayer::StartFireWeapon - WeaponComponent is nullptr!"));
		return;
	}

	// 발사 시작
	WeaponComp->StartFire();
}

/**
 * 발사 중지 (좌클릭 뗌)
 *
 * 동작:
 * 1. 무기가 장착되어 있는지 확인
 * 2. WeaponComponent->StopFire() 호출
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

	// WeaponComponent 가져오기
	UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent();
	if (!WeaponComp)
	{
		return;
	}

	// 발사 중지
	WeaponComp->StopFire();
}

/**
 * 재장전 (R키)
 *
 * 동작:
 * 1. 무기가 장착되어 있는지 확인
 * 2. WeaponComponent->Reload() 호출
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

	// WeaponComponent 가져오기
	UWeaponComponent* WeaponComp = CurrentWeapon->GetWeaponComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ABSCharacterPlayer::ReloadWeapon - WeaponComponent is nullptr!"));
		return;
	}

	// 재장전 시작
	WeaponComp->Reload();
}
