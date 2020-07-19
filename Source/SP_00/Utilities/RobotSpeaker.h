// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RobotSpeaker.generated.h"

/**
 * 
 */
UCLASS()
class SP_00_API ARobotSpeaker : public AActor
{
	GENERATED_BODY()
	ARobotSpeaker();
protected:
	virtual void BeginPlay() override;
public:
	UPROPERTY(EditAnyWhere)
		class UAudioComponent* audioComp;

	UPROPERTY(EditAnyWhere)
	TMap<FString, USoundBase*> mapSound;

	UFUNCTION(BlueprintCallable)
		void SetSpeakerSound(const FString& soundName);

};
