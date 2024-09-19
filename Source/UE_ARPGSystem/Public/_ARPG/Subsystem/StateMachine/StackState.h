// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../../Interfaces/StackStateInterface.h"
#include "StackState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStackState, Log, All);

/**
 * This is a Abstract Base StackState class, for some custom using
 */
UCLASS(Abstract, BlueprintType, Blueprintable, HideCategories = ("Cooking", "LOD", "Physics", "Activation", "Tags", "Rendering"))
class UE_ARPGSYSTEM_API UStackState : public UObject, public IStackStateInterface
{
	GENERATED_BODY()
public:
	// IStackStateInterface
	virtual void EnterState(EStackAction::Type InStackAction) override;
	virtual void ExitState(EStackAction::Type InStackAction) override;
	virtual void UpdateState(float DeltaTime) override;
	
};
