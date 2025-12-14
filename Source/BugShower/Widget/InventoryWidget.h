// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/ItemListEntryObject.h"
#include "InventoryWidget.generated.h"
/**
 * 
 */

// �б� ���
UENUM()
enum class ESplitMode : uint8
{
    None,
    VicinityToInventory,   // �ֺ�(����) �� �κ��丮 : �ݱ�(�Ϻ�)
    InventoryToVicinity    // �κ��丮 �� �ֺ� : ������(�Ϻ�)
};

class UListView;                    // ���漱��
class USplitQuantityDialog;

UCLASS()
class BUGSHOWER_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(meta = (BindWidget))
	UListView* VicinityList;

	UPROPERTY(meta = (BindWidget))
	UListView* InventoryList;
	
    // �����Ϳ��� WBP_SplitQuantityDialog ����
    UPROPERTY(EditDefaultsOnly, Category="DragDrop")
    TSubclassOf<USplitQuantityDialog> SplitDialogClass;

public:
	//�ֺ� �������� �����Ѵ�.
	UFUNCTION(BlueprintCallable) 
	void SetVicinity(const TArray<AActor*>& Rows);

	//�κ��丮 �������� �����Ѵ�.
    UFUNCTION(BlueprintCallable) 
	void SetInventory(const TArray<UBSItemInstance*>& InventoryItems);

private:
	UItemListEntryObject* MakeRow(UObject* Outer, UTexture2D* Icon, const FText& Name, int32 Qty, AItemActor* Source, UBSItemInstance* SourceInstance);

public:
	//�巡�� �� ������� ����� �߻����� �� ����Ǵ� �Լ�.
	//InGeometry : �� ������ ȭ�� �� ��ġ / ũ�� ����.��� ��ǥ ��� � ����� �� ����.
	//InDragDropEvent : �巡�� ����(���콺 ��ǥ ��)�� ��� �̺�Ʈ.
	//InOperation : �巡�� ���� �� ������ UDragDropOperation ��ü.���� ���� Payload�� �巡���� ���� �����Ͱ� ��� ����.
	//��ȯ�� bool : ����� ó�������� true, �����ϸ� false.
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// 드롭된 위치가 어느 위젯 위인지 확인
	// @param InGeometry: 이 위젯(InventoryWidget)의 지오메트리
	// @param InDragDropEvent: 드롭 이벤트 (마우스 위치 포함)
	// @return: 0=알 수 없음, 1=VicinityList, 2=InventoryList
	int32 GetDropTargetWidget(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent);

private:
    // Alt-��� �� ���ؽ�Ʈ ����
    UPROPERTY() 
	TWeakObjectPtr<UItemListEntryObject> PendingSplitObj = nullptr;

    UPROPERTY() 
	ESplitMode PendingMode = ESplitMode::None;

    UFUNCTION() 
	void HandleSplitConfirm(int32 Quantity);

    UFUNCTION()
	void HandleSplitCancel();

private:
	// PickUpDetectorComponent에서 주변 아이템 갱신 이벤트를 받기 위한 콜백
	UFUNCTION()
	void OnNearbyItemsRefreshed();

	// InventoryComponent에서 인벤토리 변경 이벤트를 받기 위한 콜백
	UFUNCTION()
	void OnInventoryItemsChanged();

	// PickUpDetectorComponent 캐싱
	UPROPERTY()
	TWeakObjectPtr<class UPickUpDetectorComponent> CachedPickUpDetector;

	// InventoryComponent 캐싱
	UPROPERTY()
	TWeakObjectPtr<class UInventoryComponent> CachedInventory;

	// ========================================
	// 3D 캐릭터 프리뷰
	// ========================================

public:
	// Image 위젯 (Blueprint에서 BindWidget)
	UPROPERTY(meta = (BindWidget))
	class UImage* Img_CharacterPreview;

private:
	// Render Target (Scene Capture의 출력)
	UPROPERTY()
	TObjectPtr<class UTextureRenderTarget2D> RenderTarget;

	// Scene Capture Component (3D를 2D 텍스처로 렌더링)
	UPROPERTY()
	TObjectPtr<class USceneCaptureComponent2D> SceneCapture;

	// 프리뷰용 캐릭터 메시 (Actor가 아니라 Component만)
	UPROPERTY()
	TObjectPtr<class USkeletalMeshComponent> PreviewMesh;

	// 카메라 암 (회전용)
	UPROPERTY()
	TObjectPtr<class USceneComponent> CameraArm;

	// 현재 회전 각도
	float CurrentYaw = 0.0f;

	// 마우스 드래그 중인지
	bool bIsDraggingCharacter = false;

	// 드래그 시작 위치
	FVector2D DragStartPosition;

	// 3D 프리뷰 씬 초기화
	void SetupPreviewScene();

	// 3D 프리뷰 씬 정리
	void CleanupPreviewScene();

	// 카메라 위치 업데이트 (Yaw 각도 기반 공전)
	void UpdateCameraPosition(float Yaw);

	// 마우스 이벤트 처리
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
