// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/StackWidgetBase.h"

void UStackWidgetBase::EnterState(EStackAction InStackAction)
{
	switch (InStackAction)
	{
	case EStackAction::Push:
		{
			AddToViewport();
			break;
		}
	case EStackAction::Pop:
		{
			SetVisibility(ESlateVisibility::Visible);
			break;
		}
	}
	ReceiveEnterState(InStackAction);
}

void UStackWidgetBase::ExitState(EStackAction InStackAction)
{
	switch (InStackAction)
	{
	case EStackAction::Push:
		{
			SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	case EStackAction::Pop:
		{
			RemoveFromParent();
			break;
		}
	}
	ReceiveExitState(InStackAction);
}

void UStackWidgetBase::UpdateState(float DeltaTime)
{
	ReceiveUpdateState(DeltaTime);
}
