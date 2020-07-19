// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SP_00Character.h"

#include "Projectiles/SP_00Projectile.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Containers/EnumAsByte.h"

#include "DrawDebugHelpers.h"

#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"

#include "Components/WidgetComponent.h"

#include <algorithm>

#include <string>

#include "GameFramework/CharacterMovementComponent.h"

#include "GenericTeamAgentInterface.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

#include "Components/PostProcessComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/BlendableInterface.h"
#include "AIController.h"
#include "Weapon/Weapon.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "AI/AIHelper.h" 

#include "Perception/AIPerceptionStimuliSourceComponent.h"

#include "Animation/AnimInstance.h"

#include "Components/PostProcessComponent.h"

#include "Components/BoxComponent.h"

#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

ASP_00Character::ASP_00Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(30.0f, 90.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	//Mesh1P->SetOnlyOwnerSee(true);
	GetMesh()->SetupAttachment(RootComponent);
	GetMesh()->bCastDynamicShadow = false;
	GetMesh()->CastShadow = false;

	weaponPlayer = CreateDefaultSubobject<UWeapon>(TEXT("PlayerWeapon"));

	weaponPlayer->SetupAttachment(RootComponent);

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(weaponPlayer);

	weaponWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WeaponWidget"));
	weaponWidget->SetupAttachment(weaponPlayer);

	weaponPlayer->MuzzleLocation = MuzzleLocation;
	weaponPlayer->weaponWidget = weaponWidget;

	HP = 100;

	stimuliPerception = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSourcePerception"));

	TeammID = FGenericTeamId((uint8)GenericTeamIdCustom::playerID);

	postProcessVolumeBound = CreateDefaultSubobject<UBoxComponent>(TEXT("PostProcessVolumeBound"));

	postProcessVolumeBound->SetupAttachment(FirstPersonCameraComponent);

	visionAlterator = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessVolumeVision"));
	
	visionAlterator->SetupAttachment(postProcessVolumeBound);
}

void ASP_00Character::callbackHasEnter(bool _b)
{
	if (_b)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "entre");
		OnEnterDelegate.Broadcast();
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "sort");
		OnExitDelegate.Broadcast();
	}
}

void ASP_00Character::TakeDamage(int DamageValue)
{
	HP -= DamageValue;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::FromInt(HP));

	if (HP <= 0)
	{
		HP = 0;
		DeathHandle();
	}
}

void ASP_00Character::PlayerInputSetter(bool setON)
{
	bUseControllerRotationPitch = bUseControllerRotationYaw = setON;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (setON)
	{
		if (PlayerController != NULL)
		{
			this->EnableInput(PlayerController);
		}
	}
	else
	{
		if (PlayerController != NULL)
		{
			this->DisableInput(PlayerController);
		}
	}
}

void ASP_00Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	weaponPlayer->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("R_wristEnd_JSocket"));

	weaponPlayer->Characterref = this;

	characterMovement = GetCharacterMovement();

	InitPostProcess();

	//Init Crouch parameters

	CapsuleHeightStandingUp = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	CapsuleHeightCrouched = CapsuleHeightStandingUp * 0.10f;

	crouchTimer = 0.0f;
	isCrouching = false;
	IsPassiveCheckCrounching = false;

	queryArrayCrouch.Add(TEnumAsByte<EObjectTypeQuery>(EObjectTypeQuery::ObjectTypeQuery1));
	queryArrayCrouch.Add(TEnumAsByte<EObjectTypeQuery>(EObjectTypeQuery::ObjectTypeQuery2));

	playerIsAiming = false;
	hasWeapon = true;


	robotHelperSpeakerRef = GetWorld()->SpawnActor<ARobotSpeaker>(RobotHelperSpeaker.Get(), GetActorTransform());

	postProcessVolumeBound->SetVisibility(false);

	visionAlterator->bEnabled = false;

}

void ASP_00Character::Tick(float deltatime)
{
	Super::Tick(deltatime);

	if (isCrouching)
	{
		CrouchImpl(deltatime);
	}

	if (isManuallyMoving)
	{
		if (isManualStep)
		{
			timerStepsManual += deltatime;
			if (timerStepsManual >= stepsDelay)
			{
				timerStepsManual -= stepsDelay;

				OnNewStep.Broadcast();
			}
		}
	}

}

