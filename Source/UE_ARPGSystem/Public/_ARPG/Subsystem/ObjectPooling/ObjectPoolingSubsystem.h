// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ObjectPoolingSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogObjectPoolingSystem, Log, All)

class UCustomObjectPool;

/**
 * 
 */
UCLASS()
class UE_ARPGSYSTEM_API UObjectPoolingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ObjectPoolingSystem")
	UCustomObjectPool* CreatePool(FString PoolName);

	UFUNCTION(BlueprintCallable, Category = "ObjectPoolingSystem")
	UCustomObjectPool* GetPool(FString PoolName);

	UFUNCTION(BlueprintCallable, Category = "ObjectPoolingSystem")
	void DestroyPool(FString PoolName);
	
private:
	/* All ObjectPooling that created in this GameInstance */
	TMap<FString, TWeakObjectPtr<UCustomObjectPool>> PoolMap;
};
