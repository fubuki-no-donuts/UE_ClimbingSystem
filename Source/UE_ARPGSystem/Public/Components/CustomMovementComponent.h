// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climb UMETA(DisplayName = "Climb Mode")
	};
}

/**
 * 
 */
UCLASS()
class UE_ARPGSYSTEM_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCustomMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region ClimbCore

private:

	/**
	 * @brief Gather Climbable Surfaces
	 *
	 * @return bool	Return true if there is at least one climbable surface, and Return false if there is none.
	 */
	bool TraceClimbableSurfaces();

	/**
	 * @brief Performs a line trace to detect is there a surface that blocked sight
	 *
	 * @param TraceDistance			The Length of the trace line
	 * @param TraceStartOffset		The Offset from the eye's height
	 * @return FHitResult			Return the hit result
	 */
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f);

	/**
	 * @brief Return a bool that shows owner can or can not start climbing
	 *
	 * @return bool 
	 */
	bool CanStartClimbing();

	/**
	 * @brief Start Climbing
	 */
	void StartClimbing();

	/**
	 * @brief Stop Climbing
	 */
	void StopClimbing();

protected:

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

#pragma endregion

#pragma region ClimbTraces

private:

	/**
	 * @brief Performs a trace to detect climbable surfaces in the environment.
	 * 
	 * @param Start							Where trace starts
	 * @param End							Where trace ends
	 * @param bShowDebugShape				Whether to show the trace's debug shape
	 * @param bDrawPersistentShapes			Continue to Draw
	 * @return TArray<FHitResult>			Return array of hit results
	 */
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End,
	    bool bShowDebugShape = false, bool bDrawPersistentShapes = false);

	/**
	 * @brief Performs a line trace at eye height to decide whether to climb
	 *
	 * @param Start Where trace starts
	 * @param End Where trace ends
	 * @param bShowDebugShape Whether to show the trace's debug shape
	 * @param bDrawPersistentShapes Continue to Draw
	 * @return FHitResult Return the hit results
	 */
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, 
		bool bShowDebugShape = false, bool bDrawPersistentShapes = false);

#pragma endregion

#pragma region ClimbCoreVariable

private:

	/* Climbable Surfaces that detected */
	TArray<FHitResult> ClimbableSurfacesTracedResults;

#pragma endregion

#pragma region ClimbBPVariable

private:

	/* Array of Climbable Surface Types */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbableSurfaceTraceTypes;

	/* Capsule Trace Radius */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceRadius = 50.f;

	/* Capsule Trace Half Height */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float ClimbCapsuleTraceHalfHeight = 72.f;

#pragma endregion

public:

	/**
	 * @brief Toggle Climbing Movement
	 *
	 * @param bEnableClimb Whether to enable climbing
	 */
	void ToggleClimbing(bool bEnableClimb);

	/**
	 * @brief 是否处于攀爬状态
	 *
	 * @return bool 角色处于攀爬状态则为true，非攀爬状态则为false
	 */
	bool IsClimbing() const;
};
