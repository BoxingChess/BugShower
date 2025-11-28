// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBTService, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogBTTask, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLogic, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogServer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogPooling, Log, All);


//In Behavior Tree Service/Task, use these macros for logging
#pragma region InBehaviorTree
// Verbose level - OFF by default, turn ON with console: "Log LogBTService Verbose"
#define LOG_BT(Message, ...) \
    UE_LOG(LogBTService, Verbose, TEXT("[%s:%d] %s : " Message), TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

// Always visible (for important info)
#define LOG_BT_INFO(Message, ...) \
    UE_LOG(LogBTService, Log, TEXT("[BT] " Message), ##__VA_ARGS__)

// Always visible (for warnings)
#define LOG_BT_WARNING(Message, ...) \
    UE_LOG(LogBTService, Warning, TEXT("[BT] " Message), ##__VA_ARGS__)

// Always visible (for errors)
#define LOG_BT_ERROR(Message, ...) \
    UE_LOG(LogBTService, Error, TEXT("[BT] " Message), ##__VA_ARGS__)
#pragma endregion InBehaviorTree

// Imple Logic in Client, use these macros for logging
#pragma region InLogic
#define LOG_FUNCTION(Message, ...) \
    UE_LOG(LogLogic, Verbose, TEXT("[%s:%d] %s : " Message), TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

#define LOG_LOGIC_INFO(Message, ...) \
    UE_LOG(LogLogic, Log, TEXT("[LOGIC] " Message), ##__VA_ARGS__)

// Always visible (for warnings)
#define LOG_LOGIC_WARNING(Message, ...) \
    UE_LOG(LogLogic, Warning, TEXT("[LOGIC] " Message), ##__VA_ARGS__)

// Always visible (for errors)
#define LOG_LOGIC_ERROR(Message, ...) \
    UE_LOG(LogLogic, Error, TEXT("[LOGIC] " Message), ##__VA_ARGS__)


#pragma endregion InLogic


#pragma region InServer

#define LOG_NETWORK_FUNCTION(Message, ...) \
    UE_LOG(LogNet, Verbose, TEXT("[%s:%d] %s : " Message), TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

#define LOG_NETWORK_INFO(Message, ...) \
    UE_LOG(LogNet, Log, TEXT("[NETWORK] " Message), ##__VA_ARGS__)

#define LOG_NETWORK_WARNING(Message, ...) \
    UE_LOG(LogNet, Warning, TEXT("[NETWORK] " Message), ##__VA_ARGS__)

#define LOG_NETWORK_MODE ((GetNetMode()== ENetMode::NM_Client) ? *FString::Printf("Client%d"),GPlayInEditorID) : ((GetNetMode()==ENetMode::NM_Standalone) ? TEXT("STAND ALONE") : TEXT("SERVER"))

#pragma endregion InServer

#pragma region InPoolingSubSystem

#define LOG_POOLING_FUNCTION(Message, ...) \
    UE_LOG(LogPooling, Verbose, TEXT("[%s:%d] %s : " Message), TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

#define LOG_POOLING_INFO(Message, ...) \
    UE_LOG(LogPooling, Log, TEXT("[POOLING] " Message), ##__VA_ARGS__)

#define LOG_POOLING_WARNING(Message, ...) \
    UE_LOG(LogPooling, Warning, TEXT("[POOLING] " Message), ##__VA_ARGS__)

#define LOG_POOLING_ERROR(Message, ...) \
    UE_LOG(LogPooling, Error, TEXT("[POOLING] " Message), ##__VA_ARGS__)

#pragma endregion InPoolingSubSystem