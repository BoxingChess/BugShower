// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoalVolume.generated.h"

/**
 * Goal condition type - determines when victory is triggered
 */
UENUM(BlueprintType)
enum class EGoalCondition : uint8
{
	AnyPlayer		UMETA(DisplayName = "Any Player"),		// Victory when any player reaches goal
	AllPlayers		UMETA(DisplayName = "All Players"),		// Victory when all players reach goal
	MajorityPlayers	UMETA(DisplayName = "Majority Players")	// Victory when majority of players reach goal
};

/**
 * Goal Volume Actor
 * Triggers game victory when player(s) reach the goal area
 * Designed for multiplayer with server authority
 */
UCLASS()
class BUGSHOWER_API AGoalVolume : public AActor
{
	GENERATED_BODY()

public:
	AGoalVolume();

protected:
	virtual void BeginPlay() override;

public:
	// Trigger box component for detecting player overlap
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goal")
	TObjectPtr<class UBoxComponent> TriggerBox;

	// Visual mesh component (optional, for editor visualization)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goal")
	TObjectPtr<class UStaticMeshComponent> GoalMesh;

	// Particle effect to play when goal is reached (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal|Effects")
	TObjectPtr<class UParticleSystem> VictoryParticle;

	// Sound to play when goal is reached (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal|Effects")
	TObjectPtr<class USoundBase> VictorySound;

	// Goal condition - determines when victory is triggered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal|Settings")
	EGoalCondition GoalCondition;

	// Delay before transitioning to lobby after victory (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal|Settings", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float VictoryDelaySeconds;

	// Enable debug logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goal|Debug")
	bool bEnableDebugLog;

protected:
	// Players who have reached the goal
	UPROPERTY()
	TSet<TObjectPtr<AController>> PlayersInGoal;

	// Has victory been triggered?
	bool bVictoryTriggered;

	// Handle for victory delay timer
	FTimerHandle VictoryTimerHandle;

protected:
	// Called when an actor enters the trigger volume
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when an actor leaves the trigger volume
	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Check if victory conditions are met
	bool CheckVictoryCondition();

	// Trigger game victory
	void TriggerVictory();

	// Execute victory after delay
	void ExecuteVictory();

	// Play victory effects (particles, sound)
	void PlayVictoryEffects();

	// Get number of alive players in the game
	int32 GetAlivePlayerCount() const;

	// Blueprint event called when goal is reached
	UFUNCTION(BlueprintImplementableEvent, Category = "Goal")
	void OnGoalReached();
};
