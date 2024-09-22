// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "_ARPG/Interfaces/StackStateInterface.h"
#include "StackUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class UE_ARPGSYSTEM_API UStackUserWidget : public UUserWidget, public IStackStateInterface
{
	GENERATED_BODY()

public:
	virtual void EnterState(EStackAction::Type StackAction) override;
	virtual void ExitState(EStackAction::Type StackAction) override;
	virtual void UpdateState(float DeltaTime) override;
	
};
