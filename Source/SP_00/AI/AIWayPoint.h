// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "AIWayPoint.generated.h"

USTRUCT(BlueprintType)
struct FAIWayPointData
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite)
		FVector pos;

	UPROPERTY(BlueprintReadWrite)
		FString area;

	UPROPERTY(BlueprintReadWrite)
		FRotator baseRotation;

	UPROPERTY(BlueprintReadWrite)
		float timer;

	int orderInPath;
};

UCLASS()
class SP_00_API AAIWayPoint : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AAIWayPoint();

	UPROPERTY(BlueprintReadOnly)
		USceneComponent* transform;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FString area = "default";

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		float timer = 2.f;

	UPROPERTY(EditAnywhere)
		int orderInPath = 0;
};
