// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_AI.h"

#include "SP_AIController.h"
#include "Projectiles/SP_00Projectile.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimationAsset.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SpotLightComponent.h"
#include "Components/SceneComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASP_AI::ASP_AI()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	spotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("BeaconSpotLight"));


	spotLightComponent->SetupAttachment(GetMesh(), "PerceptionSocket");
}

// this function shoots a projectile and plays a shoot sound
void ASP_AI::Fire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FTransform spawnLocation = GetMesh()->GetSocketTransform(TEXT("Weapon_Socket"), ERelativeTransformSpace::RTS_World);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// spawn the projectile at the muzzle
				World->SpawnActor<ASP_00Projectile>(ProjectileClass, spawnLocation, ActorSpawnParams);
			
		}

	}

	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (FireAM != NULL)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAM);
		}
	}
}

// this function defines the entity as dead and playes its death anim
void ASP_AI::DeathHandle()
{
	isDead = true;

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	GetMesh()->PlayAnimation(deathAnim, false);

	mainMat->SetScalarParameterValue("FadeInElectric", 1.0f);

	spotLightComponent->DestroyComponent();
}

// Called when the game starts or when spawned
void ASP_AI::BeginPlay()
{
	Super::BeginPlay();

	if (Controller != nullptr)
	{
		SP_AIController = Cast<ASP_AIController>(Controller);

		if (SP_AIController != nullptr)
		{
			// init serialized blackboard values
			UBlackboardComponent* BB;

			BB = SP_AIController->GetBlackboardComponent();

			if (BB != nullptr)
			{
				BB->SetValueAsFloat(TEXT("CoolDown"), FireCooldown);
				BB->SetValueAsFloat(TEXT("DistanceToStop"), DistanceToStop);
				BB->SetValueAsFloat(TEXT("StopPurchase"), StopPurchase);
				BB->SetValueAsFloat(TEXT("WarnDelay"), WarnDelay);

				for (int i = 0; i < areas.Num(); i++)
				{
					patrolsRoutes.Append(AAIHelper::GetSingleton()->GetPatrolRoute(areas[i]));
				}

				if (patrolsRoutes.Num() > 0)
				{
					BB->SetValueAsVector(TEXT("NextWayPoint"), patrolsRoutes[0].pos);
				}

			}
		}
	}

	currentState = RobotAI_State::Idle_Default;

	isDead = fadeOut = false;

	timerDeathSequence = timerFadeOutDeathSequence =  0.0f;

	mainMat = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), this);
	GetMesh()->SetMaterial(0, mainMat);
}

// Called every frame
void ASP_AI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isDead)
	{
		if (fadeOut)
		{
			timerFadeOutDeathSequence += DeltaTime;

			if (timerFadeOutDeathSequence >= deathSequenceFadeOut)
			{
				timerFadeOutDeathSequence = deathSequenceFadeOut;

				SetActorTickEnabled(false);
			}

			mainMat->SetScalarParameterValue("FadeInElectric", FMath::Lerp(1.0f, 0.0f, timerFadeOutDeathSequence / deathSequenceFadeOut));
		}
		else
		{
			timerDeathSequence += DeltaTime;

			if (timerDeathSequence >= deathSequenceTimer)
			{
				timerDeathSequence = deathSequenceTimer;

				fadeOut = true;
			}
		}
	}
}

// Called to bind functionality to input
void ASP_AI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

AAIHelper* ASP_AI::GetHelper()
{
	return AAIHelper::GetSingleton();
}

void ASP_AI::GetPerceptionLockRot_Implementation(FVector & OutLocation, FRotator & OutRotation) const
{
	OutLocation = GetActorLocation();
	OutRotation = GetActorRotation();
}

void ASP_AI::TakeDamage(int DamageValue)
{
	HP -= DamageValue;

	if (HP <= 0)
	{
		HP = 0;

		if (!isDead)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "AI Agent died");
			DeathHandle();
			SP_AIController->DeathHandle();
		}
	}
}

void ASP_AI::GetActorEyesViewPoint(FVector & Location, FRotator & Rotation) const
{
	GetPerceptionLockRot(Location, Rotation);
}

// this function changes the model's emissives according to the entity's state as a feedback to wether the player is seen
void ASP_AI::SwitchAIState(RobotAI_State newState)
{
	if (currentState != newState)
	{
		currentState = newState;
	}

	switch (currentState)
	{
	case RobotAI_State::Patrol_Default:
	case RobotAI_State::Idle_Default:
	case RobotAI_State::Idle_LookAround:
		mainMat->SetVectorParameterValue("EmissiveColor", colorStateDefault);
		spotLightComponent->SetLightColor(colorStateDefault);
		mainMat->SetScalarParameterValue("EmissiveIntensity", colorIntensityStateDefault);
		spotLightComponent->SetIntensity(colorLightIntensityStateDefault);
		break;
	case RobotAI_State::Patrol_Suspicious:
	case RobotAI_State::Idle_Suspicious:
		mainMat->SetVectorParameterValue("EmissiveColor", colorStateSuspicious);
		spotLightComponent->SetLightColor(colorStateSuspicious);
		mainMat->SetScalarParameterValue("EmissiveIntensity", colorIntensityStateSuspicious);
		spotLightComponent->SetIntensity(colorLightIntensityStateSuspicious);
		break;
	case RobotAI_State::Attack:
		mainMat->SetVectorParameterValue("EmissiveColor", colorStateAttack);
		spotLightComponent->SetLightColor(colorStateAttack);
		mainMat->SetScalarParameterValue("EmissiveIntensity", colorIntensityStateAttack);
		spotLightComponent->SetIntensity(colorLightIntensityStateAttack);
		break;
	}
}
