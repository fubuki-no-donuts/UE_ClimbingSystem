// Fill out your copyright notice in the Description page of Project Settings.


#include "_ARPG/Subsystem/ObjectPooling/ObjectPoolingSubsystem.h"
#include "_ARPG/Subsystem/ObjectPooling/CustomObjectPool.h"

DEFINE_LOG_CATEGORY(LogObjectPoolingSystem)

UCustomObjectPool* UObjectPoolingSubsystem::CreatePool(FString PoolName)
{
	UE_LOG(LogObjectPoolingSystem, Display, TEXT("Try to Create ObjectPool named: %s"), *PoolName);

	/* Check if exist */
	auto ObjectPoolFound = GetPool(PoolName);
	if (ObjectPoolFound != nullptr)
	{
		return ObjectPoolFound;
	}

	/* otherwise Create a new one */
	auto ObjectPoolNew = NewObject<UCustomObjectPool>(this, UCustomObjectPool::StaticClass());
	if (ObjectPoolNew)
	{
		PoolMap.Add(PoolName, ObjectPoolNew);
	}

	return ObjectPoolNew;
}

UCustomObjectPool* UObjectPoolingSubsystem::GetPool(FString PoolName)
{
	/* Check if there is one with same name and Valid */
	TWeakObjectPtr<UCustomObjectPool>* FoundPool = PoolMap.Find(PoolName);
	if (FoundPool != nullptr && FoundPool->IsValid())
	{
		return FoundPool->Get();
	}

	/* Not found */
	return nullptr;
}

void UObjectPoolingSubsystem::DestroyPool(FString PoolName)
{

}
