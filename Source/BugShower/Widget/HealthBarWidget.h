// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

/*
 �׻� ǥ�õǴ� ü�¹� �����̴�.
 Ʈ��ũ�� �������� ������ ���¹̾� ��µ� �ð������� ���̰� �������ٲ���.
 */
/**
 * 플레이어 체력바 위젯
 * 항상 화면에 표시되며 HP, 에블라 입자 등을 표시
 */
UCLASS()
class BUGSHOWER_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * HP 업데이트 (C++에서 호출, Blueprint에서 구현)
	 * @param CurrentHP 현재 HP
	 * @param MaxHP 최대 HP
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateHealth(float CurrentHP, float MaxHP);

	/**
	 * 에블라 입자 업데이트 (C++에서 호출, Blueprint에서 구현)
	 * @param CurrentAblaParticle 현재 에블라 입자
	 * @param MaxAblaParticle 최대 에블라 입자
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateAblaParticle(float CurrentAblaParticle, float MaxAblaParticle);
};
