// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class SP_00_API SP_00Magazine
{
private:
	int nbMaxBullet;
	int nbCurrentBullet;


public:
	SP_00Magazine();
	SP_00Magazine(int size);
	~SP_00Magazine();

	// returns false if max capacity is exceeded
	bool setNbCurrentBullet(int nbBullet);
	int getNbCurrentBullet();

	FORCEINLINE float getCurrentMagazineSizePercentage() { return (float)nbCurrentBullet / (float)nbMaxBullet; }


	void setNbMaxBullet(int nbBullet);
	int getNbMaxBullet();

	bool canFire();

};
