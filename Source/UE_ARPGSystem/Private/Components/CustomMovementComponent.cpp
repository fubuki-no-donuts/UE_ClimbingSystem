// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "_ARPG/BaseCharacter.h"
#include "UE_ARPGSystem/DebugHelper/DebugHelper.h"


UCustomMovementComponent::UCustomMovementComponent()
{
	// A default climbable surface type
	ClimbableSurfaceTraceTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
}

void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
		OwningPlayerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
	}

	OwningPlayerCharacter = Cast<ABaseCharacter>(CharacterOwner);
}

#pragma region OverridenFunctions

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Test
	FVector Test1;
	FVector Test2;
	CanStartVaulting(Test1, Test2, true);
	CanStartClimbing(true);
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// Enter Climbing
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f); // Set Height to Half of original height

		OnEnterClimbStateDelegate.ExecuteIfBound();
	}

	// Exit Climbing
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f); // Set Height to original height

		// Set Rotation Correct
		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation);

		StopMovementImmediately();

		OnExitClimbStateDelegate.ExecuteIfBound();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	// Handle Climbing Phys
	if (IsClimbing())
	{
		PhysClimb(deltaTime, Iterations);
	}

	Super::PhysCustom(deltaTime, Iterations);
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}

	return Super::GetMaxSpeed();
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}

	return Super::GetMaxAcceleration();
}

FVector UCustomMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity,
	const FVector& CurrentVelocity) const
{
	// Use Root Motion Velocity instead if the playing montage has valid root motion
	if (IsFalling() && OwningPlayerAnimInstance && OwningPlayerAnimInstance->IsAnyMontagePlaying())
	{
		return RootMotionVelocity;
	}
	else
	{
		return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
	}
}

#pragma endregion

#pragma region ClimbCore

void UCustomMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			PlayClimbMontage(IdleToClimbMontage);
		}
		else if (CanStartClimbingDown())
		{
			PlayClimbMontage(ClimbToDownMontage);
		}
		else
		{
			TryStartVaulting();
		}
	}

	if (!bEnableClimb)
	{
		StopClimbing(); // Exit Climbing State
	}
}

void UCustomMovementComponent::RequestHopping()
{
	const FVector UnrotatedLastInputVector = 
		UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), GetLastInputVector());

	// Just Press Space bar, hop up by default
	if (UnrotatedLastInputVector == FVector::ZeroVector)
	{
		HandleHopUp();
		return ;
	}

	const float DotResult = 
		FVector::DotProduct(UnrotatedLastInputVector.GetSafeNormal(), FVector::UpVector);

	if (DotResult >= 0.9f)
	{
		HandleHopUp();
	}
	else if (DotResult <= -0.9f)
	{
		HandleHopDown();
	}
	else
	{

	}
}

bool UCustomMovementComponent::CanStartClimbing(bool bShowDebugShape)
{
	if (IsFalling() or
		!TraceClimbableSurfaces(bShowDebugShape) or
		!TraceFromEyeHeight(100.f, 0, bShowDebugShape).bBlockingHit)
	{
		return false;
	}

	return true;
}

