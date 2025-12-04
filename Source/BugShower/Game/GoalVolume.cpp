// Fill out your copyright notice in the Description page of Project Settings.

#include "GoalVolume.h"
#include "BSGameModeBase.h"
#include "BSGameStateBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AGoalVolume::AGoalVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create trigger box component
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create optional goal mesh for visualization
	GoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMesh"));
	GoalMesh->SetupAttachment(RootComponent);
	GoalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Default settings
	GoalCondition = EGoalCondition::AnyPlayer;
	VictoryDelaySeconds = 2.0f;
	bEnableDebugLog = false;
	bVictoryTriggered = false;
}

void AGoalVolume::BeginPlay()
{
	Super::BeginPlay();

	// Only bind overlap events on server
	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGoalVolume::OnTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AGoalVolume::OnTriggerEndOverlap);

		if (bEnableDebugLog)
		{
			UE_LOG(LogTemp, Log, TEXT("[GoalVolume] Initialized with condition: %s"),
				*UEnum::GetValueAsString(GoalCondition));
		}
	}
}

void AGoalVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Server authority check
	if (!HasAuthority())
		return;

	// Victory already triggered
	if (bVictoryTriggered)
		return;

	// Check if overlapping actor is a player character
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
		return;

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (!PlayerController)
		return;

	// Add player to the set
	PlayersInGoal.Add(PlayerController);

	if (bEnableDebugLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[GoalVolume] Player %s entered goal area (%d/%d alive)"),
			*PlayerController->GetName(), PlayersInGoal.Num(), GetAlivePlayerCount());
	}

	// Check if victory conditions are met
	if (CheckVictoryCondition())
	{
		TriggerVictory();
	}
}

void AGoalVolume::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Server authority check
	if (!HasAuthority())
		return;

	// Victory already triggered - don't remove players
	if (bVictoryTriggered)
		return;

	// Check if overlapping actor is a player character
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
		return;

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (!PlayerController)
		return;

	// Remove player from the set
	PlayersInGoal.Remove(PlayerController);

	if (bEnableDebugLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[GoalVolume] Player %s left goal area (%d/%d alive)"),
			*PlayerController->GetName(), PlayersInGoal.Num(), GetAlivePlayerCount());
	}
}

bool AGoalVolume::CheckVictoryCondition()
{
	const int32 AlivePlayers = GetAlivePlayerCount();
	const int32 PlayersInGoalCount = PlayersInGoal.Num();

	// No alive players in game
	if (AlivePlayers == 0)
		return false;

	switch (GoalCondition)
	{
	case EGoalCondition::AnyPlayer:
		// At least one alive player in goal
		return PlayersInGoalCount > 0;

	case EGoalCondition::AllPlayers:
		// All alive players must be in goal
		return PlayersInGoalCount >= AlivePlayers;

	case EGoalCondition::MajorityPlayers:
		// More than half of alive players in goal
		return PlayersInGoalCount > (AlivePlayers / 2);

	default:
		return false;
	}
}

void AGoalVolume::TriggerVictory()
{
	if (bVictoryTriggered)
		return;

	bVictoryTriggered = true;

	if (bEnableDebugLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[GoalVolume] Victory triggered! Condition: %s"),
			*UEnum::GetValueAsString(GoalCondition));
	}

	// Play victory effects
	PlayVictoryEffects();

	// Call Blueprint event
	OnGoalReached();

	// Execute victory after delay
	if (VictoryDelaySeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(VictoryTimerHandle, this, &AGoalVolume::ExecuteVictory,
			VictoryDelaySeconds, false);
	}
	else
	{
		ExecuteVictory();
	}
}

void AGoalVolume::ExecuteVictory()
{
	// Get game mode and trigger victory
	ABSGameModeBase* GameMode = Cast<ABSGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		if (bEnableDebugLog)
		{
			UE_LOG(LogTemp, Log, TEXT("[GoalVolume] Calling GameMode->EndGame(true)"));
		}

		GameMode->EndGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GoalVolume] Failed to get ABSGameModeBase!"));
	}
}

void AGoalVolume::PlayVictoryEffects()
{
	// Play particle effect
	if (VictoryParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VictoryParticle, GetActorLocation());
	}

	// Play sound effect
	if (VictorySound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), VictorySound, GetActorLocation());
	}
}

int32 AGoalVolume::GetAlivePlayerCount() const
{
	const UWorld* World = GetWorld();
	if (!World)
		return 0;

	const ABSGameStateBase* BSGameState = World->GetGameState<ABSGameStateBase>();
	if (!BSGameState)
		return 0;

	return BSGameState->AlivePlayerCount;
}
