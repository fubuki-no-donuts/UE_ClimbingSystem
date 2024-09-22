// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "_ARPG/Components/StackStateMachineComponent.h"
#include "UIManagerComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUIManager, Log, All);

UENUM(BlueprintType)
enum EUIStateCastResult
{
	Success,
	Failed
};

class UStackUserWidget;

/**
 * 
 */
UCLASS()
class UE_ARPGSYSTEM_API UUIManagerComponent : public UStackStateMachineComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void InitUIState();

	UFUNCTION(BlueprintPure, Category = "UIManager")
	UStackUserWidget* GetUI(FName UIName);

	
};
