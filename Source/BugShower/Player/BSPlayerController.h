// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"

#include "BSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BUGSHOWER_API ABSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABSPlayerController();

	virtual void BeginPlay() override;

	// Enable/Disable game input (for UI mode)
	void EnableGameInput();
	void DisableGameInput();

	// ========================================
	// 멀티플레이어 세이브 시스템
	// ========================================

	/**
	 * 서버가 클라이언트에게 인벤토리 저장 명령 전송
	 * 게임 클리어 시 서버가 각 클라이언트에게 호출
	 * 클라이언트가 자신의 로컬 디스크에 저장
	 */
	UFUNCTION(Client, Reliable)
	void ClientSaveInventory();

	/*
	Input Mapping Context��?
	Ű����, ���콺 �� �Է� ��ġ�� � �׼ǿ� �������� �����ϴ� ������ ����
	���� ��� "W-������ �̵�" ������ �����ϴ� ����
	�̶� TObjectPtr<>�� �𸮾��� ����Ʈ �������̸�, �����Ϳ��� .uasset�� �巡�� �ؼ� ������ �� �ִ�.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

};
