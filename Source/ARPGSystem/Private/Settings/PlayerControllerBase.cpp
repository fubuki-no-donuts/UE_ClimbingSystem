// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/PlayerControllerBase.h"

#include "Systems/UIManager.h"
#include "UI/StackWidgetBase.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance NOT Valid!"));
		return ;
	}
	
	UUIManager* UIManager = GameInstance->GetSubsystem<UUIManager>();
	if (UIManager == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManager NOT Valid!"));
		return ;
	}
	
	UIManager->OpenUI(TEXT("UI_HUD_Main"));
}
