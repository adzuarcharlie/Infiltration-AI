// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_00Magazine.h"

SP_00Magazine::SP_00Magazine()
{
	nbMaxBullet = 12;
	nbCurrentBullet = 12;
}

SP_00Magazine::SP_00Magazine(int size)
{
	nbCurrentBullet = nbMaxBullet = size;
}

SP_00Magazine::~SP_00Magazine()
{
}

//retourne false si on depasse la capasite maximal du chargeur
bool SP_00Magazine::setNbCurrentBullet(int nbBullet)
{
	if (nbBullet <= nbMaxBullet)
	{
		nbCurrentBullet = nbBullet;
		return true;
	}
	return false;
}

int SP_00Magazine::getNbCurrentBullet()
{
	return nbCurrentBullet;
}

void SP_00Magazine::setNbMaxBullet(int nbBullet)
{
	nbMaxBullet = nbBullet;
}

int SP_00Magazine::getNbMaxBullet()
{
	return nbMaxBullet;
}

bool SP_00Magazine::canFire()
{
	if (nbCurrentBullet > 0)
	{
		nbCurrentBullet--;
		return true;
	}
	return false;
}


