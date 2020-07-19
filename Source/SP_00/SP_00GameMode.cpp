// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SP_00GameMode.h"
#include "UI/SP_00HUD.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/SP_00Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Player/SP_00PlayerController.h"

void ASP_00GameMode::PreInitializeComponents()
{
	TArray<AActor*> Container;
	UGameplayStatics::GetAllActorsOfClass(Cast<const UObject>(GetWorld()), ASP_00Character::StaticClass(), Container);
	Super::PreInitializeComponents();
	if (Container.Num() == 0)
	{
		realPlayer = nullptr;
	}
	else
	{
		realPlayer = Container[0];
	}
	
}

void ASP_00GameMode::BeginPlay()
{
	APawn* toDelete = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (realPlayer != nullptr && toDelete != realPlayer)
	{
		realPlayer->SetActorTransform(toDelete->GetTransform());
		GetWorld()->GetFirstPlayerController()->Possess(Cast<APawn>(realPlayer));
		toDelete->Destroy();
	}
}

ASP_00GameMode::ASP_00GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/SilentProject/Player/Blueprints/PlayerCharacterBlueprint"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	// use our custom HUD class
	HUDClass = ASP_00HUD::StaticClass();

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/SilentProject/Player/Controller/SP_00Controller_BP"));

	PlayerControllerClass = PlayerControllerClassFinder.Class;
}

void ASP_00GameMode::ChangePlayerPossessTarget(AActor * ref)
{
	APawn* toSave = GetWorld()->GetFirstPlayerController()->GetPawn();

	if (ref != nullptr && toSave != ref)
	{
		ref->Destroy();
		ref = toSave;
	}
}
