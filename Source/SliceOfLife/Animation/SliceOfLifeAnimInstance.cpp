#include "SliceOfLife/Animation/SliceOfLifeAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "KismetAnimationLibrary.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "SliceOfLife/Components/PlayerMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

USliceOfLifeAnimInstance::USliceOfLifeAnimInstance()
    : Speed(0.f)
    , bIsInAir(false)
    , bIsCrouching(false)
    , bIsDashing(false)
    , bIsAttacking(false)
    , bIsCharging(false)
    , bIsInHitstun(false)
    , MovementDirection(0.f)
    , bIsDoubleJump(false)
    , bFacingRight(true)
    , CachedPlayerCharacter(nullptr)
    , CachedMovementComponent(nullptr)
{
}

void USliceOfLifeAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* OwnerPawn = TryGetPawnOwner();
    CachedPlayerCharacter = OwnerPawn ? Cast<APlayerCharacter>(OwnerPawn) : nullptr;
    if (CachedPlayerCharacter)
    {
        CachedMovementComponent = Cast<UPlayerMovementComponent>(CachedPlayerCharacter->GetCharacterMovement());
    }
}

void USliceOfLifeAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* OwnerPawn = TryGetPawnOwner();
    if (!OwnerPawn)
    {
        return;
    }

    if (!CachedPlayerCharacter)
    {
        CachedPlayerCharacter = Cast<APlayerCharacter>(OwnerPawn);
    }
    if (CachedPlayerCharacter && !CachedMovementComponent)
    {
        CachedMovementComponent = Cast<UPlayerMovementComponent>(CachedPlayerCharacter->GetCharacterMovement());
    }

    const FVector Velocity = OwnerPawn->GetVelocity();
    Speed = Velocity.Size2D();

    // Falling/jump state
    if (UCharacterMovementComponent* Move = CachedPlayerCharacter ? CachedPlayerCharacter->GetCharacterMovement() : nullptr)
    {
        bIsInAir = Move->IsFalling();
    }
    else
    {
        bIsInAir = false;
    }

    // Crouch state
    bIsCrouching = CachedPlayerCharacter ? CachedPlayerCharacter->bIsCrouched : false;

    // Dash state (from custom movement component if present)
    bIsDashing = CachedMovementComponent ? CachedMovementComponent->IsDashing() : false;

    // Combat states from owning character
    if (CachedPlayerCharacter)
    {
        bIsAttacking = CachedPlayerCharacter->IsAttacking();
        bIsCharging = CachedPlayerCharacter->IsCharging();
        bIsInHitstun = CachedPlayerCharacter->IsInHitstun();
    }
    else
    {
        bIsAttacking = false;
        bIsCharging = false;
        bIsInHitstun = false;
    }

    // Optional direction for blend spaces (use KismetAnimationLibrary per UE5 API)
    MovementDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerPawn->GetActorRotation());

    // Facing right based on velocity X sign (fallback). Designers can override in BP if mesh flip logic differs.
    bFacingRight = Velocity.X >= 0.f;

    // Optional double-jump flag: leave as-is (to be set by character on second jump) unless you want to derive from movement component state
}

void USliceOfLifeAnimInstance::SetFacingRight(bool bRight)
{
    bFacingRight = bRight;
}


