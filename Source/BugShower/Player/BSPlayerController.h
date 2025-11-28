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

	/*
	Input Mapping Context��?
	Ű����, ���콺 �� �Է� ��ġ�� � �׼ǿ� �������� �����ϴ� ������ ����
	���� ��� "W-������ �̵�" ������ �����ϴ� ����
	�̶� TObjectPtr<>�� �𸮾��� ����Ʈ �������̸�, �����Ϳ��� .uasset�� �巡�� �ؼ� ������ �� �ִ�.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

};
