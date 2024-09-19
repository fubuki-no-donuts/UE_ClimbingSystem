// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "_ARPG/Interfaces/StackStateInterface.h"
#include "StackStateMachineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStackStatePushPopSignature, TScriptInterface<IStackStateInterface>, State);

UCLASS( Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE_ARPGSYSTEM_API UStackStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStackStateMachineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* State Management */
public:
	UFUNCTION(BlueprintCallable, Category = "StackStateMachine")
	void PushState(TScriptInterface<IStackStateInterface> NewState);

	UFUNCTION(BlueprintCallable, Category = "StackStateMachine")
	void PopState();

	UFUNCTION(BlueprintCallable, Category = "StackStateMachine")
	void PopStates(int Count);

	UFUNCTION(BlueprintCallable, Category = "StackStateMachine")
	void PopAllStates();

	UFUNCTION(BlueprintCallable, Category = "StackStateMachine")
	void LoopState(float DeltaTime);

	
	/* Dynamic Delegate */
protected:
	UPROPERTY(BlueprintAssignable, Category = "StackStateMachine")
	FOnStackStatePushPopSignature OnStatePushed;
	
	UPROPERTY(BlueprintAssignable, Category = "StackStateMachine")
	FOnStackStatePushPopSignature OnStatePopped;

	virtual void StatePushed(TScriptInterface<IStackStateInterface> PushedState);
	virtual void StatePopped(TScriptInterface<IStackStateInterface> PoppedState);
	
	/* States */
protected:
	TArray<TScriptInterface<IStackStateInterface>> States;

	UPROPERTY(VisibleAnywhere)
	float CurrentStateTime = 0.f;

	/* Helper Functions */
public:
	UFUNCTION(BlueprintPure, Category = "StackStateMachine")
	FORCEINLINE int32 GetStateCount() const { return States.Num(); }

	UFUNCTION(BlueprintPure, Category = "StackStateMachine")
	FORCEINLINE float GetCurrentStateTime() const { return CurrentStateTime; }
	
	UFUNCTION(BlueprintPure, Category = "StackStateMachine")
	FORCEINLINE const TScriptInterface<IStackStateInterface>& GetCurrentState() const { return States.Last(); }
	
	UFUNCTION(BlueprintPure, Category = "StackStateMachine")
	TScriptInterface<IStackStateInterface> GetState(int32 Index)
	{
		if (!States.IsValidIndex(Index))
		{
			return TScriptInterface<IStackStateInterface>();
		}
	
		return States[Index];
	}
};
