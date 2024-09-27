// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/UIManager.h"

#include "Blueprint/UserWidget.h"
#include "UI/StackWidgetBase.h"
#include "Misc/ConfigCacheIni.h"

void UUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UIRegisterTable = LoadObject<UDataTable>(this, TEXT("/Game/_ARPG/Data/DT_RegisterInfo_UI.DT_RegisterInfo_UI"));
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
	UIStates.Push(NewState);
	NewState->EnterState(EStackAction::Push);

	// Broadcast New State Pushed
	StatePushed(NewState);
}

void UUIManager::PopState()
{
	// No Valid State to pop
	if (GetStateCount() < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Valid State to Pop up!"));
		return ;
	}

	auto OldState = GetCurrentState();
	if (OldState)
	{
		OldState->ExitState(EStackAction::Pop);
	}
	UIStates.Pop();

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

UStackWidgetBase* UUIManager::OpenUI(FName UIName)
{
	auto LocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (LocalPlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalPlayerController NOT Valid!"));
		return nullptr;
	}
	
	// Exist?
	auto TryFoundInstance = UIInstances.Find(UIName);
	if (TryFoundInstance)
	{
		return *TryFoundInstance;
	}

	// Nope
	if (UIRegisterTable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIRegisterTable NOT Valid!"));
		return nullptr;
	}
	
	FRegisterInfoUI* FoundInfo = UIRegisterTable->FindRow<FRegisterInfoUI>(UIName, TEXT("UIRegisterTable"));
	if (FoundInfo == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIName: %s NOT Valid!"), *UIName.ToString());
		return nullptr;
	}

	UClass* LoadWidgetClass = FoundInfo->WidgetClass.LoadSynchronous();
	if (LoadWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIName: %s WidgetClass NOT Valid!"), *UIName.ToString());
		return nullptr;
	}
	
	auto NewUI = CreateWidget<UStackWidgetBase>(
		LocalPlayerController,
		LoadWidgetClass);
	if (NewUI)
	{
		PushState(NewUI); // If a NewState is created, it must be pushed into stack.
		UIInstances.Add(UIName, NewUI);
		return NewUI;
	}

	UE_LOG(LogTemp, Warning, TEXT("UIName: %s CreateWidget Failed!"), *UIName.ToString());
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
