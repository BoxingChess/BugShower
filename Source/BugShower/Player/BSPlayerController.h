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

	/*
	Input Mapping Context란?
	키보드, 마우스 등 입력 장치를 어떤 액션에 매핑할지 정의하는 데이터 에셋
	예를 들어 "W-앞으로 이동" 같은걸 설정하는 역할
	이때 TObjectPtr<>는 언리얼의 스마트 포인터이며, 에디터에서 .uasset을 드래그 해서 연결할 수 있다.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

};