void ASP_00Character::InitPostProcess()
{
	if (FirstPersonCameraComponent->PostProcessSettings.WeightedBlendables.Array.Num() > 0)
	{

		currentVisionMode = VisionMode::standardVision;

		if (FirstPersonCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Object != NULL)
		{
			UMaterialInterface *visionModeMatInst = Cast<UMaterialInterface>(FirstPersonCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Object);

			FirstPersonCameraComponent->RemoveBlendable(visionModeMatInst);
			visionModeMaterial1 = UMaterialInstanceDynamic::Create(visionModeMatInst, NULL);

			visionModeMaterial1->CopyScalarAndVectorParameters(*visionModeMatInst, ERHIFeatureLevel::ES2);

			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial1, 0.0f);
		}


		if (FirstPersonCameraComponent->PostProcessSettings.WeightedBlendables.Array[1].Object != NULL)
		{
			UMaterialInterface *visionModeMatInst = Cast<UMaterialInterface>(FirstPersonCameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Object);

			FirstPersonCameraComponent->RemoveBlendable(visionModeMatInst);
			visionModeMaterial2 = UMaterialInstanceDynamic::Create(visionModeMatInst, NULL);

			visionModeMaterial2->CopyScalarAndVectorParameters(*visionModeMatInst, ERHIFeatureLevel::ES2);

			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial2, 0.0f);
		}

	}
	else
	{
		visionModeMaterial1 = NULL;
		UE_LOG(LogFPChar, Warning, TEXT("Post Process material hasn't been set no vision mode therefore"));
	}
}

void ASP_00Character::ChangeAim()
{
	playerIsAiming = !playerIsAiming;
}

void ASP_00Character::OnVisionModeChanged(FKey _keyPressed)
{
	VisionMode modeToChange;

	if(_keyPressed == EKeys::Ampersand)
	{
		modeToChange = VisionMode::standardVision;
	}
	else if (_keyPressed == EKeys::E_AccentAigu)
	{
		modeToChange = VisionMode::XRayVision;
	}
	else if (_keyPressed == EKeys::Quote)
	{
		modeToChange = VisionMode::goalHighlightVision;
	}

	if (currentVisionMode == modeToChange)
	{
		return;
	}
	else
	{
		currentVisionMode = modeToChange;
	}

	if (visionModeMaterial1 != NULL)
	{
		if (currentVisionMode == VisionMode::standardVision)
		{
			modeToChange = VisionMode::standardVision;
			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial1, 0.0f);
			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial2, 0.0f);
		}
		else if (currentVisionMode == VisionMode::XRayVision)
		{
			modeToChange = VisionMode::XRayVision;
			visionModeMaterial1->SetScalarParameterValue(TEXT("InitTime"), GetWorld()->GetTimeSeconds());
			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial1, 1.0f);
			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial2, 0.0f);
		}
		else if (currentVisionMode == VisionMode::goalHighlightVision)
		{
			modeToChange = VisionMode::goalHighlightVision;
			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial1, 0.0f);
			FirstPersonCameraComponent->AddOrUpdateBlendable(visionModeMaterial2, 1.0f);
		}
	}
	else
	{
		UE_LOG(LogFPChar, Warning, TEXT("Post Process material hasn't been set, no vision mode therefore"));
	}
}

void ASP_00Character::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
}

void ASP_00Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	/*PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);*/

	//Bind crouch

	// made by a co-worker but this is shitty !

	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASP_00Character::CrouchCustom);
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASP_00Character::UnCrouchCustom);
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASP_00Character::UnCrouchCustom);

	//Bind Reload
	//PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASP_00Character::Reload);


	//Bind running
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASP_00Character::OnStartRunning);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASP_00Character::OnEndRunning);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, weaponPlayer, &UWeapon::OnFire);


	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &ASP_00Character::OnUse);

	//Bind Aiming

	// made by a co-worker but it was bugged !

	//PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASP_00Character::ChangeAim);
	//PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASP_00Character::ChangeAim);

	//Bind Change vision event
	PlayerInputComponent->BindAction("ChangeVisionMode", IE_Pressed, this, &ASP_00Character::OnVisionModeChanged);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ASP_00Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASP_00Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASP_00Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASP_00Character::LookUpAtRate);
}

void ASP_00Character::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASP_00Character::OnStartRunning()
{
	if (characterMovement != NULL)
	{
		characterMovement->MaxWalkSpeed += SpeedRunVelocity;
	}
}

void ASP_00Character::OnEndRunning()
{
	if (characterMovement != NULL)
	{
		characterMovement->MaxWalkSpeed -= SpeedRunVelocity;
	}
}

