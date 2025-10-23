// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBTService, Log, All);

#define LOG_BT(Message, ...) \
    UE_LOG(LogBTService, Warning, TEXT("[%s:%d] %s : " Message), TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

DECLARE_LOG_CATEGORY_EXTERN(LogBTTask, Log, All);