#pragma once

#include "CoreMinimal.h"
#include "ItemEnum.h"
#include "BSStaticItem.generated.h"

class AItemActor;

USTRUCT(BlueprintType)
struct FBSStaticItem
{
	GENERATED_BODY();

	// ������ �̸� (UI, �κ��丮 ��� �÷��̾�� �������� �̸�)
    // ��: "����óġŰƮ", "M416" ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    // ������ ������ (UI���� �ð������� ������ �� ���Ǵ� �̹���)
    // ��: �κ��丮 ���Կ� �� Texture2D �̹���
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Icon = nullptr;

    // ������ ���� (�����̳� �� ���� ��� ���)
    // ��: "��� �� ü���� 75 ȸ���մϴ�."
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    // �ش� �������� ���� �������� ���� (true�� ���� ���� ����)
    // ��: ź��, ���� ���� true / ����, �� ���� false
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStackable = true;

    //Item �ϳ��� ����, ���� InventoryStack�� �ױ� ���ؼ��� �ش� ������ ������ ���� ���� �ִ´�.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Weight = 1;

    // �ϳ��� �κ��丮 ���Կ� ���� �� �ִ� �ִ� ����
    // ��: 9mm ź���� 999������, ȸ������ 10������ �� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStackSize = 999;

    // �߰��� �޽� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* WorldMesh = nullptr;

};
