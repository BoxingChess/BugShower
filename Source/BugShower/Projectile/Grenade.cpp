// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AGrenade::AGrenade()
{
	// Grenade specific settings
	InitialSpeed = 1500.0f;
	MaxSpeed = 1500.0f;
	GravityScale = 1.5f;
	Damage = 0.0f; // Grenade doesn't do impact damage, only explosion damage

	// Enable bouncing for grenades
	if (ProjectileMovement)
	{
		ProjectileMovement->bShouldBounce = true;
		ProjectileMovement->Bounciness = 0.3f;
		ProjectileMovement->Friction = 0.5f;
	}

	// Increase collision radius for grenade
	if (CollisionComponent)
	{
		CollisionComponent->InitSphereRadius(10.0f);
	}

	// Make grenade mesh bigger and more visible
	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.4f)); // Bigger (40% size instead of 20%)
	}
}

void AGrenade::BeginPlay()
{
	Super::BeginPlay();

	// Start fuse timer
	GetWorld()->GetTimerManager().SetTimer(
		ExplosionTimerHandle,
		this,
		&AGrenade::Explode,
		FuseTime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("Grenade spawned! Will explode in %.1f seconds"), FuseTime);
}

void AGrenade::Explode()
{
	UE_LOG(LogTemp, Warning, TEXT("BOOM! Grenade exploded at location: %s"), *GetActorLocation().ToString());

	// Set explosion center at ground level (move sphere center up by radius so bottom touches ground)
	FVector ExplosionLocation = GetActorLocation();
	ExplosionLocation.Z += ExplosionRadius * 0.5f; // Move center up so sphere is visible above ground

	// Apply radial damage
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		ExplosionDamage,
		ExplosionLocation,
		ExplosionRadius,
		UDamageType::StaticClass(),
		TArray<AActor*>(), // Ignore actors (empty array means hit all)
		this,
		GetInstigatorController(),
		true // Full damage
	);

	// Draw debug sphere (wireframe) to visualize explosion radius
	DrawDebugSphere(
		GetWorld(),
		ExplosionLocation,
		ExplosionRadius,
		24, // More segments for smoother sphere
		FColor::Orange,
		false,
		3.0f, // Display for 3 seconds
		0,
		3.0f // Thicker lines
	);

	// Just use debug spheres for now - simpler and more reliable
	// Draw a filled-looking sphere with multiple overlapping debug spheres
	for (int i = 0; i < 3; i++)
	{
		DrawDebugSphere(
			GetWorld(),
			ExplosionLocation,
			ExplosionRadius - (i * 50.0f), // Nested spheres
			32,
			FColor(255, 100, 0, 100), // Orange-red
			false,
			2.5f,
			0,
			5.0f - i
		);
	}

	// TODO: Spawn explosion particle effect here
	// TODO: Play explosion sound here

	// Destroy grenade
	Destroy();
}

void AGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Grenades bounce, they don't explode on impact
	// Just log the bounce and show explosion radius preview at grenade's current location
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		UE_LOG(LogTemp, Log, TEXT("Grenade bounced off: %s at location: %s"),
			*OtherActor->GetName(),
			*GetActorLocation().ToString());

		// Draw explosion radius preview at grenade's current location (lifted up for visibility)
		FVector PreviewLocation = GetActorLocation();
		PreviewLocation.Z += ExplosionRadius * 0.5f; // Lift center up so sphere is visible

		DrawDebugSphere(
			GetWorld(),
			PreviewLocation,
			ExplosionRadius,
			32, // More segments for smoother circle
			FColor::Yellow,
			false,
			2.0f, // Display for 2 seconds (longer)
			0,
			4.0f // Thicker lines
		);
	}
}
