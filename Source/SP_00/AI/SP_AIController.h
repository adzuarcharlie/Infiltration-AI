// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIHelper.h"
#include "SP_AIController.generated.h"

/**
 *
 */
UCLASS()
class SP_00_API ASP_AIController : public AAIController
{
	GENERATED_BODY()
		TArray<FAIOrder> Orders;
protected:

	UPROPERTY(EditAnywhere)
		UBehaviorTree* BTAsset;

	UPROPERTY(EditAnywhere)
		UBlackboardData* BBAsset;

	UPROPERTY()
		UBlackboardComponent* currentBB;

	UFUNCTION(BlueprintCallable)
		UBehaviorTree* GetBT();

	UFUNCTION(BlueprintCallable)
		UBlackboardData* GetBB();

	UFUNCTION(BlueprintCallable)
		UBlackboardComponent* GetBlackboard();
public:

	UFUNCTION(BlueprintCallable)
		// add an order to the list
		void GiveOrder(FAIOrder _order, bool _isPriority, bool _isReplacing, bool _isPriorityOverExistingOrder);

	UFUNCTION(BlueprintCallable)
		bool HasOrder();

	UFUNCTION(BlueprintCallable)
		// get the first order of the array, which is the one the entity is executing
		FAIOrder GetCurrentOrder();

	UFUNCTION(BlueprintCallable)
		void ClearOrders();

	UFUNCTION(BlueprintCallable)
		void RemoveCurrentOrder();

	UFUNCTION(BlueprintCallable)
		void ReplaceCurrentOrder(FAIOrder _order);

	UFUNCTION(BlueprintCallable)
		void RemoveOrdersOfType(EAIOrderType _typeToRemove);

	void DeathHandle();

	virtual void OnPossess(APawn* InPawn) override;
};
