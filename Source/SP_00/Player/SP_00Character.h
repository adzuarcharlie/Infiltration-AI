// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once
#include <vector>
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "Components/TextBlock.h"
#include "Interfaces/Damagable.h"
#include "Utilities/RobotSpeaker.h"
#include "SP_00Character.generated.h"

class UInputComponent;
class UMaterialInstanceDynamic;
class UAIPerceptionStimuliSourceComponent;
class UWidgetComponent;
class UweaponWidget;
class UWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnManualStep);


UCLASS(config=Game)
class ASP_00Character : public ACharacter, public IDamagable, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = Movement)
	float SpeedRunVelocity = 300.0f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float CrouchVelocity = 100.0f;

	/*/** Pawn mesh: 1st person view (arms; seen only by self) */
	//UPROPERTY(VisibleDefaultsOnly, Category=Mesh)*/
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UCharacterMovementComponent* characterMovement;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UAIPerceptionStimuliSourceComponent *stimuliPerception;

	FGenericTeamId TeammID;

	void OnVisionModeChanged(FKey _keyPressed);

	UMaterialInstanceDynamic* visionModeMaterial1;
	UMaterialInstanceDynamic* visionModeMaterial2;

	UPROPERTY(EditDefaultsOnly)
	int HP;

public:
	ASP_00Character();	

	void callbackHasEnter(bool _b);

	virtual void TakeDamage(int DamageValue) override;

	UFUNCTION(BlueprintImplementableEvent)
	void DeathHandle();

	UPROPERTY(EditAnywhere, Category = Weapon)
	class UWeapon* weaponPlayer;

	FOnEnter OnEnterDelegate;
	FOnExit OnExitDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnManualStep OnNewStep;

	UPROPERTY(BlueprintAssignable)
	FOnUse OnUseDelegate;

	UFUNCTION(BlueprintCallable)
	void PlayerInputSetter(bool setON);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float deltatime) override;

public:

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* MuzzleLocation;

	UPROPERTY(VisibleDefaultsOnly, Category = UI)
	UWidgetComponent* weaponWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = crouch)
	float hauteurEye;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = crouch)
	float CapsuleHeightCrouched;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = crouch)
	bool isCrouching = false;

	float CapsuleHeightStandingUp;
	int ratio;
	bool IsPassiveCheckCrounching;
	FTimerHandle crounchCheckHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = crouch)
	float crouchTime = 0.4f;

	float crouchTimer;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float BaseLookUpRate;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

	enum VisionMode
	{
		standardVision,
		XRayVision,
		goalHighlightVision,
		VisionCount
	};

	FORCEINLINE VisionMode GetCurrentPlayerCurrentVisionMode() { return currentVisionMode; }

protected:

	VisionMode currentVisionMode;
	
	void CrouchCustom();

	void CrouchImpl(float DeltaTime);

	void UnCrouchCustom();

	TArray<TEnumAsByte<EObjectTypeQuery>> queryArrayCrouch;

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	void OnStartRunning();

	void OnEndRunning();

	void OnUse();

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	void InitPostProcess();

	bool hasBadVision;

	bool isManualStep;
	float timerStepsManual;

	void ChangeAim();

	ARobotSpeaker* robotHelperSpeakerRef;

public:
	/** Returns Mesh1P subobject **/
	//FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return TeammID; }

	UPROPERTY(BlueprintReadOnly)
	bool playerIsAiming;

	UPROPERTY(BlueprintReadOnly)
		bool hasWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UAnimMontage* RecoilAnimationNotAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UAnimMontage* RecoilAnimationAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		class UAnimMontage* UseWithWeapon;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		class USoundBase* AirPunchSound;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		class USoundBase* HitPunchSound;

	UFUNCTION(BlueprintCallable)
		void PlayerHasWeapon(bool setHasWeapon);

	UFUNCTION(BlueprintCallable)
		void HideWeapon(bool isHidden);

	void CallbackShotPlayer();

	UPROPERTY(EditAnyWhere)
		TSubclassOf<ARobotSpeaker> RobotHelperSpeaker;

	UFUNCTION(BlueprintCallable)
		void HelperSpeakerAISetSound(const FString& dialogName);

	UFUNCTION(BlueprintCallable)
		void SetCurrentPlayerVisionQuality(bool isVisionGood);

	UFUNCTION(BlueprintCallable)
		void SetManualStep(bool isStepManual, float overrideDelay = 1.1f);

	UPROPERTY(EditDefaultsOnly, Category = CustomPostProcess)
	class UBoxComponent* postProcessVolumeBound;

	UPROPERTY(EditDefaultsOnly, Category = CustomPostProcess)
	class UPostProcessComponent* visionAlterator;

	UPROPERTY(EditAnyWhere, Category = Steps)
		float stepsDelay;


	UPROPERTY(Interp, EditAnywhere, BlueprintReadOnly, meta=(DisplayName="Player Is Manually moving"))
		bool isManuallyMoving;
};
