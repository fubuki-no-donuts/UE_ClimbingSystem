// Fill out your copyright notice in the Description page of Project Settings.


#include "_ARPG/Components/StackStateMachineComponent.h"

DEFINE_LOG_CATEGORY(LogStackStateMachine);

// Sets default values for this component's properties
UStackStateMachineComponent::UStackStateMachineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStackStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UStackStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (GetStateCount() > 0 && GetCurrentState() != nullptr)
	{
		GetCurrentState()->UpdateState(DeltaTime);
	}
}

void UStackStateMachineComponent::PushState(TScriptInterface<IStackStateInterface> NewState)
{
	if (GetStateCount() > 0)
	{
		auto CurrentState = GetCurrentState();
		if (CurrentState != nullptr)
		{
			CurrentState->ExitState(EStackAction::STATE_Push); // Exit old one
		}
	}
	
	States.Push(NewState);
	
	CurrentStateTime = 0.f;
	NewState->EnterState(EStackAction::STATE_Push);

	StatePushed(NewState);
}

void UStackStateMachineComponent::PopState()
{
	if (GetStateCount() <= 0)
	{
		UE_LOG(LogStackStateMachine, Warning, TEXT("There are no states in this StackStateMachine!"));
		return;
	}

	auto OldState = GetCurrentState();
	if (OldState != nullptr)
	{
		OldState->ExitState(EStackAction::STATE_Pop);
	}
	
	States.Pop();
	
	if (GetStateCount() > 0)
	{
		CurrentStateTime = 0.f;
		GetCurrentState()->EnterState(EStackAction::STATE_Push);
	}
	
	StatePopped(OldState);
}

void UStackStateMachineComponent::PopStates(int Count)
{
	if (GetStateCount() <= 0)
	{
		UE_LOG(LogStackStateMachine, Warning, TEXT("There are no states in this StackStateMachine!"));
		return;
	}

	if (GetStateCount() < Count)
	{
		UE_LOG(LogStackStateMachine, Warning, TEXT("Trying to pop more states than available. Popping all remaining states."));
		Count = GetStateCount();
	}
	
	for (int Index = Count; Index > 0; --Index)
	{
		PopState();
	}
}

void UStackStateMachineComponent::PopAllStates()
{
	if (GetStateCount() <= 0)
	{
		UE_LOG(LogStackStateMachine, Warning, TEXT("There are no states in this StackStateMachine!"));
		return;
	}
	
	PopStates(GetStateCount());
}

void UStackStateMachineComponent::LoopState(float DeltaTime)
{
	GetCurrentState()->UpdateState(DeltaTime);
}

void UStackStateMachineComponent::StatePushed(TScriptInterface<IStackStateInterface> PushedState)
{
	UE_LOG(LogStackStateMachine, Display, TEXT("Pushed State: %s"), *PushedState.GetObject()->GetName());

	if (OnStatePushed.IsBound())
	{
		OnStatePopped.Broadcast(PushedState);
	}
}

void UStackStateMachineComponent::StatePopped(TScriptInterface<IStackStateInterface> PoppedState)
{
	UE_LOG(LogStackStateMachine, Display, TEXT("Popped State: %s"), *PoppedState.GetObject()->GetName());

	if (OnStatePopped.IsBound())
	{
		OnStatePopped.Broadcast(PoppedState);
	}
}

