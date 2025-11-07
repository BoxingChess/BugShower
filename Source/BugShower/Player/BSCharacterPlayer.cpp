// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BSCharacterPlayer.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/Movement/MovementInputComponent.h"					//자체컴포넌트
#include "Component/PickUp/PickUpDetectorComponent.h"					//자체컴포넌트
#include "Component/Inventory/InventoryComponent.h"						//자체컴포넌트

#include "DrawDebugHelpers.h" // 디버그 라인용
#include "Kismet/GameplayStatics.h" // 데미지용

ABSCharacterPlayer::ABSCharacterPlayer()
{
	//메쉬의 위치와 회전을 설정한다. - 월드 좌표가 아닌 캡슐 컴포넌트 기준의 상대 위치 -> 그냥 쓸 경우 공중에 뜨기 때문
//언리얼의 기본 메쉬들이 오른쪽을 앞으로 보기때문에 -90도를 돌려주었음
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));

	//메쉬가 어떤 방식으로 애니메이션을 재생할 것인지 설정, AnimationBlueprint - 메시가 애니메이션 블루프린트(UAnimInstance)를 사용
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	//캐릭터의 SkeletalMeshComponent의 충돌 설정을 "CharacterMesh"라는 이름의 충돌 프로파일로 설정한다는 뜻. -> 나머지 노션에 정리
	//즉, 캐릭터의 스켈레탈 메시에 충돌 설정을 CharacterMesh로 설정한다는 뜻 즉 내부에 있는 ECC_Visibility 채널에 대해 Block, Ignore, Overlap 중 뭘로 처리할 것인지를 판단
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

	//Player캐릭터의 메쉬스켈레톤 컴포넌트를 업데이트 해준다.
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

	///컴포넌트 생성--------------------------------------------------------------------------------------------------------------------------------------------------
	
	//이동 관련 컴포넌트 생성
	MovementComponentOnGround = CreateDefaultSubobject<UMovementInputComponent>(TEXT("MovementComponentOnGround"));

	//UI 관련 컴포넌트 생성 -> 이건 플레이어 밖으로 빼야한다.
	//UIComponent = CreateDefaultSubobject<UUI_InGameComponent>(TEXT("UIComponent"));

	//줍기 관련 컴포넌트
	PickUpDetectorComponent = CreateDefaultSubobject<UPickUpDetectorComponent>(TEXT("PickUpDetectorComponent"));

	//인벤토리 컴포넌트
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	



	//발사 관련
	static ConstructorHelpers::FObjectFinder<UInputAction> FireActionRef(TEXT("/Game/Input/IA_Fire.IA_Fire"));
	if (FireActionRef.Succeeded())
	{
		FireAction = FireActionRef.Object;
	}


	// 스프링암 생성
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent); // 캡슐에 붙임
	SpringArm->TargetArmLength = 400.0f; // 카메라 거리
	SpringArm->bUsePawnControlRotation = true; // 마우스 회전에 따라 회전
	SpringArm->SocketOffset = FVector(0.f, 0.f, 100.f); //자연스러운 카메라 위치 조정

	//아래는 부드럽게 따라오는 카메라를 보고 싶을때 사용.
	//SpringArm->bEnableCameraRotationLag = true;
	//SpringArm->CameraRotationLagSpeed = 50.0f;

	//3인칭 카메라 생성
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPerson_FollowCamera"));
	ThirdPersonCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // 스프링암 끝에 붙임
	ThirdPersonCamera->bUsePawnControlRotation = false; // 카메라는 따로 회전하지 않게

	// 1인칭 카메라 생성
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPerson_EyeCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head")); // 'head' 본에 부착 (Skeleton에 따라 이름 다를 수 있음)
	FirstPersonCamera->bUsePawnControlRotation = true; // 마우스 회전에 따라 회전
	FirstPersonCamera->SetActive(false); // 초기엔 비활성화

	// bUseControllerRotationYaw:
	// true일 경우, 캐릭터의 Yaw 회전(좌우 회전)을 PlayerController의 Rotation에 맞춰 회전시킨다.
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); // 회전 속도 빠르게


	// bUseControllerRotationPitch:
	// 캐릭터가 Pitch 회전(상하 회전)을 컨트롤러에 따라 할지 여부.
	// true면 캐릭터 자체가 위/아래를 바라보게 되므로, (말 그대로 캐릭터 메쉬자체가 위/아래를 보게된다.)
	// 3인칭에선 false로 두고, 카메라만 Pitch를 반영하는 것이 일반적이다
	bUseControllerRotationPitch = false;

	// 현재 시점을 ThirdPerson으로 설정한다. (초기값)
	CurrentViewMode = ECameraViewMode::ThirdPerson;
	ThirdPersonCamera->SetActive(true);
	FirstPersonCamera->SetActive(false);



	//캐릭터 스테이트 설정
	CharacterState = ECharacterState::Normal;
}

