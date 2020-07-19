// Fill out your copyright notice in the Description page of Project Settings.

#include "RobotSpeaker.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"

ARobotSpeaker::ARobotSpeaker()
{
	audioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));

	audioComp->SetupAttachment(RootComponent);
}



void ARobotSpeaker::BeginPlay()
{
	Super::BeginPlay();
}

void ARobotSpeaker::SetSpeakerSound(const FString& soundName)
{
	if (audioComp->Sound != NULL)
	{
		audioComp->Stop();
	}

	if (mapSound.Contains(soundName))
	{
		audioComp->SetSound(mapSound[soundName]);
		audioComp->Play();
	}
}