void ASP_00Character::OnUse()
{
	OnUseDelegate.Broadcast();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != NULL && UseWithWeapon != NULL)
	{
		AnimInstance->Montage_Play(UseWithWeapon);
	}

	if (OnUseDelegate.IsBound() == true)
	{
		if (HitPunchSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitPunchSound, GetActorLocation());
		}
	}
	else if(AirPunchSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AirPunchSound, GetActorLocation());
	}

}

void ASP_00Character::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ASP_00Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASP_00Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASP_00Character::CrouchCustom()
{
	if (Controller != NULL)
	{
		isCrouching = true;
		ratio = 1;

		if (IsPassiveCheckCrounching)
		{
			IsPassiveCheckCrounching = false;
			GetWorldTimerManager().ClearTimer(crounchCheckHandle);
		}
	}
}

void ASP_00Character::CrouchImpl(float DeltaTime)
{
	/*const float TargetBEH = isCrouching ? CrouchedEyeHeight : hauteurEye;
	const float TargetCapsuleSize = isCrouching ? characterMovement->CrouchedHalfHeight : hauteurCapsule;*/
	
	crouchTimer += DeltaTime * ratio;

	if (crouchTimer > crouchTime)
	{
		crouchTimer = crouchTime;
		IsPassiveCheckCrounching = true;
		isCrouching = false;
	}
	else if (crouchTimer < 0.0f)
	{
		crouchTimer = 0.0f;
		isCrouching = false;
	}

	GetCapsuleComponent()->SetCapsuleHalfHeight(FMath::Lerp(CapsuleHeightStandingUp, CapsuleHeightCrouched, crouchTimer / crouchTime), true);
	
	
	//BaseEyeHeight = FMath::FInterpTo(BaseEyeHeight, TargetBEH, DeltaTime, 10.0f);
	// Dist and DeltaMovCaps are used for the interpolation value added to RelativeLocation.Z
	//const float Dist = TargetCapsuleSize - GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	//const float DeltaMovCaps = Dist * FMath::Clamp<float>(DeltaTime*10.0f, 0.f, 1.f);
	//GetCapsuleComponent()->SetRelativeLocation(FVector(GetCapsuleComponent()->RelativeLocation.X, GetCapsuleComponent()->RelativeLocation.Y, (GetCapsuleComponent()->RelativeLocation.Z + DeltaMovCaps)), true);
}

void ASP_00Character::UnCrouchCustom()
{
	if (!IsPassiveCheckCrounching)
	{
		return;
	}
	
	if (Controller != NULL)
	{
		TArray<AActor*> foundActor;
		TArray<AActor*> ignorActor;

		if (!UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation() + FVector(0.f,0.f, 90.f), 30.f, queryArrayCrouch, NULL, ignorActor, foundActor))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "touche rien");
			ratio = -2;
			isCrouching = true;
			IsPassiveCheckCrounching = false;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "touche");
			
			GetWorld()->GetTimerManager().SetTimer(crounchCheckHandle, this, &ASP_00Character::UnCrouchCustom, 0.4f);
		}
	}
}

void ASP_00Character::PlayerHasWeapon(bool setHasWeapon)
{
	hasWeapon = setHasWeapon;
}

void ASP_00Character::HideWeapon(bool isHidden)
{
	weaponPlayer->SetVisibility(!isHidden, true);
}

void ASP_00Character::CallbackShotPlayer()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != NULL)
	{
		if (playerIsAiming)
		{
			if (RecoilAnimationAiming != NULL)
			{
				AnimInstance->Montage_Play(RecoilAnimationAiming);
			}
		}
		else
		{
			if (RecoilAnimationNotAiming != NULL)
			{
				AnimInstance->Montage_Play(RecoilAnimationNotAiming);
			}
		}
	}
}

void ASP_00Character::HelperSpeakerAISetSound(const FString & dialogName)
{
	if (robotHelperSpeakerRef != NULL)
	{
		robotHelperSpeakerRef->SetSpeakerSound(dialogName);
	}
}

void ASP_00Character::SetCurrentPlayerVisionQuality(bool isVisionGood)
{
	hasBadVision = !isVisionGood;

	if (hasBadVision)
	{
		postProcessVolumeBound->SetVisibility(true);
		visionAlterator->bEnabled = true;
	}
	else
	{
		postProcessVolumeBound->SetVisibility(false);
		visionAlterator->bEnabled = false;
	}
}

void ASP_00Character::SetManualStep(bool isStepManual, float overrideDelay)
{
	isManualStep = isStepManual;
	timerStepsManual = 0.0f;
	stepsDelay = overrideDelay;
}
