// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/StackStateInterface.h"
#include "StackWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class ARPGSYSTEM_API UStackWidgetBase : public UUserWidget, public IStackStateInterface
{
	GENERATED_BODY()

#pragma region StackStateInterface
	
public:
	virtual void EnterState(EStackAction InStackAction) override;
	virtual void ExitState(EStackAction InStackAction) override;
	virtual void UpdateState(float DeltaTime) override;
	
#pragma endregion 
	
#pragma region BlueprintMethods
	
public:
	// Override in Blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "StackWidgetBase", meta = (DisplayName = "OnEnterState"))
	void ReceiveEnterState(EStackAction InStackAction);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "StackWidgetBase", meta = (DisplayName = "OnExitState"))
	void ReceiveExitState(EStackAction InStackAction);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "StackWidgetBase", meta = (DisplayName = "OnUpdateState"))
	void ReceiveUpdateState(float DeltaTime);
	
#pragma endregion
};
