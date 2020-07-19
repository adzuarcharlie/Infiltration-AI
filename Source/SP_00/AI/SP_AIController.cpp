// Fill out your copyright notice in the Description page of Project Settings.


#include "SP_AIController.h"

#include "BrainComponent.h"

UBehaviorTree* ASP_AIController::GetBT()
{
	return BTAsset;
}

UBlackboardData* ASP_AIController::GetBB()
{
	return BBAsset;
}

UBlackboardComponent* ASP_AIController::GetBlackboard()
{
	return currentBB;
}

// this function adds an order to the list at the correct place (wether it has priority, if it replaces former order of the same type etc)
void ASP_AIController::GiveOrder(FAIOrder _order, bool _isPriority, bool _isReplacing, bool _isPriorityOverExistingOrder)
{
	bool hasReplaced = false;
	if (!_isPriorityOverExistingOrder)
	{
		for (int32 i = 0; i < Orders.Num() && !hasReplaced; i++)
		{
			if (Orders[i].type == _order.type)
			{
				return;
			}
		}
	}
	if (_isReplacing)
	{
		for (int32 i = 0; i < Orders.Num() && !hasReplaced; i++)
		{
			if (Orders[i].type == _order.type)
			{
				Orders[i] = _order;
				hasReplaced = true;
			}
		}
	}
	if (!hasReplaced) // only insert if it didn't replace any element of the array
	{
		if (_isPriority)
		{
			Orders.Insert(_order, 0);
		}
		else
		{
			Orders.Add(_order);
		}
	}
}

bool ASP_AIController::HasOrder()
{
	return Orders.Num() > 0;
}

FAIOrder ASP_AIController::GetCurrentOrder()
{
	FAIOrder defaultValue;
	return HasOrder() ? Orders[0] : defaultValue;
}

void ASP_AIController::ClearOrders()
{
	for (int i = 0; i < Orders.Num(); i++)
	{
		if (Orders[i].deletedWhenPlayerIsSpotted)
		{
			Orders.RemoveAt(i);
			i--;
		}
	}
}

void ASP_AIController::RemoveCurrentOrder()
{
	if (HasOrder())
		Orders.RemoveAt(0);
}

void ASP_AIController::ReplaceCurrentOrder(FAIOrder _order)
{
	if (HasOrder())
		Orders[0] = _order;
}

void ASP_AIController::RemoveOrdersOfType(EAIOrderType _typeToRemove)
{
	for (int32 i = Orders.Num() - 1; i >= 0; i--)
	{
		if (Orders[i].type == _typeToRemove)
		{
			Orders.RemoveAt(i);
		}
	}
}

void ASP_AIController::DeathHandle()
{
	StopMovement();
	GetBrainComponent()->StopLogic("isDead");
}

void ASP_AIController::OnPossess(APawn * InPawn)
{
	Super::OnPossess(InPawn);

	if (BTAsset != nullptr)
	{
		RunBehaviorTree(BTAsset);
	}

	if (BBAsset != nullptr)
	{
		UseBlackboard(BBAsset, currentBB);

	}
}
