// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/StackStateInterface.h"
#include "UIManager.generated.h"

class UStackWidgetBase;

// LayerLevel enum of UI
UENUM(BlueprintType)
enum ELayerLevel : uint8
{
	BottomLayer, // BottomLayer, HUD mostly, and It decides what widget to show that followed
	
	FirstLayer, // Opened from BottomLayer
	SecondLayer, // Opened from FirstLayer
	ThirdLayer, // Opened from SecondLayer
	
	TipLayer // For Notify, will automatically close and be destroyed
};

// Register Info of UI
USTRUCT(BlueprintType)
struct FRegisterInfoUI : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RegisterInfoUI")
	TSoftClassPtr<UStackWidgetBase> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RegisterInfoUI")
	TEnumAsByte<ELayerLevel> LayerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RegisterInfoUI")
	bool bInputOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RegisterInfoUI")
	bool bPreCreate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RegisterInfoUI")
	bool bDestroyOnPop;
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

	// BottomLayer, only one exist at a time
	UPROPERTY()
	UStackWidgetBase* UIContext;
	
	// Stack for UI
	UPROPERTY()
	TArray<TScriptInterface<IStackStateInterface>> UIStates;

	// Stack for Tips
	UPROPERTY()
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
	FORCEINLINE int32 GetStateCount() const { return UIStates.Num(); }
	
	UFUNCTION(BlueprintCallable, Category = "StateStateManager")
	FORCEINLINE const TScriptInterface<IStackStateInterface>& GetCurrentState() const { return UIStates.Last(); }

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
	UStackWidgetBase* OpenUI(FName UIName);

	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void CloseUI(FName UIName);
	
	UFUNCTION(BlueprintCallable, Category = "UIManager")
	void ReleaseAll();
	
private:
	// /Script/Engine.DataTable'/Game/_ARPG/Data/DT_UI_Info.DT_UI_Info'
	UPROPERTY()
	UDataTable* UIRegisterTable;
	
	UPROPERTY()
	TMap<FName, UStackWidgetBase*> UIInstances;
	
#pragma endregion 
	
};
