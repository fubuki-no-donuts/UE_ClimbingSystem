// Fill out your copyright notice in the Description page of Project Settings.


#include "_ARPG/UI/StackUserWidget.h"

#include "_ARPG/Subsystem/StateMachine/StackState.h"

void UStackUserWidget::EnterState(EStackAction::Type StackAction)
{
	switch (StackAction)
	{
	case EStackAction::STATE_Push:
		{
			AddToViewport();
			break;
		}
	case EStackAction::STATE_Pop:
		{
			SetVisibility(ESlateVisibility::Visible);
			break;
		}
	}
}

void UStackUserWidget::ExitState(EStackAction::Type StackAction)
{
	switch (StackAction)
	{
	case EStackAction::STATE_Push:
		{
			SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	case EStackAction::STATE_Pop:
		{
			RemoveFromParent();
			break;
		}
	}
}

void UStackUserWidget::UpdateState(float DeltaTime)
{
	UE_LOG(LogStackState, Display, TEXT("Update Widget: %s"), *GetName());
}
