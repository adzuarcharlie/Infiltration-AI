// Fill out your copyright notice in the Description page of Project Settings.


#include "AIWayPoint.h"

// Sets default values
AAIWayPoint::AAIWayPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	transform = CreateDefaultSubobject<USceneComponent>("transform");
}