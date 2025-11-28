// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "DebugDrawUtils.h"


// 2. 실제 변수 정의

TAutoConsoleVariable<int32> CVarBSMasterDebug(
	TEXT("BS.debug.MasterEnable"),
	1, // 기본값: 켜짐
	TEXT("All Debug management \n0: off\n 1: Individual settings"),
	ECVF_Cheat
);

// Console variables for debug visualization
TAutoConsoleVariable<int32> CVarDebugMonsterOnOff (
	TEXT("BS.debug.monster.DebugDraw"),
	0,
	TEXT("Show monster Debug\n0: Off, 1: On"),
	ECVF_Cheat);

extern TAutoConsoleVariable<int32> CVarDebugMonsterAttackRange(
	TEXT("BS.debug.monster.AttackRange"),
	0,
	TEXT("Show monster attack range circles (min/max)\n0: Off, 1: On"),
	ECVF_Cheat);

extern TAutoConsoleVariable<int32> CVarDebugMonsterTargetLine(
	TEXT("BS.debug.monster.TargetLine"),
	0,
	TEXT("Show line to target player\n0: Off, 1: On"),
	ECVF_Cheat);

extern TAutoConsoleVariable<int32> CVarDebugMonsterForwardVector(
	TEXT("BS.debug.Monster.ForwardVector"),
	0,
	TEXT("Show monster forward vector arrow\n0: Off, 1: On"),
	ECVF_Cheat);
