// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h" // 델리게이트 매크로 정의
#include "GameFramework/Actor.h"              // AActor 기호 보장 (인텔리센스 안정화)


#include "PickUpDetectorComponent.generated.h"



class AItemActor;

/*
델리게이트는 간단히 말해 언리얼에서 "이벤트"를 알리는 신호
어떤 객체가 무언가 바뀌었다를 브로트캐스트 할 경우, 거기에 바인딩해둔 다른 함수들이 자동으로 호출된다.

Single-cast delegate: 한 개의 리스너만 가질 수 있음
Multicast delegate: 여러 리스너를 가질 수 있음 (UI, 사운드, 이펙트 등 여러 군데서 동시에 듣기 좋음)
Dynamic: 블루프린트에서도 바인딩 가능 (리플렉션을 통해 저장/언바인딩 지원)
*/

/// 완료 알림 -  UIComponent에서 받아다 처리할수 있도록 한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVicinityScanCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusItemChanged, AItemActor*, FocusedItem);

// UPickUpDetectorComponent.h
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUGSHOWER_API UPickUpDetectorComponent : public UActorComponent
{
   GENERATED_BODY()
public:
    UPickUpDetectorComponent();

protected:
    virtual void BeginPlay() override;
public:
    void TickComponent(float dt, ELevelTick, FActorComponentTickFunction*);

public:
    // 레이캐스트 설정
    UPROPERTY(EditAnywhere, Category="Pickup|Trace") //블루프린트 에디터 수정 가능, 에디터에서 PickUp폴더 밑에 Trace하위 카테고리 표시
    float FocusTraceDistance = 550.f;   //카메라 레이캐스트 최대 거리. 카메라에서 쏘니까 800으로 세팅해준다.

    //레이캐스트에 사용할 충돌 채널.ECC_Visibility - 시야 관련 트레이스
    UPROPERTY(EditAnywhere, Category="Pickup|Trace")
    TEnumAsByte<ECollisionChannel> FocusTraceChannel = ECC_Visibility; 

    // 근접 수집 설정
    //발밑 근처 아이템을 탐색할 반경
    UPROPERTY(EditAnywhere, Category="Pickup|Nearby")
    float NearbyRadius = 150.f;

    //오버랩 혹은 라인트레이스로 잡힌 액터들 중 아이템 액터만 걸러내기 위한 클래스 필터를 에디터에서 바꿀수 있게 하려고, 현재는 쓰지 않지만 추후 쓸수 있도록 바꾼다.
    UPROPERTY(EditAnywhere, Category="Vicinity")
    TSubclassOf<AActor> ItemActorClass;

    //근처 아이템 탐색 시 어떤 충돌 채널을 검색할지 지금은 WorldDynamic
    UPROPERTY(EditAnywhere, Category="Vicinity")
    TEnumAsByte<ECollisionChannel> NearbyChannel = ECC_WorldDynamic;

    //현재 카메라가 보고있는 아이템 액터를 반환
    UFUNCTION(BlueprintPure)    //부수 효과가 없는 getter이기에 pure노드로 사용할수 있게끔..
    AItemActor* GetFocusedItem() const { return FocusedItem.Get(); }

    //현재 발밑 주변에서 찾은 아이템 리스트 반환
    UFUNCTION(BlueprintPure)
    const TArray<AActor*>& GetNearbyItems() const { return NearbyItems; }

    // 카메라에서 레이캐스트를 쏴 포커스된 대상 업데이트
    UFUNCTION(BlueprintCallable)
    bool LineTraceFocus();            


//아이템 줍기 관련--------------------------------------------------
    // 발밑 주변 아이템 목록 갱신
    UFUNCTION(BlueprintCallable) 
    void RefreshNearbyList(bool isForced = false);

    // 주변아이템 검색 완료 시 브로드캐스트되는 이벤트
	UPROPERTY(BlueprintAssignable)
    FOnVicinityScanCompleted OnRefreshNearbyList;

    //라인트레이스로 바라보는 아이템 변경 이벤트
    UPROPERTY(BlueprintAssignable)
    FOnFocusItemChanged OnFocusItemChanged;   


    //현재 포커스된 아이템 줍기
    UFUNCTION(BlueprintCallable) 
    void TryPickupFocused(); 

    //특정 액터를 줍기(UI 드래그 & 드롭용)
     UFUNCTION(Server, Reliable)
     void ServerTryPickup(class AItemActor* Item);


     //특정 액터를 일부분만 줍기(Alt + 드래그 & 드랍용)
     UFUNCTION(Server, Reliable)
     void ServerTryPickupPartial(class AItemActor* Item, int32 Quantity);

private:
    //GC안전, 순환참조를 막자.. 이따가 다른 Component도 고쳐주자..
    UPROPERTY(Transient) //Transient - 저장되지 않음
    TWeakObjectPtr<class ABSCharacterPlayer> OwnerChar;

    //캐릭터의 인벤토리 컴포넌트 포인터
    UPROPERTY(Transient) 
    TWeakObjectPtr<class UInventoryComponent> Inventory;

    //현재 보고있는 아이템 액터
    UPROPERTY(Transient) 
    TWeakObjectPtr<AItemActor> FocusedItem;

    
    // 이전 프레임 결과 저장용
    UPROPERTY()
    TArray<AActor*>  PreviousNearbyItems;


    //발밑 주변 아이템 목록
    UPROPERTY(Transient) 
    TArray<AActor*> NearbyItems;

    //대상이 아이템 액터인지 확인
    bool IsItemActor(AActor* Actor) const; 

    // Inventory->AddItem 호출 래퍼
    bool PickupInternal(AActor* Item);    

public:
    //상태를 바꾸어 다른 로직을 수행하도록 한다.
    UFUNCTION(BlueprintCallable) 
    void ChangeState();

private:
    ///상태 플래그
    bool bIsInventoryOpen = false;
    bool bIsLineTrace = true;
		
};
