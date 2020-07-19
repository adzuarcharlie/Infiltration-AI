// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SP_00PlayerController.generated.h"

/**
 Custom class empty for template purposes
 */
UCLASS()
class SP_00_API ASP_00PlayerController : public APlayerController
{
	GENERATED_BODY()
private:
	TMap<FName, class UCameraShake*> playingShakes;
public:
	UPROPERTY(EditAnyWhere)
	TMap<FName, TSubclassOf <class UCameraShake>> mapShakes;


	UFUNCTION(BlueprintCallable)
		void ApplyShake(FName shakeName);

	UFUNCTION(BlueprintCallable)
		void RemoveShake(FName shakeName);
	
};
