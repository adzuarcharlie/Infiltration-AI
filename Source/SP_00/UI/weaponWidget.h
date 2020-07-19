// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "weaponWidget.generated.h"

/**
 * 
 */
UCLASS()
class SP_00_API UweaponWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UIWeapon")
	void UpdateMunitionInfo(int currentAmount, float currentMunitionPercentage);


};