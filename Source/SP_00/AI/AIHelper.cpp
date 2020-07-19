// Fill out your copyright notice in the Description page of Project Settings.


#include "AIHelper.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "SP_AIController.h"
#include "Kismet/GameplayStatics.h"
#include "AIWayPoint.h"
#include "SP_AI.h"

AAIHelper* AAIHelper::instance = nullptr;

// Sets default values
AAIHelper::AAIHelper()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AAIHelper::~AAIHelper()
{
	instance = nullptr;
}

TArray<FAIWayPointData> AAIHelper::GetPatrolRoute(FString patrolArea)
{
	if (patrolsPoint.Contains(patrolArea))
	{
		return patrolsPoint[patrolArea];
	}
	else
	{
		return TArray<FAIWayPointData>();
	}
}

// Called when the game starts or when spawned
void AAIHelper::BeginPlay()
{
	Super::BeginPlay();

	if (instance == nullptr)
	{
		instance = this;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Multiple instance AIHelper founded !");
	}


	InitManual();
}

void AAIHelper::InitManual()
{
	TArray<AActor*> wayPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAIWayPoint::StaticClass(), wayPoints);

	for (int i = 0; i < wayPoints.Num(); i++)
	{
		AAIWayPoint* wp = Cast<AAIWayPoint>(wayPoints[i]);
		FAIWayPointData newWPD;
		newWPD.area = wp->area;
		newWPD.pos = wp->GetActorLocation();
		newWPD.baseRotation = wp->GetActorRotation();
		newWPD.timer = wp->timer;
		newWPD.orderInPath = wp->orderInPath;

		if (patrolsPoint.Contains(newWPD.area))
		{
			patrolsPoint[newWPD.area].Add(newWPD);
		}
		else
		{
			patrolsPoint.Add(newWPD.area, TArray<FAIWayPointData>());
			patrolsPoint[newWPD.area].Add(newWPD);
		}
	}
}

// Called every frame
void AAIHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// for every entity, if there is an entity from the same squad seeing the player, it gives them the player's position so that they
	// track him
	for (auto squad = viewers.CreateConstIterator(); squad; ++squad)
	{
		if (squad.Value().Num() > 0)
		{
			FAIOrder order;
			order.type = EAIOrderType::TRACK;
			order.trackPos = playerPos;
			order.deletedWhenPlayerIsSpotted = true;
			if (entities.Contains(squad.Key()))
			{
				for (int i = 0; i < entities[squad.Key()].Num(); i++)
				{
					if (!squad.Value().Contains(entities[squad.Key()][i]))
					{
						AController* Controller = entities[squad.Key()][i]->GetController();
						if (Controller != nullptr)
						{
							ASP_AIController* RealController = Cast<ASP_AIController>(Controller);

							if (RealController != nullptr)
							{
								RealController->GiveOrder(order, true, true, true);
								RealController->RemoveOrdersOfType(EAIOrderType::CHECK);
								RealController->RemoveOrdersOfType(EAIOrderType::SEEK);
							}
						}
					}
				}
			}
		}
	}

	// if entities are looking for the player, we clear seen areas
	for (auto zone = searchZones.CreateIterator(); zone; ++zone)
		if (zone.Value().Num() > 0)
		{
			ClearCheckedPoints(zone.Key(), 500.f, 60.f, 200.f);
		}
}

// when the player is lost, we create a search zone around the last seen pos
FAIOrder AAIHelper::MakeSearchZone(FVector _pos, float _radius, int nbChunks, AActor* _owner, float cellSize)
{
	TArray<FVector> centers;
	FAIOrder order;
	order.type = EAIOrderType::SEEK;
	order.deletedWhenPlayerIsSpotted = true;
	order.suspicious = true;
	centers.Add(_pos);
	unsigned char isTaken = 0; // 8 bools
	if (nbChunks >= 8)
	{
		for (int i = 0; i < 8; i++)
		{
			float angle = i * PI / 4.f;
			FVector temp = _pos;
			temp.X += FMath::Cos(angle) * _radius * 2.f;
			temp.Y += FMath::Sin(angle) * _radius * 2.f;
			centers.Add(temp);
		}
	}
	else
	{
		for (int i = 0; i < nbChunks; i++)
		{
			int randAngle;
			do
			{
				randAngle = FMath::RandRange(0, 7);
			} while (isTaken & (1 << randAngle));
			float angle = randAngle * PI / 4.f;
			FVector temp = _pos;
			temp.X += FMath::Cos(angle) * _radius * 2.f;
			temp.Y += FMath::Sin(angle) * _radius * 2.f;
			centers.Add(temp);
			isTaken |= 1 << randAngle;
		}
	}
	// make search field
	for (int i = 0; i < centers.Num(); i++)
	{
		for (float y = centers[i].Y - _radius + cellSize / 2.f; y < centers[i].Y + _radius; y += cellSize)
		{
			for (float x = centers[i].X - _radius + cellSize / 2.f; x < centers[i].X + _radius; x += cellSize)
			{
				FVector waypoint = FVector(x, y, _pos.Z - 100.f);
				FNavLocation outLocation; // trash data
				// we project the point on the navmesh ; if no point is found, it means this position is out of reach
				if (_owner->GetWorld()->GetNavigationSystem()->GetMainNavData()->ProjectPoint(waypoint, outLocation, FVector(0.1f, 0.1f, 150.f)))
				{
					order.seekPos.Add(waypoint);
				}
			}
		}
	}

	return order;
}

