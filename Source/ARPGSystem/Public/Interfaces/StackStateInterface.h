// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StackStateInterface.generated.h"

// 定义栈的两种行为
UENUM(BlueprintType)
enum EStackAction
{
	Push,
	Pop
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UStackStateInterface : public UInterface
{
	GENERATED_BODY()
};

// 栈日志
DECLARE_LOG_CATEGORY_EXTERN(LogStateMachine, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogStackStateMachine, Log, All);

// 栈接口广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateEnterSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateExitSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateUpdateSignature);

/**
 * 
 */
class ARPGSYSTEM_API IStackStateInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 栈状态的三种方法
	virtual void EnterState(EStackAction InStackAction) = 0;
	virtual void ExitState(EStackAction InStackAction) = 0 ;
	virtual void UpdateState(float DeltaTime) = 0;
};
