// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

DECLARE_DELEGATE(FOnEnterClimbState)
DECLARE_DELEGATE(FOnExitClimbState)

UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climb UMETA(DisplayName = "Climb Mode")
	};
}

class UAnimMontage;
class UAnimInstance;
class ABaseCharacter;

/**
 * 
 */
UCLASS()
class UE_ARPGSYSTEM_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCustomMovementComponent();

#pragma region OverridenFunctions

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual float GetMaxSpeed() const override;

	virtual float GetMaxAcceleration() const override;

	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;

#pragma endregion

#pragma region Delegate

public:
	FOnEnterClimbState OnEnterClimbStateDelegate;
	FOnExitClimbState OnExitClimbStateDelegate;

#pragma endregion


#pragma region Getter & Setter

public:
	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }

#pragma endregion

#pragma region ClimbCore

public:

	/**
	 * @brief Toggle Climbing Movement
	 *
	 * @param bEnableClimb		Whether to enable climbing
	 */
	void ToggleClimbing(bool bEnableClimb);

	/**
	 * @brief Request hopping movement
	 */
	void RequestHopping();

	/**
	 * @brief Check if owner is climbing now
	 *
	 * @return bool			Return true if owner is climbing, otherwise return false
	 */
	bool IsClimbing() const;

	FVector GetUnrotatedClimbVelocity() const;

private:

	/**
	 * @brief Gather Climbable Surfaces
	 *
	 * @return bool	Return true if there is at least one climbable surface, and Return false if there is none.
	 */
	bool TraceClimbableSurfaces(bool bShowDebugShape = false, bool bDrawPersistentShapes = false);

	/**
	 * @brief Performs a line trace to detect is there a surface that blocked sight
	 *
	 * @param TraceDistance			The Length of the trace line
	 * @param TraceStartOffset		The Offset from the eye's height
	 * @return FHitResult			Return the hit result
	 */
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f, 
		bool bShowDebugShape = false, bool bDrawPersistentShapes = false);

	/**
	 * @brief Return a bool that shows owner can or can not start climbing
	 *
	 * @return bool 
	 */
	bool CanStartClimbing(bool bShowDebugShape = false);

	/**
	 * @brief Return a bool that shows owner can or can not climbing down 
	 *
	 * @return bool
	 */
	bool CanStartClimbingDown(bool bShowDebugShape = false);

	/**
	 * @brief Start Climbing
	 *
	 */
	void StartClimbing();

	/**
	 * @brief Stop Climbing
	 *
	 */
	void StopClimbing();

	/**
	 * @brief Handle Climbing Physic
	 *
	 */
	void PhysClimb(float deltaTime, int32 Iterations);

	/**
	 * @breif Process Climbable Surface Info
	 *
	 */
	void ProcessClimbableSurfaceInfo();

	/**
	 * @brief Should Character Stop Climbing
	 *
	 * @return bool			Return true if character should stop climbing
	 */
	bool CheckShouldStopClimbing();

	/**
	 * @brief Reached floor or not
	 *
	 * @return bool			Return true if reached floor
	 */
	bool CheckHasReachedFloor(bool bShowDebugShape = false);

	/**
	 * @brief Reached ledge
	 *
	 * @return bool			Return true if reached ledge
	 */
	bool CheckHasReachedLedge(bool bShowDebugShape = false);

	void TryStartVaulting();

	bool CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition, bool bShowDebugShape = false);

	/**
	 * @brief Return the climb rotation
	 *
	 * @param DeltaTime		Used for interpolation
	 * @return FQuat		The Rotation while Climbing
	 */
	FQuat GetClimbRotation(float DeltaTime);

	/**
	 * @brief Snap Character's movement to climbable surface
	 *
	 * @param DeltaTime		Used for interpolation
	 */
	void SnapMovementToClimbableSurfaces(float DeltaTime);

	/**
	 * @brief Play the Input Montage
	 *
	 * @param MontageToPlay			The montage need to play
	 */
	void PlayClimbMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition);

	bool CheckCanHopUp(FVector& OutHopUpTargetPosition, bool bShowDebugShape = false);
	void HandleHopUp();

	bool CheckCanHopDown(FVector& OutHopDownTargetPosition, bool bShowDebugShape = false);
	void HandleHopDown();


#pragma endregion

#pragma region ClimbCoreVariable

private:

	/* Climbable Surfaces that detected */
	TArray<FHitResult> ClimbableSurfacesTracedResults;

	FVector CurrentClimbableSurfaceLocation;

	FVector CurrentClimbableSurfaceNormal;

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	UPROPERTY()
	ABaseCharacter* OwningPlayerCharacter;

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
	 * @param Start						Where trace starts
	 * @param End						Where trace ends
	 * @param bShowDebugShape			Whether to show the trace's debug shape
	 * @param bDrawPersistentShapes		Continue to Draw
	 * @return FHitResult				Return the hit results
	 */
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, 
		bool bShowDebugShape = false, bool bDrawPersistentShapes = false);

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

	/*  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float MaxClimbableDegree = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float MinimumHeightToClimb = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	float MaximumHeightToReach = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	UAnimMontage* IdleToClimbMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToTopMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToDownMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	UAnimMontage* VaultMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HopUpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing",
		meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HopDownMontage;

#pragma endregion

};
