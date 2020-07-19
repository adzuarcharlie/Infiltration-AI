// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_00PlayerController.h"
#include "Camera/CameraShake.h"

void ASP_00PlayerController::ApplyShake(FName shakeName)
{
	if (mapShakes.Contains(shakeName))
	{
		playingShakes.Add(shakeName, PlayerCameraManager->PlayCameraShake(mapShakes[shakeName]));
	}
}

void ASP_00PlayerController::RemoveShake(FName shakeName)
{
	if (playingShakes.Contains(shakeName))
	{
		PlayerCameraManager->StopCameraShake(playingShakes[shakeName]);
		playingShakes.Remove(shakeName);
	}
}
