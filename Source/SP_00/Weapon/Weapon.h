// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/SP_00Magazine.h"
#include "Weapon.generated.h"


class UWidgetComponent;
class UweaponWidget;
class ASP_00Character;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SP_00_API UWeapon : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:	
	// Sets default values for this component's properties
	UWeapon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Location on gun mesh where projectiles should spawn. */
	class USceneComponent* MuzzleLocation;

	UWidgetComponent* weaponWidget;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	int MagazineSize = 8;

	void OnFire();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class ASP_00Projectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector GunOffset;

	ASP_00Character* Characterref;

private:

	SP_00Magazine magazine;

	UweaponWidget* WW;
};
