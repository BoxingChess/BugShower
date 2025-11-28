// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"



extern TAutoConsoleVariable<int32> CVarBSMasterDebug;

// DebugUtils.h
class BSDebugUtils
{
public:
	// 이 함수 하나로 체크
	static bool IsEnabled(const TAutoConsoleVariable<int32>& LocalCVar)
	{
		// 마스터가 꺼져있으면 무조건 false
		if (CVarBSMasterDebug.GetValueOnGameThread() == 0)
		{
			return false;
		}
		// 마스터가 켜져있으면 Local CVar 값 리턴
		return LocalCVar.GetValueOnGameThread() > 0;
	}
};
// Console variables for debug visualization

//in DetectPlayer.cpp
extern TAutoConsoleVariable<int32> CVarDebugMonsterOnOff;
extern TAutoConsoleVariable<int32> CVarDebugMonsterAttackRange;
extern TAutoConsoleVariable<int32> CVarDebugMonsterTargetLine;

//in MonsterBase
extern TAutoConsoleVariable<int32> CVarDebugMonsterForwardVector;
