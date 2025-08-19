#include "PlayerCharacter.h"
#include "SliceOfLife/Components/PlayerMovementComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SliceOfLife/Animation/SliceOfLifeAnimInstance.h"
#include "SliceOfLife/Weapons/WeaponBase.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

    // Use the Character's movement component as our custom type
    PlayerMovementComponent = Cast<UPlayerMovementComponent>(GetCharacterMovement());
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // Capsule to roughly match 1m tall Brocky (Kirby-like) — sphere visual of 50cm radius
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->InitCapsuleSize(50.0f, 50.0f); // radius, half-height (units in cm)
        
        // Configure collision rules for the player capsule
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Capsule->SetCollisionObjectType(ECC_Pawn);
        Capsule->SetGenerateOverlapEvents(true);
        Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
        Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);    // stand on floors
        Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // overlap enemies & item drops
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);          // block other pawns
    }

    // Setup camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 3.0f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

	// Setup follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Initialize state variables
	bIsMoving = false;
	bIsRunning = false;
	bIsAttacking = false;
	bIsCharging = false;
	bIsInHitstun = false;
	CurrentHealthPercent = 1.0f;

    PreviousAttackState = EAttackState::Idle;
    bHasHitThisSwing = false;

    // Default to 2.5D constraint at construction time
    ApplyPlaneConstraintSettings();

    // We will rotate manually for 2.5D facing
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = false;
    }
    bUseControllerRotationYaw = false;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind to component events
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &APlayerCharacter::OnHealthChanged);
		HealthComponent->OnHitstunChanged.AddDynamic(this, &APlayerCharacter::OnHitstunChanged);
	}

    // Try to add mapping context at BeginPlay (controller may not be valid yet)
    AddDefaultMappingContext();

    // Apply designer movement tuning at runtime to CharacterMovementComponent
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = DesignerMaxWalkSpeed;
        Move->MaxWalkSpeedCrouched = DesignerMaxWalkSpeedCrouched;
        Move->JumpZVelocity = DesignerJumpZVelocity;
        Move->AirControl = DesignerAirControl;
        Move->GravityScale = DesignerGravityScale;
        // Re-apply plane constraint in case designers changed the toggle pre-Play
        ApplyPlaneConstraintSettings();
        Move->bOrientRotationToMovement = false;
    }

    // Normalize actor yaw so PlayerStart rotation doesn't affect side-scroller forward
    SetActorRotation(FRotator(0.f, 0.f, 0.f));

    // Spawn and attach default weapon (Skewer)
    if (SkewerClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        CurrentWeapon = GetWorld()->SpawnActor<AWeaponBase>(SkewerClass, GetActorTransform(), SpawnParams);
        if (CurrentWeapon && GetMesh())
        {
            static const FName WeaponSocketName(TEXT("SkewerSocket"));
            CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
        }
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sync state from components for UI/logic
    UpdateCombatState();

    // Damage handling is now via UE damage system and animation notifies.

    // Global hurt box debug draw
    static const auto CVarShowHitboxes = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowHitboxes"));
    const bool bShow = CVarShowHitboxes ? (CVarShowHitboxes->GetInt() != 0) : false;
    if (bShow)
    {
        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            const FVector Location = Capsule->GetComponentLocation();
            const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
            const float Radius = Capsule->GetScaledCapsuleRadius();
            DrawDebugCapsule(GetWorld(), Location, HalfHeight, Radius, FQuat::Identity, FColor::Blue, false, 0.f, 0, 1.5f);
        }
    }
    // Only auto-flip by velocity when NOT attacking (prevents montage jank)
    if (!bIsAttacking)
    {
        const FVector Vel = GetVelocity();
        if (FMath::Abs(Vel.X) > 5.f)
        {
            SetFacing(Vel.X > 0.f);
        }
    }
}