// this function clears all points that are currently being seen by any entity
void AAIHelper::ClearCheckedPoints(FName _squad, float _sightDistance, float _sightAngle, float _senseDistance)
{
	if (!entities.Contains(_squad) || entities[_squad].Num() == 0
		|| !searchZones.Contains(_squad)) return; // this security shouldn't be useful, but you never know
	UWorld* world = entities[_squad][0]->GetWorld();
	for (int j = 0; j < entities[_squad].Num(); j++)
	{
		for (int i = searchZones[_squad].Num() - 1; i >= 0; i--)
		{
			FHitResult hit;
			if (world->LineTraceSingleByChannel(hit, searchZones[_squad][i], entities[_squad][j]->GetActorLocation(), ECollisionChannel::ECC_Camera)) // if the entity has a line of sight to the point
			{
				FVector toPoint = searchZones[_squad][i] - entities[_squad][j]->GetActorLocation();
				if (toPoint.Size() < _senseDistance) // the entity can sense points that are right behind it
				{
					searchZones[_squad].RemoveAt(i);
				}
				else if (toPoint.Size() < _sightDistance && FVector::DotProduct(toPoint, entities[_squad][j]->GetActorForwardVector()) > cos(_sightAngle)) // or the points that are in front of it and in a certain distance
				{
					searchZones[_squad].RemoveAt(i);
				}
			}
		}
	}
}

// this function returns the closest point among the search zone
FVector AAIHelper::ChooseNextSeekPos(FName _squad, FVector _pos)
{
	if (!searchZones.Contains(_squad) || searchZones[_squad].Num() == 0)
	{
		return _pos;
	}
	FVector result = searchZones[_squad][0];
	float bestDistance = FVector::Distance(result, _pos);
	float currentDistance;
	for (int i = 1; i < searchZones[_squad].Num(); i++)
	{
		currentDistance = FVector::Distance(searchZones[_squad][i], _pos);
		if (currentDistance < bestDistance)
		{
			result = searchZones[_squad][i];
			bestDistance = currentDistance;
		}
	}
	return result;
}

// this function chooses the next pos an entity has to go to, according to the areas it is allowed to reach
// and wether or not it follows a predefined path order
FAIWayPointData AAIHelper::ChooseNextPatrolPos(ASP_AI* _entity)
{
	if (_entity->areas.Num() <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Entity has no patrol zone !");
		return FAIWayPointData();
	}

	if (_entity->usePathOrder)
	{
		FString area = _entity->areas[0]; // only the first area is taken when the entity follows the path order
		if (patrolsPoint.Contains(area))
		{
			int nbPoints = 0;
			// count points that follow the current waypoint
			for (int i = 0; i < patrolsPoint[area].Num(); i++)
			{
				if (patrolsPoint[area][i].orderInPath == _entity->waypointIndex + 1)
				{
					nbPoints++;
				}
			}
			if (nbPoints == 0) // if none is found
			{
				// maybe we are at the end of the path, so we look for the first point
				for (int i = 0; i < patrolsPoint[area].Num(); i++)
				{
					if (patrolsPoint[area][i].orderInPath == 0)
					{
						nbPoints++;
					}
				}
				if (nbPoints > 0) // if some points are found, get a random point within them
				{
					int randPoint = rand() % nbPoints;
					int currentIndex = 0;
					for (int i = 0; i < patrolsPoint[area].Num(); i++)
					{
						if (patrolsPoint[area][i].orderInPath == 0)
						{
							if (currentIndex == randPoint)
							{
								_entity->waypointIndex = 0;
								return patrolsPoint[area][i];
							}
							else
							{
								currentIndex++;
							}
						}
					}
					// if we haven't reached the return yet, there is a problem
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Couldn't find next waypoint, but there should be one ! (start of path)");
					return FAIWayPointData();
				}
				else // if no point is found at index 0, there is a problem !
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "There is no next waypoint !");
					return FAIWayPointData();
				}
			}
			else // if some points are found, get a random point within them
			{
				int randPoint = rand() % nbPoints;
				int currentIndex = 0;
				for (int i = 0; i < patrolsPoint[area].Num(); i++)
				{
					if (patrolsPoint[area][i].orderInPath == _entity->waypointIndex + 1)
					{
						if (currentIndex == randPoint)
						{
							_entity->waypointIndex++;
							return patrolsPoint[area][i];
						}
						else
						{
							currentIndex++;
						}
					}
				}
				// if we haven't reached the return yet, there is a problem
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Couldn't find next waypoint, but there should be one ! (middle or end of path)");
				return FAIWayPointData();
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Patrol zone not found !");
			return FAIWayPointData();
		}
	}
	else // if the entity doesn't follow a predefined path order, simply choose a random point from a random zone
	{
		FString area = _entity->areas[rand() % _entity->areas.Num()];
		if (patrolsPoint.Contains(area))
		{
			return patrolsPoint[area][rand() % patrolsPoint[area].Num()];
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Patrol zone not found !");
			return FAIWayPointData();
		}
	}
}

