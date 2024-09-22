// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CustomObjectPool.generated.h"

class IObjectPoolItem;

DECLARE_LOG_CATEGORY_EXTERN(LogCustomObjectPool, Log, All);

/**
 * 
 */
UCLASS()
class UE_ARPGSYSTEM_API UCustomObjectPool : public UObject
{
	GENERATED_BODY()

public:
	// Get an available object from the pool
	UFUNCTION(BlueprintCallable, Category = "ObjectPooling")
	UObject* GetPooledObject();

	// Release an object back to the pool
	UFUNCTION(BlueprintCallable, Category = "ObjectPooling")
	void ReleasePooledObject(TScriptInterface<IObjectPoolItem> ObjectToRelease);

	// Initialize the pool with a specific object type and size
	UFUNCTION(BlueprintCallable, Category = "ObjectPooling")
	void InitializePool(TSubclassOf<UObject> ObjectClass, int32 InitialSize);
	
protected:
	// Helper function to create a new object for the pool
	TScriptInterface<IObjectPoolItem> CreateNewPooledObject();
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ObjectPooling", meta = (AllowPrivateAccess = "true"))
	int32 MaxPoolSize = 10000;
	
	// Class of the objects that the pool manages (must implement IObjectPoolItem)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ObjectPooling", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UObject> PooledObjectClass;

	// Pool of available objects (implements IObjectPoolItem)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPooling", meta = (AllowPrivateAccess = "true"))
	TArray<TScriptInterface<IObjectPoolItem>> PooledObjects;

	// Store objects that are currently in use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPooling", meta = (AllowPrivateAccess = "true"))
	TArray<TScriptInterface<IObjectPoolItem>> ActiveObjects;

public:
	UFUNCTION(BlueprintCallable, Category = "ObjectPooling")
	FORCEINLINE int32 GetMaxPoolSize() const { return MaxPoolSize; }

	UFUNCTION(BlueprintCallable, Category = "ObjectPooling")
	FORCEINLINE int32 GetAvailableObjectNum() const { return PooledObjects.Num(); }

	UFUNCTION(BlueprintCallable, Category = "ObjectPooling")
	FORCEINLINE int32 GetTotalObjectNum() const { return PooledObjects.Num() + ActiveObjects.Num(); }
};