void APlayerCharacter::SetFacing(bool bRight)
{
    if (bFacingRight == bRight)
    {
        return;
    }
    bFacingRight = bRight;
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        constexpr float BaseYawOffset = 90.f; // Mesh's rest orientation offset
        // Invert the mapping so visuals match movement (A=left, D=right)
        const float VisualYaw = BaseYawOffset + (bFacingRight ? 180.f : 0.f);
        MeshComp->SetRelativeRotation(FRotator(0.f, VisualYaw, 0.f));

        if (USliceOfLifeAnimInstance* Anim = Cast<USliceOfLifeAnimInstance>(MeshComp->GetAnimInstance()))
        {
            Anim->SetFacingRight(bFacingRight);
        }
    }
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnMove);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnMove);

        // Jump
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnJump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnJumpReleased);

		// Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnDash);

		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnCrouchReleased);

		// Combat
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnLightAttack);
		EnhancedInputComponent->BindAction(TiltAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnTiltAttack);
		EnhancedInputComponent->BindAction(AerialAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnAerialAttack);
		EnhancedInputComponent->BindAction(SmashAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OnSmashAttackStart);
		EnhancedInputComponent->BindAction(SmashAttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::OnSmashAttackRelease);
	}
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    AddDefaultMappingContext();
}

void APlayerCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    AddDefaultMappingContext();
}

void APlayerCharacter::AddDefaultMappingContext()
{
    if (!DefaultMappingContext)
    {
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
            {
                // Remove first to avoid duplicates, then add
                Subsystem->ClearAllMappings();
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void APlayerCharacter::ApplyPlaneConstraintSettings()
{
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bConstrainToPlane = bConstrainToYPlane;
        Move->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
        Move->SetPlaneConstraintOrigin(FVector::ZeroVector);
        Move->SetPlaneConstraintEnabled(bConstrainToYPlane);
    }
}

void APlayerCharacter::OnMove(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    // Side-scroller: drive world-X; ignore world-Y (plane constrained); optional: Y maps to vertical (Z)
    if (!MovementVector.IsNearlyZero() && !bIsInputLocked)
    {
        AddMovementInput(FVector(1.f, 0.f, 0.f), MovementVector.X);
        // If vertical input needed: AddMovementInput(FVector(0.f, 0.f, 1.f), MovementVector.Y);
    }
    bIsMoving = !MovementVector.IsNearlyZero();
    bIsRunning = FMath::Abs(MovementVector.X) > 0.8f || FMath::Abs(MovementVector.Y) > 0.8f;

    // Facing snap based on horizontal input
    if (FMath::Abs(MovementVector.X) > 0.1f)
    {
        SetFacing(MovementVector.X > 0.f);
    }
}

void APlayerCharacter::OnJump(const FInputActionValue& Value)
{
    if (!bIsInHitstun)
    {
        Jump();
    }
}

void APlayerCharacter::OnJumpReleased(const FInputActionValue& Value)
{
    StopJumping();
}

void APlayerCharacter::OnDash(const FInputActionValue& Value)
{
    if (PlayerMovementComponent && !bIsInHitstun)
    {
        PlayerMovementComponent->DashPressed();
    }
}

void APlayerCharacter::OnCrouch(const FInputActionValue& Value)
{
    if (!bIsInHitstun)
    {
        Crouch();
    }
}

void APlayerCharacter::OnCrouchReleased(const FInputActionValue& Value)
{
    UnCrouch();
}

void APlayerCharacter::OnLightAttack(const FInputActionValue& Value)
{
	if (CombatComponent && !bIsInHitstun)
	{
		CombatComponent->LightAttack();
		UpdateCombatState();
	}
}

void APlayerCharacter::OnTiltAttack(const FInputActionValue& Value)
{
	if (CombatComponent && !bIsInHitstun)
	{
		// Get movement input for tilt direction
		FVector2D TiltDirection = FVector2D::ZeroVector;
		if (PlayerMovementComponent)
		{
			// You could get the current movement input here if needed
			TiltDirection = FVector2D(1.0, 0.0); // Default forward tilt
		}
		
		CombatComponent->TiltAttack(TiltDirection);
		UpdateCombatState();
	}
}

void APlayerCharacter::OnAerialAttack(const FInputActionValue& Value)
{
	if (CombatComponent && !bIsInHitstun)
	{
		CombatComponent->AerialAttack();
		UpdateCombatState();
	}
}

void APlayerCharacter::OnSmashAttackStart(const FInputActionValue& Value)
{
	if (CombatComponent && !bIsInHitstun)
	{
		CombatComponent->SmashAttackStart();
		UpdateCombatState();
	}
}

void APlayerCharacter::OnSmashAttackRelease(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->SmashAttackRelease();
		UpdateCombatState();
	}
}

void APlayerCharacter::OnHealthChanged(float NewHealth)
{
	if (HealthComponent)
	{
		CurrentHealthPercent = HealthComponent->GetHealthPercent();
	}
}

void APlayerCharacter::OnHitstunChanged(bool bInHitstun)
{
	bIsInHitstun = bInHitstun;
}

void APlayerCharacter::UpdateCombatState()
{
	if (CombatComponent)
	{
		bIsAttacking = CombatComponent->IsAttacking();
		bIsCharging = CombatComponent->IsCharging();
	}
}

float APlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (HealthComponent && Actual > 0.f)
    {
        // For generic damage, use forward vector knockback as placeholder
        const FVector KnockDir = GetActorForwardVector();
        const float KnockForce = 600.f;
        HealthComponent->TakeDamage(Actual, KnockDir, KnockForce, DamageCauser);
    }
    return Actual;
}

