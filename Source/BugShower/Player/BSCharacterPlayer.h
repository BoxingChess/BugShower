// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/BSCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "EnhancedInputComponent.h"
#include "BSCharacterTypes.h"

#include "BSCharacterPlayer.generated.h"

class UMovementInputComponent;
class UUI_InGameComponent;
class UPickUpDetectorComponent;
class UInventoryComponent;
class UPlayerStatComponent;
class AWeaponActor;

/*
 �� Ŭ������ ABSCharacterBase�� ��ӹ޴� �÷��̾� ĳ���� Ŭ�����̴�.
 */
UCLASS()
class BUGSHOWER_API ABSCharacterPlayer : public ABSCharacterBase
{
	GENERATED_BODY()
	
public:
	ABSCharacterPlayer();

protected:
	virtual void BeginPlay() override;

public:
/*
�𸮾� �������� �÷��̾��� �Է��� ó���ϱ� ���� �������̵� �ؾ��ϴ� �Լ�
Ű���峪 �е��� �Է��� ���� �����ڵ忡 �����ϴ� �ٽ� �Լ�
*/
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

///�ش� �÷��̾ ������ �ִ� Ŀ���� ������Ʈ��
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UMovementInputComponent> MovementComponentOnGround;

	//UI�� �����ϴ� ���� ���� �÷��̾ �ƴ� �ܺ��� UIComponent���� �����ϰԲ� �� ���̴�.
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	//TObjectPtr<UUI_InGameComponent> UIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPickUpDetectorComponent> PickUpDetectorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPlayerStatComponent> StatComponent;

protected:
	// Camera boom that follows the character (spring arm component)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	// Third person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> ThirdPersonCamera;

	// First person camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FirstPersonCamera;

	// ========================================
	// 인벤토리 3D 프리뷰용 카메라
	// ========================================

	// 인벤토리 UI용 카메라 암 (회전용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> InventoryCameraArm;

	// 인벤토리 UI용 SceneCapture2D (3D 캐릭터를 렌더링하여 UI에 표시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneCaptureComponent2D> InventoryCamera;

public:
	// Camera system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	ECameraViewMode CurrentViewMode;

	ECharacterState CharacterState;

	void SetCurrentViewMode(ECameraViewMode NewMode);
	void SetCharacterState(ECharacterState _newState);

	ECameraViewMode GetCurrentViewMode();
	ECharacterState GetCharacterState();

	void ToggleViewMode();
	void SetCameraViewMode(ECameraViewMode NewMode);

	void StartThirdPersonZoom();
	void EndThirdPersonZoom();

public:
	// Called when this character is possessed/unpossessed by a controller
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	// ========================================
	// 무기 시스템
	// ========================================

protected:
	// 게임 시작 시 기본 무기 자동 장착 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoEquipDefaultWeapon = false;

	// 기본 무기 DataAsset (bAutoEquipDefaultWeapon이 true일 때만 사용)
	// 에디터에서 설정 가능 (None이면 무기 없이 시작)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (EditCondition = "bAutoEquipDefaultWeapon"))
	TObjectPtr<class UWeaponDataAsset> DefaultWeaponData;

	// 현재 장착된 무기 (런타임에 자동으로 설정됨)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AWeaponActor> CurrentWeapon;

public:
	/**
	 * 무기 장착 (로컬 실행)
	 * WeaponActor를 캐릭터 손에 부착하고 사용 가능 상태로 만듦
	 * 데디서버 환경에서는 ServerEquipWeapon을 호출해야 함
	 *
	 * @param Weapon - 장착할 WeaponActor (nullptr이면 무시)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(AWeaponActor* Weapon);

	/**
	 * 무기 장착 서버 RPC
	 * 클라이언트가 무기를 주울 때 서버에 요청하는 함수
	 * 서버에서 EquipWeapon()을 실행하고 자동으로 모든 클라이언트에 리플리케이트됨
	 *
	 * @param Weapon - 장착할 WeaponActor
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon")
	void ServerEquipWeapon(AWeaponActor* Weapon);

	/**
	 * 무기 해제 (로컬 실행)
	 * 현재 장착된 무기를 손에서 떼어냄 (땅에 떨어뜨리거나 파괴)
	 * 빈손 상태가 됨
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon();

	/**
	 * 무기 해제 서버 RPC
	 * 클라이언트가 무기를 버릴 때 서버에 요청하는 함수
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon")
	void ServerUnequipWeapon();

	/**
	 * 발사 시작 (좌클릭 누름)
	 * CurrentWeapon의 WeaponComponent->StartFire() 호출
	 * 무기가 없으면 아무 일도 안 일어남
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFireWeapon();

	/**
	 * 발사 중지 (좌클릭 뗌)
	 * CurrentWeapon의 WeaponComponent->StopFire() 호출
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFireWeapon();

	/**
	 * 재장전 (R키)
	 * CurrentWeapon의 WeaponComponent->Reload() 호출
	 * 무기가 없거나 이미 재장전 중이면 무시
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ReloadWeapon();

	/**
	 * 현재 장착된 무기 가져오기
	 * @return 현재 무기 (없으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	AWeaponActor* GetCurrentWeapon() const { return CurrentWeapon; }

	/**
	 * 무기 장착 여부 확인
	 * @return 무기를 들고 있으면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasWeaponEquipped() const { return CurrentWeapon != nullptr; }

	// ========================================
	// 입력 액션 (Enhanced Input)
	// ========================================

public:
	// 무기 발사 (좌클릭 - Started/Completed)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Weapon")
	TObjectPtr<class UInputAction> IA_FireWeapon;

	// 무기 재장전 (R키 - Triggered)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Weapon")
	TObjectPtr<class UInputAction> IA_ReloadWeapon;

	// ========================================
	// 수류탄 시스템 (기존 코드)
	// ========================================

public:
	// 수류탄 발사 (우클릭 - Started)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Grenade")
	TObjectPtr<class UInputAction> FireAction;

	// Projectile class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AProjectileBase> ProjectileClass;

	// 수류탄 발사 (우클릭)
public:
	void Fire();

	// ========================================
	// 데미지 시스템
	// ========================================

public:
	/**
	 * 데미지를 받았을 때 호출되는 함수
	 * StatComponent로 데미지를 전달하여 HP 감소 처리
	 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	                         class AController* EventInstigator, AActor* DamageCauser) override;

private:
	/**
	 * HP 변경 시 호출되는 콜백 함수
	 * UI 업데이트 등에 사용
	 */
	UFUNCTION()
	void OnPlayerHPChanged(float CurrentHP, float MaxHP);

	/**
	 * 플레이어 사망 시 호출되는 콜백 함수
	 */
	UFUNCTION()
	void OnPlayerDied();

	// ========================================
	// 스탯 Getter 함수 (Coupling 방지)
	// ========================================

public:
	/**
	 * 최대 점프 횟수 가져오기
	 * MovementComponent가 StatComponent를 직접 알 필요 없게 하기 위한 간접 레이어
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetMaxJumpCount() const;

	/**
	 * 점프력 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetJumpPower() const;

	/**
	 * 걷기 속도 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetWalkSpeed() const;

	/**
	 * 달리기 속도 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetSprintSpeed() const;

	/**
	 * 무기 장착 상태 가져오기
	 * MovementComponent, AnimInstance가 StatComponent를 직접 알 필요 없게 하기 위한 간접 레이어
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool GetIsArmed() const;

	/**
	 * 무기 장착 상태 설정
	 * EquipWeapon/UnequipWeapon에서 호출됨
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetIsArmed(bool bNewIsArmed);

};
