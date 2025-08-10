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
    }

    // Placeholder body mesh
    PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBody"));
    PlaceholderBody->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereFinder.Succeeded())
    {
        PlaceholderBody->SetStaticMesh(SphereFinder.Object);
        // Scale so diameter is 100cm (1m). Engine sphere default radius is 50cm with scale 1.0, so 1.0 is already 1m diameter.
        PlaceholderBody->SetWorldScale3D(FVector(1.0f));
        PlaceholderBody->SetRelativeLocation(FVector(0.0, 0.0, 50.0f)); // Sit on ground
        PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PlaceholderBody->SetGenerateOverlapEvents(false);
    }

	// Setup camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 3.0f;

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
    }
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Sync state from components for UI/logic
    UpdateCombatState();

    // Damage handling is now via UE damage system and animation notifies.
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

void APlayerCharacter::OnMove(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    // Use built-in Character movement input
    if (!MovementVector.IsNearlyZero())
    {
        AddMovementInput(GetActorForwardVector(), MovementVector.X);
        AddMovementInput(GetActorRightVector(), MovementVector.Y);
    }
    bIsMoving = !MovementVector.IsNearlyZero();
    bIsRunning = FMath::Abs(MovementVector.X) > 0.8f || FMath::Abs(MovementVector.Y) > 0.8f;
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
