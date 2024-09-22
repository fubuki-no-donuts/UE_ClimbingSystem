// Fill out your copyright notice in the Description page of Project Settings.


#include "_ARPG/Subsystem/ObjectPooling/CustomObjectPool.h"
#include "_ARPG/Interfaces/ObjectPoolItem.h"

DEFINE_LOG_CATEGORY(LogCustomObjectPool)

UObject* UCustomObjectPool::GetPooledObject()
{
	if (GetAvailableObjectNum() <= 0)
	{
		return nullptr;
	}
	
	TScriptInterface<IObjectPoolItem> PooledObject = PooledObjects.Pop();

	// Call OnGet() when the object is retrieved from the pool
	PooledObject->OnGet();

	// Add to Active array
	ActiveObjects.Add(PooledObject);
	return PooledObject.GetObject();
}

void UCustomObjectPool::ReleasePooledObject(TScriptInterface<IObjectPoolItem> ObjectToRelease)
{
	if (ObjectToRelease.GetObject())
	{
		ActiveObjects.Remove(ObjectToRelease);

		// Call OnRelease() when the object is returned to the pool
		ObjectToRelease->OnRelease();

		PooledObjects.Add(ObjectToRelease);
	}
}

void UCustomObjectPool::InitializePool(TSubclassOf<UObject> ObjectClass, int32 InitialSize)
{
	PooledObjectClass = ObjectClass;
	MaxPoolSize = InitialSize;

	// Pre-create objects to fill the pool
	for (int32 i = 0; i < InitialSize; i++)
	{
		TScriptInterface<IObjectPoolItem> NewPooledObject = CreateNewPooledObject();
		if (NewPooledObject.GetObject())
		{
			PooledObjects.Add(NewPooledObject);
		}
	}
}

TScriptInterface<IObjectPoolItem> UCustomObjectPool::CreateNewPooledObject()
{
	if (GetTotalObjectNum() >= MaxPoolSize)
	{
		UE_LOG(LogCustomObjectPool, Warning, TEXT("This ObjectPool has already reached Max Limited Capacity!"));
		return nullptr;
	}
	
	if (PooledObjectClass)
	{
		UObject* NewPooledObject = NewObject<UObject>(this, PooledObjectClass);

		// Ensure the new object implements IObjectPoolItem
		if (IObjectPoolItem* PoolItem = Cast<IObjectPoolItem>(NewPooledObject))
		{
			// Call OnCreate when the object is created
			PoolItem->OnCreate();
		}
		
		return NewPooledObject;
	}
	
	return nullptr;
}
