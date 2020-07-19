// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"

#include "UI/weaponWidget.h"
#include "Components/WidgetComponent.h"

#include "Projectiles/SP_00Projectile.h"
#include "Player/SP_00Character.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

#include "Animation/AnimInstance.h"


// Sets default values for this component's properties
UWeapon::UWeapon()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	this->bCastDynamicShadow = false;
	this->CastShadow = false;
}


// Called when the game starts
void UWeapon::BeginPlay()
{
	Super::BeginPlay();

	weaponWidget->InitWidget();

	magazine.setNbMaxBullet(MagazineSize);
	magazine.setNbCurrentBullet(MagazineSize);

	WW = Cast<UweaponWidget>(weaponWidget->GetUserWidgetObject());
	if (WW != NULL)
	{
		WW->UpdateMunitionInfo(magazine.getNbCurrentBullet(), magazine.getCurrentMagazineSizePercentage());
	}
	
}


// Called every frame
void UWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeapon::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (magazine.canFire() && Characterref != NULL && Characterref->hasWeapon)
			{

				int currentBullet = magazine.getNbCurrentBullet();

				if (WW != NULL)
				{
					WW->UpdateMunitionInfo(magazine.getNbCurrentBullet(), magazine.getCurrentMagazineSizePercentage());
				}

			//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::FromInt(currentBullet));

				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = MuzzleLocation->GetComponentLocation();
				FVector forward = MuzzleLocation->GetForwardVector();


				// I made this to fix my co-worker's work because the bullet would spawn inside the player
				FTransform spawnTransform = MuzzleLocation->GetComponentTransform();
				spawnTransform.AddToTranslation(forward * 20.f);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<ASP_00Projectile>(ProjectileClass, spawnTransform, ActorSpawnParams);

			
			}
			else
			{
				return;
			}
		}
	}

	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetOwner()->GetActorLocation());
	}

	if (FireAnimation != NULL)
	{
		UAnimInstance* AnimInstance = this->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation);
		}
	}

	if (Characterref != NULL)
	{
		Characterref->CallbackShotPlayer();
	}
}