void ABSCharacterPlayer::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

	UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Initing..."));
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	MovementComponentOnGround->FL_SetupPlayerInputComponent(PlayerInputComponent);

	// PlayerInputComponent를 EnhancedInputComponent로 캐스팅
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInput Casting"));

		EnhancedInput->BindAction(FireAction, ETriggerEvent::Triggered, this, &ABSCharacterPlayer::Fire);
	}

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
	// 카메라 전환
	if (NewMode == ECameraViewMode::FirstPerson)
	{
		FirstPersonCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);

		// 메쉬 숨기기 예시
		//GetMesh()->HideBoneByName("head", EPhysBodyOp::PBO_None);
		GetMesh()->SetOwnerNoSee(true);

		//그래도 그림자는 그리게 해야한다.
		GetMesh()->SetCastShadow(true);		//이 메쉬가 그림자를 만들것인가?
		GetMesh()->bCastHiddenShadow = true; // 메쉬가 카메라에 보이지 않아도 그림자를 투사할 것인가?

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
	// 민감도도 줄이려면 변수 따로 두고 조절

	// 집중 모드 카메라 전환 시
	SpringArm->SocketOffset = FVector(0.f, 100.f, 100.f); //카메라를 캐릭터 오른쪽으로 옮긴다.
}

void ABSCharacterPlayer::EndThirdPersonZoom()
{
	ThirdPersonCamera->SetFieldOfView(90.0f);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 100.f); //자연스러운 카메라 위치 조정
}


// 캐릭터가 새로운 컨트롤러에 의해 점유(Possess)될 때 호출되는 함수.
// 서버에서만 호출됨.
// NewController가 붙는 시점이므로, MovementComponent 내부의 OwnerController 포인터를 갱신해준다.
// 이렇게 하면 컨트롤러가 교체되었을 때(예: UI 전용 컨트롤러로 변경) 입력 처리 로직을 즉시 반영할 수 있다.
void ABSCharacterPlayer::PossessedBy(AController* NewController)
{
	
	Super::PossessedBy(NewController);
	if (MovementComponentOnGround)
	{
		MovementComponentOnGround->RefreshControllerCache();
	}
	
}


// Controller 변수가 네트워크를 통해 복제(Replicate)되어 클라이언트에서 변경되었을 때 호출되는 함수.
// 멀티플레이 환경에서 클라이언트는 서버처럼 PossessedBy()가 호출되지 않기 때문에,
// OnRep_Controller()를 통해 동일한 갱신 작업을 수행해야 한다.
// 여기서도 MovementComponent의 OwnerController를 새로 세팅하여
// UI 전용 컨트롤러가 붙었을 때 입력을 막거나, 새로운 컨트롤러에 맞는 동작을 적용할 수 있다.
void ABSCharacterPlayer::OnRep_Controller()
{
	
	Super::OnRep_Controller();
	if (MovementComponentOnGround)
	{
		MovementComponentOnGround->RefreshControllerCache();
	}
	
}

///TODO : ProjectTileMovementComponent을 사용해서 하나의 컴포넌트를 만들어 빼보자.
void ABSCharacterPlayer::Fire()
{
	// 시작점 = 캐릭터의 오른 손의 Bone위치로 한다.
	//FVector Start = GetActorLocation();
	FVector Start = GetMesh()->GetSocketLocation(TEXT("hand_r")); // "hand_r"가 오른손

	// 끝점 = 카메라 정면 방향 * 길이
	FVector End = Start + (ThirdPersonCamera->GetForwardVector() * 10000.0f);

	// 충돌 무시 설정 (자기 자신 제외)
	FCollisionQueryParams Params;	//충돌 검사 시 사용되는 설정 정보
	Params.AddIgnoredActor(this);	//자기 자신을 무시 시킨다.

	// 라인트레이스
	FHitResult HitResult;
	//월드에서 하나의 Line을 쏴서 처음 맞는 액터를 확인한다.
	//관통같이 여러 액터를 맞게하고 싶으면 LineTraceMultiByChannel를 쏴야한다.
	bool bHit = GetWorld()->LineTraceSingleByChannel
	(
		HitResult,		//충돌 정보를 저장하는 구조체
		Start,			//시작 지점
		End,			//끝 지점
		ECC_Visibility,	//충돌 채널
		Params			//충돌 옵션
	);

	// 디버그 라인 그리기
	FColor LineColor = bHit ? FColor::Red : FColor::Green;
	FVector LineEnd = bHit ? HitResult.ImpactPoint : End;

	DrawDebugLine
	(
		GetWorld(),
		Start,
		LineEnd,
		LineColor,
		false,         // 영구 표시 여부
		1.0f,          // 지속 시간
		0,             // 깊이 우선 순위
		1.0f           // 선 굵기
	);

	// 피격 처리
	if (bHit && HitResult.GetActor())	//충돌한 액터가 실제로 존재하는지 확인(충돌 대상이 유효한 경우만 아래 로직을 수행한다.)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
		UGameplayStatics::ApplyDamage
		(
			HitResult.GetActor(),	//피해를 입을 액터(즉, 데미지를 입을 액터를 의미한다.)
			10.0f,					//피해량(추후 총기, 근접무기에 맞게 다시 세팅을 해야한다. TODO)
			GetController(),		//공격자의 컨트롤러
			this,					// 피해를 준 원인 (발사한 캐릭터 자신)
			nullptr					// 데미지 타입 (null이면 기본값 사용/데미지 타입 클래스 (예: 화염, 폭발 등 커스텀 가능)/ 추후 이펙트, 넉백, 슬로우, 지속 피해등 효과 차별화 가능)
		);
	}
}
