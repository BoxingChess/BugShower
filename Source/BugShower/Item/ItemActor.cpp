// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Item/ItemActor.h"
#include "Game/BSGameInstance.h"
#include "Manager/ResourceManager/ItemResourceManager/ItemResourceManager.h"

// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 메쉬 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));

	// 트랜스폼 조작이 가능하도록 루트 컴포넌트로 설정
	RootComponent = MeshComponent;

}

// Called when the game starts or when spawned
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (StaticItemInfo && StaticItemInfo->WorldMesh)
	{
		MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Loading StaticItemInfo"));

	}
}

//런타임 전용 처리방식
void AItemActor::InitializeItemBS_Item(FBS_Item& _item)
{
	///동적 정보 세팅
	ItemInformation = _item;

	///정적 정보 세팅
	// GameInstance에서 ResourceManager 가져오기
	if (UBSGameInstance* GI = GetWorld()->GetGameInstance<UBSGameInstance>())
	{
		UItemResourceManager* RM = GI->GetItemResourceManager(); // getter 함수
		if (RM)
		{
			StaticItemInfo = RM->GetStaticItem(_item.ItemType, _item.ItemID);

			if (StaticItemInfo && StaticItemInfo->WorldMesh && MeshComponent)
			{
				MeshComponent->SetStaticMesh(StaticItemInfo->WorldMesh);
			}
		}
	}
}

