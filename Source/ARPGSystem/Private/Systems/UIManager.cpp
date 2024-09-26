// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/UIManager.h"


void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitUI();
}

void UUIManager::Deinitialize()
{
	Super::Deinitialize();
}

void UUIManager::PushState(TScriptInterface<IStackStateInterface> NewState)
{
	// Pop Old State
	if (GetStateCount() > 0)
	{
		auto CurrentState = GetCurrentState();
		if (CurrentState)
		{
			CurrentState->ExitState(EStackAction::Push);
		}
	}

	// Push New State
	States.Push(NewState);
	NewState->EnterState(EStackAction::Push);

	// Broadcast New State Pushed
	StatePushed(NewState);
}

void UUIManager::PopState()
{
	// No Valid State to pop
	if (GetStateCount() < 0)
	{
		return ;
	}

	auto OldState = GetCurrentState();
	if (OldState)
	{
		OldState->ExitState(EStackAction::Pop);
	}
	States.Pop();

	if (GetStateCount() > 0)
	{
		GetCurrentState()->EnterState(EStackAction::Pop);
	}

	StatePopped(OldState);
}

void UUIManager::PopStates(int32 Count)
{
	int32 ValidCount = FMath::Min(Count, GetStateCount());

	while(ValidCount-- > 0)
	{
		PopState();
	}
}

void UUIManager::PopAllStates()
{
	PopStates(GetStateCount());
}

void UUIManager::StatePushed(TScriptInterface<IStackStateInterface> PushedState)
{
	if (OnStatePushed.IsBound())
	{
		OnStatePushed.Broadcast(PushedState);
	}
}

void UUIManager::StatePopped(TScriptInterface<IStackStateInterface> PoppedState)
{
	if (OnStatePopped.IsBound())
	{
		OnStatePopped.Broadcast(PoppedState);
	}
}

void UUIManager::InitUI()
{
	UIClassTable = LoadObject<UDataTable>(this, TEXT("/Game/_ARPG/Data/DT_UI_Info.DT_UI_Info"));
	if (UIClassTable)
	{
		for (FName RowName : UIClassTable->GetRowNames())
		{
			UE_LOG(LogTemp, Display, TEXT("%s"), *RowName.ToString());
		}
	}
	
	// TODO: 提前创建并初始化
}

UStackWidgetBase* UUIManager::OpenUI(FName UIName)
{
	// TODO: 先去实例中找, 没有就创建并入栈
	return nullptr;
}

void UUIManager::CloseUI(FName UIName)
{
	// TODO: 先去实例中找, 找到了就出栈直至该UI也出栈
}

void UUIManager::ReleaseAll()
{
	// TODO: 释放全部UI实例
}
