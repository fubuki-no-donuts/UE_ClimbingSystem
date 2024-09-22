// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ObjectPoolItem.generated.h"

// Object State that is InUse or NotInUse
UENUM(BLueprintType)
namespace EObjectPoolState
{
	enum Type
	{
		InUse UMETA(DisplayName = "Object is InUse"),
		NotInUse UMETA(DisplayName = "Object is NotInUse")
	};
}

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UObjectPoolItem : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UE_ARPGSYSTEM_API IObjectPoolItem
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnCreate() = 0;	// Called when the object is created (if needed)
	virtual void OnGet() = 0;		// Called when the object is retrieved from the pool
	virtual void OnRelease() = 0;	// Called when the object is returned to the pool
	virtual void OnDestroy() = 0;	// Called when the object is destroyed (if needed)
};
