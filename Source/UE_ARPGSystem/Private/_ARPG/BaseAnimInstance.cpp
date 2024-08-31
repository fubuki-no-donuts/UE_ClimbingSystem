// Fill out your copyright notice in the Description page of Project Settings.


#include "_ARPG/BaseAnimInstance.h"
#include "_ARPG/BaseCharacter.h"
#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BaseCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());

	if (BaseCharacter)
	{
		CustomMovementComponent = BaseCharacter->GetCustomMovementComponent();
	}
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!BaseCharacter || !CustomMovementComponent)
	{
		return ;
	}

	GetGroundSpeed();
	GetAirSpeed();
	GetShouldMove();
	GetIsFalling();
	GetIsClimbing();
	GetClimbVelocity();
}

void UBaseAnimInstance::GetGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(BaseCharacter->GetVelocity());
}

void UBaseAnimInstance::GetAirSpeed()
{
	AirSpeed = BaseCharacter->GetVelocity().Z;
}

void UBaseAnimInstance::GetShouldMove()
{
	bShouldMove = 
		CustomMovementComponent->GetCurrentAcceleration().Size() > 0 && 
		GroundSpeed > 5.f && 
		!bIsFalling;
}

void UBaseAnimInstance::GetIsFalling()
{
	bIsFalling = CustomMovementComponent->IsFalling();
}

void UBaseAnimInstance::GetIsClimbing()
{
	bIsClimbing = CustomMovementComponent->IsClimbing();
}

void UBaseAnimInstance::GetClimbVelocity()
{
	ClimbVelocity = CustomMovementComponent->GetUnrotatedClimbVelocity();
}