// this function returns the data from the point placed at a given position
// (useful to know how much time an entity has to wait when arrived, for instance)
FAIWayPointData AAIHelper::GetCurrentPatrolPosData(FVector _pos)
{
	for (auto path = patrolsPoint.CreateConstIterator(); path; ++path)
	{
		for (int i = 0; i < path.Value().Num(); i++)
		{
			if (FVector::Dist(path.Value()[i].pos, _pos) < 1.f)
			{
				return path.Value()[i];
			}
		}
	}
	return FAIWayPointData();
}

// this function is called at the beginning of the level, it adds the given entity to the AIHelper so it can interact with other entities
void AAIHelper::AddEntity(ASP_AI* _entity)
{
	if (!entities.Contains(_entity->squad)) // no entity from this squad is stored yet
	{
		entities.Add(_entity->squad).Add(_entity);
		// also store squad for search zones
		searchZones.Add(_entity->squad);
	}
	else
	{
		// the squad exists in the map but the entity hasn't been stored yet
		if (!entities[_entity->squad].Contains(_entity))
		{
			entities[_entity->squad].Add(_entity);
		}
	}
}

// this function is called when an entity finds the player, it adds it to the viewers list
// the Tick() will do the rest
void AAIHelper::EntitySeesPlayer(ASP_AI* _entity)
{
	if (!viewers.Contains(_entity->squad)) // the squad hasn't spotted the player yet
	{
		viewers.Add(_entity->squad).Add(_entity);
	}
	else
	{
		// the squad exists in the map but the entity hadn't seen the player so far
		if (!viewers[_entity->squad].Contains(_entity))
		{
			viewers[_entity->squad].Add(_entity);
		}
	}
}

// this function is called when an entity loses the player, it removes it from the viewers list
// then, if nobody is seeing the player anymore, it creates the search zone and asks entities to look for the player
void AAIHelper::EntityLostPlayer(ASP_AI* _entity)
{
	if (!viewers.Contains(_entity->squad)) return; // if the squad hasn't been registered, there is a problem

	if (viewers[_entity->squad].Contains(_entity))
	{
		viewers[_entity->squad].Remove(_entity);
		// if every entity from this squad lost the player, then they look for him
		if (viewers[_entity->squad].Num() == 0)
		{
			FAIOrder order = MakeSearchZone(playerPos, 500.f, 4, _entity, 100.f);
			if (!searchZones.Contains(_entity->squad))
			{
				searchZones.Add(_entity->squad, order.seekPos);
			}
			else
			{
				searchZones[_entity->squad] = order.seekPos;
			}

			if (!entities.Contains(_entity->squad)) return; // again, if the squad hasn't been registered, problem

			for (int i = 0; i < entities[_entity->squad].Num(); i++)
			{
				AController* Controller = entities[_entity->squad][i]->GetController();
				if (Controller != nullptr)
				{
					ASP_AIController* RealController = Cast<ASP_AIController>(Controller);

					if (RealController != nullptr)
					{
						RealController->GiveOrder(order, true, true, true);
						RealController->RemoveOrdersOfType(EAIOrderType::TRACK);
					}
				}
			}
		}
	}
}

void AAIHelper::UpdatePlayerPos(FVector _pos)
{
	playerPos = _pos;
}

bool AAIHelper::SearchZoneHasPoint(FName _squad, FVector _pos)
{
	if (!searchZones.Contains(_squad)) return false;
	for (int i = 0; i < searchZones[_squad].Num(); i++)
	{
		if (FVector::Dist(_pos, searchZones[_squad][i]) < 50.f)
		{
			return true;
		}
	}
	return false;
}

// this returns wether or not the search zone is cleared, so that entities can run their default behaviour when they're done searching
bool AAIHelper::IsSearchZoneFinished(FName _squad)
{
	if (!searchZones.Contains(_squad))
		return true;
	return searchZones[_squad].Num() == 0;
}