bool UCustomMovementComponent::CanStartClimbingDown(bool bShowDebugShape)
{
	// Climbing up is priority
	if (IsFalling() || CanStartClimbing())
	{
		return false;
	}

	const float HalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector ActorLocation = GetActorLocation() + UpdatedComponent->GetForwardVector() * MaximumHeightToReach;
	const FVector FirstStart = ActorLocation - FVector(0, 0, HalfHeight);

	// Height need to be higher than the character,
	// otherwise the height is no need to climb on so there is no need to climb down either.
	// It is same as the condition to start climbing up.
	// The most important: it is assumed here that there is a complete plane below the edge and there is no concave defect.
	const FVector FirstEnd = FirstStart - FVector(0, 0, MaximumHeightToReach + HalfHeight * 2); 

	FHitResult FirstHit = DoLineTraceSingleByObject(FirstStart, FirstEnd, bShowDebugShape);

	if (FirstHit.IsValidBlockingHit())
	{
		return false;
	}

	const FVector SecondStart = FirstEnd;
	const FVector SecondEnd = SecondStart - UpdatedComponent->GetForwardVector() * (MaximumHeightToReach);

	FHitResult SecondHit = DoLineTraceSingleByObject(SecondStart, SecondEnd, bShowDebugShape);

	if (SecondHit.IsValidBlockingHit())
	{
		return true;
	}

	return false;
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
	if (CheckShouldStopClimbing() || CheckHasReachedFloor())
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

	if (CheckHasReachedLedge())
	{
		PlayClimbMontage(ClimbToTopMontage);
	}
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

// Check Climbable Wall
bool UCustomMovementComponent::CheckShouldStopClimbing()
{
	if (ClimbableSurfacesTracedResults.IsEmpty())
	{
		return true;
	}

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	if (DegreeDiff <= MaxClimbableDegree) // small degree can not consider a wall
	{
		return true;
	}

	return false;
}

// Check UnClimbable
bool UCustomMovementComponent::CheckHasReachedFloor(bool bShowDebugShape)
{
	const FVector DownVector = -UpdatedComponent->GetUpVector();
	const FVector StartOffset = DownVector * MinimumHeightToClimb;

	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + DownVector;

	TArray<FHitResult> PossibleFloorHits = DoCapsuleTraceMultiByObject(Start, End, bShowDebugShape);

	if (PossibleFloorHits.IsEmpty())
	{
		return false;
	}

	for (const FHitResult& PossibleFloorHit : PossibleFloorHits)
	{
		bool bCompleteFlatFloorReached = FVector::Parallel(-PossibleFloorHit.ImpactNormal, FVector::UpVector);

		const float DotFloorNormalAndUpVector = FVector::DotProduct(PossibleFloorHit.ImpactNormal, FVector::UpVector);

		if (DotFloorNormalAndUpVector > 0 &&  // Floor's normal need to go up
			FMath::RadiansToDegrees(FMath::Acos(DotFloorNormalAndUpVector)) <= MaxClimbableDegree && // Large degree should consider as wall, not floor
			//bCompleteFlatFloorReached && 
			GetUnrotatedClimbVelocity().Z < -10.f)
		{
			return true;
		}
	}
	return false;
}

// Check ledge
bool UCustomMovementComponent::CheckHasReachedLedge(bool bShowDebugShape)
{
	const FHitResult FirstHit = TraceFromEyeHeight((MaximumHeightToReach * 2), MaximumHeightToReach, bShowDebugShape);

	if (FirstHit.IsValidBlockingHit())
	{
		return false;
	}

	FVector SecondStart = FirstHit.TraceEnd;
	FVector SecondEnd = SecondStart - UpdatedComponent->GetUpVector() * (MaximumHeightToReach * 2); 
	const FHitResult SecondHit = DoLineTraceSingleByObject(SecondStart, SecondEnd, bShowDebugShape);

	// TODO: Check with dot product
	if (SecondHit.IsValidBlockingHit() && GetUnrotatedClimbVelocity().Z > 10.f)
	{
		return true;
	}

	return false;
}

void UCustomMovementComponent::TryStartVaulting()
{
	FVector VaultStartPosition;
	FVector VaultLandPosition;

	if (CanStartVaulting(VaultStartPosition, VaultLandPosition))
	{
		// Start Vaulting
		SetMotionWarpTarget(FName("VaultStartPoint"), VaultStartPosition);
		SetMotionWarpTarget(FName("VaultLandPoint"), VaultLandPosition);

		StartClimbing(); // Set movement mode to Climbing, so gravity and others are handled
		PlayClimbMontage(VaultMontage);
	}
}

bool UCustomMovementComponent::CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition, bool bShowDebugShape)
{
	if (IsFalling())
	{
		return false;
	}

	OutVaultStartPosition = FVector::ZeroVector;
	OutVaultLandPosition = FVector::ZeroVector;

	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector UpVector = UpdatedComponent->GetUpVector();
	const FVector DownVector = - UpdatedComponent->GetUpVector();

	for (int32 i = 0; i< 5; i++)
	{
		const FVector Start = ComponentLocation + UpVector * 90.f + ComponentForward * 100.f * (i+1);
		const FVector End = Start + DownVector * 100.f * (i+1);

		FHitResult VaultTraceHit = DoLineTraceSingleByObject(Start, End, bShowDebugShape);

		if (VaultTraceHit.IsValidBlockingHit())
		{
			if (i == 0)
			{
				OutVaultStartPosition = VaultTraceHit.ImpactPoint;
			}

			if (i == 3)
			{
				OutVaultLandPosition = VaultTraceHit.ImpactPoint;
			}
		}
	}

	Debug::Print(TEXT("OutVaultStartPosition: " + OutVaultStartPosition.ToCompactString()), FColor::Cyan, 1);
	Debug::Print(TEXT("OutVaultStartPosition: " + OutVaultLandPosition.ToCompactString()), FColor::Green, 2);

	if (OutVaultStartPosition != FVector::ZeroVector && OutVaultLandPosition != FVector::ZeroVector)
	{
		return true;
	}
	else
	{
		return false;
	}
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

void UCustomMovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay || !OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying())
	{
		return ;
	}

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UCustomMovementComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == IdleToClimbMontage || Montage == ClimbToDownMontage)
	{
		StartClimbing();
		StopMovementImmediately(); // Clear Root Motion speed
	}

	if (Montage == ClimbToTopMontage || Montage == VaultMontage)
	{
		SetMovementMode(MOVE_Walking);
	}
}

