// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "_ARPG/BaseCharacter.h"
#include "UE_ARPGSystem/DebugHelper/DebugHelper.h"


UCustomMovementComponent::UCustomMovementComponent()
{
	// A default climbable surface type
	ClimbableSurfaceTraceTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
}

#pragma region OverridenFunctions

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f); // Set Height to Half of original height
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f); // Set Height to original height

		// Set Rotation Correct
		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation);

		StopMovementImmediately();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (IsClimbing())
	{
		PhysClimb(deltaTime, Iterations);
	}

	Super::PhysCustom(deltaTime, Iterations);
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	if (!IsClimbing())
	{
		return Super::GetMaxSpeed();
	}

	return MaxClimbSpeed;
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if (!IsClimbing())
	{
		return Super::GetMaxAcceleration();
	}

	return MaxClimbAcceleration;
}

#pragma endregion

#pragma region ClimbCore

void UCustomMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			StartClimbing(); // Enter Climbing State
		}
		else
		{
			Debug::Print(TEXT("Can NOT Start Climbing!"));
		}
	}
	else
	{
		StopClimbing(); // Exit Climbing State
	}
}

bool UCustomMovementComponent::CanStartClimbing()
{
	if (IsFalling() or
		!TraceClimbableSurfaces() or
		!TraceFromEyeHeight(100.f).bBlockingHit)
	{
		return false;
	}

	return true;
}

void UCustomMovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
}

void UCustomMovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}

void UCustomMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	/* Process all the climbable surfaces info */
	TraceClimbableSurfaces(); // Continue to find Climbable Surfaces while climbing
	ProcessClimbableSurfaceInfo(); // Handle the surfaces

	/* Check if we should stop climbing */
	if (CheckShouldStopClimbing())
	{
		StopClimbing();
	}

	RestorePreAdditiveRootMotionVelocity();

	if ( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		// Define the max climb speed and acceleration
		CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(deltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);

	// Handle Climb rotation
	SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(deltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		//adjust and try again
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if ( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

	/* Snap movement to climbable surfaces */
	SnapMovementToClimbableSurfaces(deltaTime);
}

void UCustomMovementComponent::ProcessClimbableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesTracedResults.IsEmpty())
	{
		return;
	}

	for (const FHitResult& TracedHitResult:ClimbableSurfacesTracedResults)
	{
		CurrentClimbableSurfaceLocation += TracedHitResult.ImpactPoint;
		CurrentClimbableSurfaceNormal += TracedHitResult.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= ClimbableSurfacesTracedResults.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();

	//Debug::Print(TEXT("CurrentClimbableSurfaceNormal = " + CurrentClimbableSurfaceNormal.ToCompactString()), FColor::Cyan, 1);
}

bool UCustomMovementComponent::CheckShouldStopClimbing()
{
	if (ClimbableSurfacesTracedResults.IsEmpty())
	{
		return true;
	}

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	if (DegreeDiff <= MaxClimbableDegree)
	{
		return true;
	}

	return false;
}

FQuat UCustomMovementComponent::GetClimbRotation(float DeltaTime)
{
	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();

	// Incase there is a animation root motion enabled,
	// so use inner quat here
	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentQuat;
	}

	// Otherwise calculate rotation manually
	// Rotate character face to climbable surface all the time
	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();

	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.f);
}

void UCustomMovementComponent::SnapMovementToClimbableSurfaces(float DeltaTime)
{
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	const FVector ProjectedCharacterToSurface = 
		(CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);

	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	// Auto move character towards to current climbable surface,
	// so that it looks like character snaps to surface all the time
	UpdatedComponent->MoveComponent(
		SnapVector * DeltaTime * MaxClimbSpeed,
		UpdatedComponent->GetComponentQuat(),
		true);
}

bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}

bool UCustomMovementComponent::TraceClimbableSurfaces()
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End, true);

	return !ClimbableSurfacesTracedResults.IsEmpty();
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;

	return DoLineTraceSingleByObject(Start, End);
}

#pragma endregion

#pragma region ClimbTraces

TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(
	const FVector& Start, const FVector& End,
    bool bShowDebugShape, bool bDrawPersistentShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None; 

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if(bDrawPersistentShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		ClimbCapsuleTraceRadius,
		ClimbCapsuleTraceHalfHeight,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutCapsuleTraceHitResults,
		false
	);

	return OutCapsuleTraceHitResults;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(
	const FVector& Start, const FVector& End,
	bool bShowDebugShape, bool bDrawPersistentShapes)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None; 

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if(bDrawPersistentShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutHit,
		false
	);

	return OutHit;
}

#pragma endregion

