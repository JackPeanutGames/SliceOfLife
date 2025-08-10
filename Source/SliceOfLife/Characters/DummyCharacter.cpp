#include "DummyCharacter.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"

ADummyCharacter::ADummyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create and setup components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	DamageDisplayWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageDisplayWidget"));
    PlaceholderBuddy = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBuddy"));

    // Setup damage display widget
	DamageDisplayWidget->SetupAttachment(RootComponent);
	DamageDisplayWidget->SetRelativeLocation(FVector(0.0f, 0.0f, DamageDisplayHeight));
	DamageDisplayWidget->SetWidgetSpace(EWidgetSpace::World);
	DamageDisplayWidget->SetDrawSize(FVector2D(200.0f, 100.0f));

    // Setup placeholder "Buddy" (sphere you can knock around)
    PlaceholderBuddy->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereFinder.Succeeded())
    {
        PlaceholderBuddy->SetStaticMesh(SphereFinder.Object);
        // 2m tall sphere: diameter 200cm, so scale 2.0 on default 100cm diameter sphere
        PlaceholderBuddy->SetWorldScale3D(FVector(2.0f));
        PlaceholderBuddy->SetRelativeLocation(FVector(0, 0, 100.0f));
        PlaceholderBuddy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PlaceholderBuddy->SetGenerateOverlapEvents(false);
    }

    // Match capsule to 2m height
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->InitCapsuleSize(50.0f, 100.0f);
    }
}

void ADummyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind to health component events
	if (HealthComponent)
	{
		HealthComponent->OnDamageReceived.AddDynamic(this, &ADummyCharacter::OnDamageReceived);
		HealthComponent->OnHealthChanged.AddDynamic(this, &ADummyCharacter::OnHealthChanged);
		HealthComponent->OnHitstunChanged.AddDynamic(this, &ADummyCharacter::OnHitstunChanged);

		// Set dummy-specific health settings
		HealthComponent->bInvulnerable = bInvulnerable;
		HealthComponent->bCanBeKnockedOut = bCanBeKnockedOut;
	}

	// Update initial display
	UpdateDamageDisplay();
}

void ADummyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update damage display position to follow the dummy
	if (DamageDisplayWidget && bShowDamageDisplay)
	{
		FVector WidgetLocation = GetActorLocation();
		WidgetLocation.Z += DamageDisplayHeight;
		DamageDisplayWidget->SetWorldLocation(WidgetLocation);
	}
}

void ADummyCharacter::UpdateDamageDisplay()
{
	if (DamageDisplayWidget && HealthComponent)
	{
		// Show/hide widget based on settings
		DamageDisplayWidget->SetVisibility(bShowDamageDisplay);

		// Update widget content if needed
		// This would typically be done through a UMG widget blueprint
		// For now, we'll just ensure the widget is visible
	}
}

void ADummyCharacter::LogHit(float Damage, FVector KnockbackDirection, float KnockbackForce)
{
	if (bLogAllHits)
	{
		UE_LOG(LogTemp, Log, TEXT("DUMMY HIT: %s took %f damage with knockback %s (force: %f)"), 
			*GetName(), Damage, *KnockbackDirection.ToString(), KnockbackForce);
	}
}

void ADummyCharacter::ResetDummy()
{
	if (HealthComponent)
	{
		HealthComponent->ResetHealth();
		UpdateDamageDisplay();
		
		UE_LOG(LogTemp, Log, TEXT("Dummy %s reset"), *GetName());
	}
}

void ADummyCharacter::SetInvulnerable(bool bNewInvulnerable)
{
	bInvulnerable = bNewInvulnerable;
	if (HealthComponent)
	{
		HealthComponent->bInvulnerable = bNewInvulnerable;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Dummy %s invulnerability set to: %s"), *GetName(), bNewInvulnerable ? TEXT("True") : TEXT("False"));
}

void ADummyCharacter::ToggleDamageDisplay()
{
	bShowDamageDisplay = !bShowDamageDisplay;
	UpdateDamageDisplay();
	
	UE_LOG(LogTemp, Log, TEXT("Dummy %s damage display toggled to: %s"), *GetName(), bShowDamageDisplay ? TEXT("True") : TEXT("False"));
}

void ADummyCharacter::OnDamageReceived(float Damage, FVector KnockbackDirection, float KnockbackForce)
{
	LogHit(Damage, KnockbackDirection, KnockbackForce);
	UpdateDamageDisplay();
}

void ADummyCharacter::OnHealthChanged(float NewHealth)
{
	UpdateDamageDisplay();
	
	UE_LOG(LogTemp, Log, TEXT("Dummy %s health changed to: %f"), *GetName(), NewHealth);
}

void ADummyCharacter::OnHitstunChanged(bool bInHitstun)
{
	UE_LOG(LogTemp, Log, TEXT("Dummy %s hitstun changed to: %s"), *GetName(), bInHitstun ? TEXT("True") : TEXT("False"));
}
