// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/StackStateInterface.h"
#include "UIManager.generated.h"

class UStackWidgetBase;

UENUM(Blueprintable)
enum ELayerLevel
{
	BottomLayer, // 最底层界面, 一般是HUD
	FirstLayer, // 一级界面, 从HUD或者按下ESC键打开的界面
	SecondLayer, // 二级界面, 一级界面的按钮打开的界面
	ThirdLayer, // 三级界面, 二级界面的按钮打开的界面
	TipLayer // 提示UI, 有固定生命周期, 因此单独维护一个栈
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStackStatePushPopSignature,
	TScriptInterface<IStackStateInterface>, State);

/**
 * 
 */
UCLASS()
class ARPGSYSTEM_API UUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region Override
	
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

#pragma endregion

#pragma region StackStateMachine

private:
	// Stack for UI
	TArray<TScriptInterface<IStackStateInterface>> States;

	// Stack for Tips
	TArray<TScriptInterface<IStackStateInterface>> TipStates;
	
public:
	// Define Push and Pop management
	UFUNCTION(BlueprintCallable, Category = "StackStateManager")
	void PushState(TScriptInterface<IStackStateInterface> NewState);

	UFUNCTION(BlueprintCallable, Category = "StackStateManager")
	void PopState();
	
	UFUNCTION(BlueprintCallable, Category = "StackStateManager")
	void PopStates(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "StackStateManager")
	void PopAllStates();

	// Helper Methods
	UFUNCTION(BlueprintCallable, Category = "StateStateManager")
	FORCEINLINE int32 GetStateCount() const { return States.Num(); }
	
	UFUNCTION(BlueprintCallable, Category = "StateStateManager")
	FORCEINLINE const TScriptInterface<IStackStateInterface>& GetCurrentState() const { return States.Last(); }

	// Delegate for State Pushed or Popped
	UPROPERTY(BlueprintAssignable, Category = "StackStateMachine")
	FOnStackStatePushPopSignature OnStatePushed;
	
	UPROPERTY(BlueprintAssignable, Category = "StackStateMachine")
	FOnStackStatePushPopSignature OnStatePopped;

	virtual void StatePushed(TScriptInterface<IStackStateInterface> PushedState);
	virtual void StatePopped(TScriptInterface<IStackStateInterface> PoppedState);
	
#pragma endregion

#pragma region UIManager

public:
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void InitUI();

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	UStackWidgetBase* OpenUI(FName UIName);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void CloseUI(FName UIName);
	
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void ReleaseAll();
	
private:
	// /Script/Engine.DataTable'/Game/_ARPG/Data/DT_UI_Info.DT_UI_Info'
	UPROPERTY()
	UDataTable* UIClassTable;

	UPROPERTY()
	TMap<FName, UStackWidgetBase*> UIInstances;
	
#pragma endregion 
	
};
