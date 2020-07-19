// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SP_00GameMode.generated.h"

UCLASS(minimalapi)
class ASP_00GameMode : public AGameModeBase
{
	GENERATED_BODY()
	virtual void PreInitializeComponents() override;

	virtual void BeginPlay() override;
	
	AActor* realPlayer;

public:
	ASP_00GameMode();

	UFUNCTION(BlueprintCallable)
		void ChangePlayerPossessTarget(AActor* ref);

	UPROPERTY(EditAnyWhere)
		bool isSubLevelGameMode;
};