void APlayerCharacter::AddItemToInventory(ECategory Category)
{
    switch (Category)
    {
        case ECategory::Limb:
            ++NumLimbs;
            UE_LOG(LogTemp, Log, TEXT("Added Limb to inventory. Total: %d"), NumLimbs);
            break;
        case ECategory::Appendage:
            ++NumAppendages;
            UE_LOG(LogTemp, Log, TEXT("Added Appendage to inventory. Total: %d"), NumAppendages);
            break;
        case ECategory::Organ:
            ++NumOrgans;
            UE_LOG(LogTemp, Log, TEXT("Added Organ to inventory. Total: %d"), NumOrgans);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Unknown category: %d"), (int32)Category);
            break;
    }
}

void APlayerCharacter::ResetInventory()
{
    NumLimbs = 0;
    NumAppendages = 0;
    NumOrgans = 0;
    UE_LOG(LogTemp, Log, TEXT("Inventory reset to zero"));
}

void APlayerCharacter::DisplayInventoryStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== INVENTORY STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Limbs: %d"), NumLimbs);
    UE_LOG(LogTemp, Log, TEXT("Appendages: %d"), NumAppendages);
    UE_LOG(LogTemp, Log, TEXT("Organs: %d"), NumOrgans);
    UE_LOG(LogTemp, Log, TEXT("Total: %d"), GetTotalInventoryCount());
    UE_LOG(LogTemp, Log, TEXT("====================="));
}

FString APlayerCharacter::GetInventoryDisplayString() const
{
    return FString::Printf(TEXT("Limbs: %d | Appendages: %d | Organs: %d | Total: %d"), 
                           NumLimbs, NumAppendages, NumOrgans, GetTotalInventoryCount());
}

bool APlayerCharacter::HasEnoughItems(int32 RequiredLimbs, int32 RequiredAppendages, int32 RequiredOrgans) const
{
    return (NumLimbs >= RequiredLimbs) && 
           (NumAppendages >= RequiredAppendages) && 
           (NumOrgans >= RequiredOrgans);
}

bool APlayerCharacter::ConsumeItems(int32 LimbsToConsume, int32 AppendagesToConsume, int32 OrgansToConsume)
{
    // Check if we have enough items
    if (!HasEnoughItems(LimbsToConsume, AppendagesToConsume, OrgansToConsume))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough items to consume. Required: L:%d A:%d O:%d, Have: L:%d A:%d O:%d"), 
               LimbsToConsume, AppendagesToConsume, OrgansToConsume, NumLimbs, NumAppendages, NumOrgans);
        return false;
    }

    // Consume the items
    NumLimbs -= LimbsToConsume;
    NumAppendages -= AppendagesToConsume;
    NumOrgans -= OrgansToConsume;

    UE_LOG(LogTemp, Log, TEXT("Consumed items. Remaining: L:%d A:%d O:%d"), NumLimbs, NumAppendages, NumOrgans);
    return true;
}
