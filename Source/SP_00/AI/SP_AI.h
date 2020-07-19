// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/Damagable.h"
#include "GenericTeamAgentInterface.h"
#include "AIWayPoint.h"
#include "AIHelper.h"
#include "SP_AI.generated.h"

UENUM(BlueprintType)
enum class RobotAI_State : uint8
{
	Patrol_Default, // walk without moving his upperbody
	Patrol_Suspicious, // walk faster and move his upperbody
	Idle_Default, // Dont move at all
	Idle_LookAround, // Move only head
	Idle_Suspicious, // idle and rotating his upperbody
	Attack
};


UCLASS()
class SP_00_API ASP_AI : public ACharacter, public IDamagable, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
private:
	class AAIHelper* Helper = nullptr;

	class ASP_AIController* SP_AIController;

	TArray<FAIWayPointData> patrolsRoutes;

public:
	// Sets default values for this character's properties
	ASP_AI();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class ASP_00Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> areas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName squad;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		int HP = 100;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		float FireCooldown = 1.0f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		float DistanceToStop = 200.0f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		float StopPurchase = 5.0f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		float WarnDelay = 2.0f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		bool usePathOrder = false;

	int waypointIndex = 0;

	/*Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
		class USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		class UAnimMontage* FireAM;

	UFUNCTION(BlueprintCallable)
		void Fire();

	void DeathHandle();

	bool isDead;
	bool fadeOut;
	float timerDeathSequence;
	float timerFadeOutDeathSequence;
	class UMaterialInstanceDynamic* mainMat;


	UPROPERTY(BlueprintReadWrite)
	RobotAI_State currentState;

	UPROPERTY(BlueprintReadWrite)
		bool isSuspicious = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
		class AAIHelper* GetHelper();

	UFUNCTION(BlueprintNativeEvent)
		void GetPerceptionLockRot(FVector &OutLocation, FRotator &OutRotation) const;
		void GetPerceptionLockRot_Implementation(FVector &OutLocation, FRotator &OutRotation) const;

	UPROPERTY(EditDefaultsOnly, Category = Death)
		class UAnimationAsset* deathAnim;

	UPROPERTY(EditAnyWhere, Category = Death)
		float deathSequenceTimer = 2.0f;

	UPROPERTY(BlueprintReadWrite)
		float aimAngle = 0.f;

	UPROPERTY(EditAnyWhere, Category = Death)
		float deathSequenceFadeOut = 1.0f;

	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return (uint8)GenericTeamIdCustom::AI_ID; }

	virtual void TakeDamage(int DamageValue) override;

	virtual void GetActorEyesViewPoint(FVector &Location, FRotator &Rotation) const override;

	UPROPERTY(EditAnywhere)
		class USpotLightComponent* spotLightComponent;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		FLinearColor colorStateDefault;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		float colorIntensityStateDefault;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		float colorLightIntensityStateDefault;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		FLinearColor colorStateSuspicious;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		float colorIntensityStateSuspicious;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		float colorLightIntensityStateSuspicious;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		FLinearColor colorStateAttack;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		float colorIntensityStateAttack;

	UPROPERTY(EditAnyWhere, Category = GameplayFeedBack)
		float colorLightIntensityStateAttack;

	UFUNCTION(BlueprintCallable)
		void SwitchAIState(RobotAI_State newState);
};