void UCustomMovementComponent::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition)
{
	if (!OwningPlayerCharacter)
	{
		return ;
	}

	OwningPlayerCharacter->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(InWarpTargetName, InTargetPosition);
}

bool UCustomMovementComponent::CheckCanHopUp(FVector& OutHopUpTargetPosition, bool bShowDebugShape)
{
	FHitResult HopUpHit = TraceFromEyeHeight(100.f, MaximumHeightToReach, bShowDebugShape);
	// In case character is too close to ledge, then character cant hop up
	FHitResult SaftyLedgeHit = TraceFromEyeHeight(100.f, 150.f, bShowDebugShape);

	if (HopUpHit.IsValidBlockingHit() && SaftyLedgeHit.IsValidBlockingHit())
	{
		OutHopUpTargetPosition = HopUpHit.ImpactPoint;
		return true;
	}

	return false;
}

void UCustomMovementComponent::HandleHopUp()
{
	FVector HopUpTargetPoint;

	if (CheckCanHopUp(HopUpTargetPoint))
	{
		SetMotionWarpTarget(FName("HopUpTargetPoint"), HopUpTargetPoint);
		PlayClimbMontage(HopUpMontage);
	}
}

bool UCustomMovementComponent::CheckCanHopDown(FVector& OutHopDownTargetPosition, bool bShowDebugShape)
{
	FHitResult HopDownHit = TraceFromEyeHeight(100.f, - 300, true, true);

	if (HopDownHit.IsValidBlockingHit())
	{
		OutHopDownTargetPosition = HopDownHit.ImpactPoint;
		return true;
	}

	return false;
}

void UCustomMovementComponent::HandleHopDown()
{
	FVector HopDownTargetPoint;

	if (CheckCanHopDown(HopDownTargetPoint))
	{
		SetMotionWarpTarget(FName("HopDownTargetPoint"), HopDownTargetPoint);
		PlayClimbMontage(HopDownMontage);
	}
}

bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}

FVector UCustomMovementComponent::GetUnrotatedClimbVelocity() const
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), Velocity);
}

bool UCustomMovementComponent::TraceClimbableSurfaces(bool bShowDebugShape, bool bDrawPersistentShapes)
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End, bShowDebugShape, bDrawPersistentShapes);

	return !ClimbableSurfacesTracedResults.IsEmpty();
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset, bool bShowDebugShape, bool bDrawPersistentShapes)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);
	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;

	return DoLineTraceSingleByObject(Start, End, bShowDebugShape, bDrawPersistentShapes);
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

