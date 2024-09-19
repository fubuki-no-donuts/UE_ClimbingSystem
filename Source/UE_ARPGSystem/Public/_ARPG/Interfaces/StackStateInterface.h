// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StackStateInterface.generated.h"

// The object stored in StackStateMachine has two common states: Push, Pop
UENUM(BLueprintType)
namespace EStackAction
{
	enum Type
	{
		STATE_Push UMETA(DisplayName = "Object be pushed into"),
		STATE_Pop UMETA(DisplayName = "Object be popped out")
	};
}

DECLARE_LOG_CATEGORY_EXTERN(LogStateMachine, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogStackStateMachine, Log, All);

// Broadcast when object's state changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateEnterSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateExitSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStateUpdateSignature);


// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UStackStateInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UE_ARPGSYSTEM_API IStackStateInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Three methods for a state object: Enter, Exit, Update
	virtual void EnterState(EStackAction::Type InStackAction) = 0;
	virtual void ExitState(EStackAction::Type InStackAction) = 0 ;
	virtual void UpdateState(float DeltaTime) = 0;
};
