// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h" // ��������Ʈ ��ũ�� ����
#include "GameFramework/Actor.h"              // AActor ��ȣ ���� (���ڸ����� ����ȭ)


#include "PickUpDetectorComponent.generated.h"



class AItemActor;

/*
��������Ʈ�� ������ ���� �𸮾󿡼� "�̺�Ʈ"�� �˸��� ��ȣ
� ��ü�� ���� �ٲ���ٸ� ���Ʈĳ��Ʈ �� ���, �ű⿡ ���ε��ص� �ٸ� �Լ����� �ڵ����� ȣ��ȴ�.

Single-cast delegate: �� ���� �����ʸ� ���� �� ����
Multicast delegate: ���� �����ʸ� ���� �� ���� (UI, ����, ����Ʈ �� ���� ������ ���ÿ� ��� ����)
Dynamic: ��������Ʈ������ ���ε� ���� (���÷����� ���� ����/����ε� ����)
*/

/// �Ϸ� �˸� -  UIComponent���� �޾ƴ� ó���Ҽ� �ֵ��� �Ѵ�.
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
    // ����ĳ��Ʈ ����
    UPROPERTY(EditAnywhere, Category="Pickup|Trace") //��������Ʈ ������ ���� ����, �����Ϳ��� PickUp���� �ؿ� Trace���� ī�װ��� ǥ��
    float FocusTraceDistance = 550.f;   //ī�޶� ����ĳ��Ʈ �ִ� �Ÿ�. ī�޶󿡼� ��ϱ� 800���� �������ش�.

    //����ĳ��Ʈ�� ����� �浹 ä��.ECC_Visibility - �þ� ���� Ʈ���̽�
    UPROPERTY(EditAnywhere, Category="Pickup|Trace")
    TEnumAsByte<ECollisionChannel> FocusTraceChannel = ECC_Visibility; 

    // ���� ���� ����
    //�߹� ��ó �������� Ž���� �ݰ�
    UPROPERTY(EditAnywhere, Category="Pickup|Nearby")
    float NearbyRadius = 150.f;

    //������ Ȥ�� ����Ʈ���̽��� ���� ���͵� �� ������ ���͸� �ɷ����� ���� Ŭ���� ���͸� �����Ϳ��� �ٲܼ� �ְ� �Ϸ���, ����� ���� ������ ���� ���� �ֵ��� �ٲ۴�.
    UPROPERTY(EditAnywhere, Category="Vicinity")
    TSubclassOf<AActor> ItemActorClass;

    //��ó ������ Ž�� �� � �浹 ä���� �˻����� ������ WorldDynamic
    UPROPERTY(EditAnywhere, Category="Vicinity")
    TEnumAsByte<ECollisionChannel> NearbyChannel = ECC_WorldDynamic;

    //���� ī�޶� �����ִ� ������ ���͸� ��ȯ
    UFUNCTION(BlueprintPure)    //�μ� ȿ���� ���� getter�̱⿡ pure���� ����Ҽ� �ְԲ�..
    AItemActor* GetFocusedItem() const { return FocusedItem.Get(); }

    //���� �߹� �ֺ����� ã�� ������ ����Ʈ ��ȯ
    UFUNCTION(BlueprintPure)
    const TArray<AActor*>& GetNearbyItems() const { return NearbyItems; }

    // ī�޶󿡼� ����ĳ��Ʈ�� �� ��Ŀ���� ��� ������Ʈ
    UFUNCTION(BlueprintCallable)
    bool LineTraceFocus();            


//������ �ݱ� ����--------------------------------------------------
    // �߹� �ֺ� ������ ��� ����
    UFUNCTION(BlueprintCallable) 
    void RefreshNearbyList(bool isForced = false);

    // �ֺ������� �˻� �Ϸ� �� ��ε�ĳ��Ʈ�Ǵ� �̺�Ʈ
	UPROPERTY(BlueprintAssignable)
    FOnVicinityScanCompleted OnRefreshNearbyList;

    //����Ʈ���̽��� �ٶ󺸴� ������ ���� �̺�Ʈ
    UPROPERTY(BlueprintAssignable)
    FOnFocusItemChanged OnFocusItemChanged;   


    //���� ��Ŀ���� ������ �ݱ�
    UFUNCTION(BlueprintCallable) 
    void TryPickupFocused(); 

    //Ư�� ���͸� �ݱ�(UI �巡�� & ��ӿ�)
     UFUNCTION(Server, Reliable)
     void ServerTryPickup(class AItemActor* Item);


     //Ư�� ���͸� �Ϻκи� �ݱ�(Alt + �巡�� & �����)
     UFUNCTION(Server, Reliable)
     void ServerTryPickupPartial(class AItemActor* Item, int32 Quantity);

private:
    //GC����, ��ȯ������ ����.. �̵��� �ٸ� Component�� ��������..
    UPROPERTY(Transient) //Transient - ������� ����
    TWeakObjectPtr<class ABSCharacterPlayer> OwnerChar;

    //ĳ������ �κ��丮 ������Ʈ ������
    UPROPERTY(Transient) 
    TWeakObjectPtr<class UInventoryComponent> Inventory;

    //���� �����ִ� ������ ����
    UPROPERTY(Transient) 
    TWeakObjectPtr<AItemActor> FocusedItem;

    
    // ���� ������ ��� �����
    UPROPERTY()
    TArray<AActor*>  PreviousNearbyItems;


    //�߹� �ֺ� ������ ���
    UPROPERTY(Transient) 
    TArray<AActor*> NearbyItems;

    //����� ������ �������� Ȯ��
    bool IsItemActor(AActor* Actor) const; 

    // Inventory->AddItem ȣ�� ����
    bool PickupInternal(AActor* Item);    

public:
    //���¸� �ٲپ� �ٸ� ������ �����ϵ��� �Ѵ�.
    UFUNCTION(BlueprintCallable)
    void ChangeState();

    // Check if inventory is currently open
    FORCEINLINE bool IsInventoryOpen() const { return bIsInventoryOpen; }

private:
    ///���� �÷���
    bool bIsInventoryOpen = false;
    bool bIsLineTrace = true;
		
};
