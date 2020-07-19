// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AIWayPoint.h"

#include "AIHelper.generated.h"

UENUM(BlueprintType)
enum EAIOrderType
{
	SEEK,
	TRACK,
	GOTO,
	CHECK
};

USTRUCT(BlueprintType)
struct FAIOrder
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite)
		TEnumAsByte<EAIOrderType> type;

	UPROPERTY(BlueprintReadWrite)
		bool deletedWhenPlayerIsSpotted; // some orders must be deleted when the player is found, like Seek, Track or Check

	// SEEK VALUES
	UPROPERTY(BlueprintReadWrite)
		TArray<FVector> seekPos; // grid of pos

	// TRACK VALUES
	UPROPERTY(BlueprintReadWrite)
		FVector trackPos; // player pos

// GOTO VALUES
	UPROPERTY(BlueprintReadWrite)
		FVector gotoPos;

	UPROPERTY(BlueprintReadWrite)
		float stopTime; // how much time does it stay on the gotoPos (overwritten if checkWhenArrived = true)

	UPROPERTY(BlueprintReadWrite)
		bool hasArrived; // used to know when to wait/check around

	// CHECK VALUES
	UPROPERTY(BlueprintReadWrite)
		float timer; // time the entity pauses for

	UPROPERTY(BlueprintReadWrite)
		FRotator baseRotation;

	UPROPERTY(BlueprintReadWrite)
		bool suspicious; // used to know which anim must be played (there is a normal check anim and a suspicious check anim)
};

enum class GenericTeamIdCustom : uint8
{
	playerID = 1,
	AI_ID = 2,
	NeutralID = 3
};

class ASP_AI;

UCLASS()
class SP_00_API AAIHelper : public AActor
{
	GENERATED_BODY()
private:
	TMap<FName, TArray<ASP_AI*>> entities; // all entities in the level

	TMap<FName, TArray<ASP_AI*>> viewers; // all entities currently seeing the player

	FVector playerPos;

	TMap<FName, TArray<FVector>> searchZones;

	FAIOrder MakeSearchZone(FVector _pos, float _radius, int nbChunks, AActor* _owner, float cellSize);

	void ClearCheckedPoints(FName _squad, float _sightDistance, float _sightAngle, float _senseDistance);

	TMap<FString, TArray<FAIWayPointData>> patrolsPoint;

	static AAIHelper *instance;

	~AAIHelper();

public:
	// Sets default values for this actor's properties
	AAIHelper();

	TArray<FAIWayPointData> GetPatrolRoute(FString patrolArea);

	UFUNCTION(BlueprintCallable, BlueprintPure) static AAIHelper* GetSingleton() { return instance; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitManual();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// There functions are to be called in AI Behaviour
	UFUNCTION(BlueprintCallable)
		FVector ChooseNextSeekPos(FName _squad, FVector _pos);

	UFUNCTION(BlueprintCallable)
		FAIWayPointData ChooseNextPatrolPos(ASP_AI* _entity);

	UFUNCTION(BlueprintCallable)
		FAIWayPointData GetCurrentPatrolPosData(FVector _pos);

	UFUNCTION(BlueprintCallable)
		void AddEntity(ASP_AI* _entity);

	UFUNCTION(BlueprintCallable)
		void EntitySeesPlayer(ASP_AI* _entity);

	UFUNCTION(BlueprintCallable)
		void EntityLostPlayer(ASP_AI* _entity);

	UFUNCTION(BlueprintCallable)
		void UpdatePlayerPos(FVector _pos);

	UFUNCTION(BlueprintCallable)
		bool SearchZoneHasPoint(FName _squad, FVector _pos);

	UFUNCTION(BlueprintCallable)
		bool IsSearchZoneFinished(FName _squad);
};