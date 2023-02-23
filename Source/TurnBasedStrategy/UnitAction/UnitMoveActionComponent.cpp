// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitMoveActionComponent.h"
#include "../UnitCharacter.h"

#include "../Manager/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

UUnitMoveActionComponent::UUnitMoveActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxActionRange = 5;

	ActionName = FString("Move");
}

void UUnitMoveActionComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UUnitMoveActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bMoveActivate)
	{
		AGridManager* gridManager = AGridManager::GetGridManager();
		if (!IsValid(gridManager))
		{
			UE_LOG(LogTemp, Warning, TEXT("Grid Manager is not Valid"));
			return;
		}

		if (Path.Num() > 0)
		{
			FVector currentLocation = GetOwner()->GetActorLocation();
			FVector worldLocation = gridManager->GridToWorld(Path[0]) + FVector(0.0f, 0.0f, currentLocation.Z);

			float dist = FVector::Distance(currentLocation, worldLocation);

			//UE_LOG(LogTemp, Warning, TEXT("Dist : %f"), dist);

			if (FMath::IsNearlyEqual(dist, 0.0f, 50.0f))
			{
				Path.RemoveAt(0);
				bIsMoving = false;
				return;
			}
			else if(bIsMoving == false)
			{
				//a simple temp Move Function.
				AAIController* aiController = Cast<AAIController>(Unit->GetController());
				if (!IsValid(aiController))
				{
					return;
				}

				aiController->MoveToLocation(worldLocation, 0.0f, false);
				bIsMoving = true;
				//UE_LOG(LogTemp, Warning, TEXT("Moving"));
				return;
			}

		}
		else
		{
			FVector currentLocation = GetOwner()->GetActorLocation();
			FGrid currentGrid = gridManager->WorldToGrid(currentLocation);

			//���� ��ġ�� �������� ���߾����.
			if (currentGrid == Destination)
			{
				//a simple temp Move Function.
				AAIController* aiController = Cast<AAIController>(Unit->GetController());
				if (!IsValid(aiController))
				{
					return;
				}

				//Stop�� ���Ŀ� Rotation �� �ٲ� ���� ������ �ذ��ϱ� ���ؼ� �� �ʿ���.
				aiController->StopMovement();

				if (OnActionEnd.IsBound())
				{
					OnActionEnd.Broadcast();
				}
				UE_LOG(LogTemp, Warning, TEXT("Stopped."));
				return;
			}
		}

	}


}

TArray<FGrid> UUnitMoveActionComponent::GetValidActionGridArray() const
{
	TArray<FGrid> validArray;

	FGrid unitGrid = Unit->GetGrid();

	AGridManager* gridManager = AGridManager::GetGridManager();
	
	if (!IsValid(gridManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid Manager is not Valid"));
		return validArray;
	}


	for (int x = -MaxActionRange; x <= MaxActionRange; x++)
	{
		for (int y = -MaxActionRange; y <= MaxActionRange; y++)
		{
			FGrid resultGrid = FGrid(x, y);
			resultGrid += unitGrid;


			//�������� �ʴ� Grid
			if (!gridManager->IsValidGrid(resultGrid))
			{
				continue;
			}
			
			////���� ���� Unit�� ��ġ
			if (resultGrid == unitGrid)
			{
				continue;
			}

			//������ �������̸� Skip
			if (gridManager->HasAnyUnitOnGrid(resultGrid))
			{
				continue;
			}

			//���� �� �ִ� ��ġ?
			if (!gridManager->IsWalkableGrid(resultGrid))
			{
				continue;
			}

			bool bisFriend = false;
			AUnitCharacter* targetUnit = gridManager->GetUnitAtGrid(resultGrid);
			if (IsValid(targetUnit) && GetOwner()->Tags.Num() > 0)
			{
				bisFriend = targetUnit->ActorHasTag(GetOwner()->Tags[0]);
			}

			//���� ������ ��ġ?
			if (bisFriend && !gridManager->HasPath(unitGrid, resultGrid, true) || !gridManager->HasPath(unitGrid, resultGrid))
			{
				continue;
			}

			//�ǵ��� �޸� �� �Ÿ�?
			if (gridManager->GetPathLength(unitGrid, resultGrid) > MaxActionRange)
			{
				continue;
			}

			//����ϸ� ���������� validArray�� �߰�

			validArray.Add(resultGrid);
		}
	}


	return validArray;
}

TArray<FGridVisualData> UUnitMoveActionComponent::GetValidActionGridVisualDataArray() const
{
	TArray<FGridVisualData> validVisualDataArray;
	FGrid unitGrid = Unit->GetGrid();

	AGridManager* gridManager = AGridManager::GetGridManager();

	if (!IsValid(gridManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid Manager is not Valid"));
		return validVisualDataArray;
	}


	for (int x = -MaxActionRange; x <= MaxActionRange; x++)
	{
		for (int y = -MaxActionRange; y <= MaxActionRange; y++)
		{
			if (FMath::Abs(x) + FMath::Abs(y) > MaxActionRange)
			{
				continue;
			}

			FGrid resultGrid = FGrid(x, y);
			resultGrid += unitGrid;
			
			//�������� �ʴ� Grid
			if (!gridManager->IsValidGrid(resultGrid))
			{
				continue;
			}

			FGridVisualData testData;
			testData.Grid = resultGrid;
			testData.GridVisualType = EGridVisualType::Move;

			if (resultGrid == unitGrid) //���� ���� Unit�� ��ġ
			{
				testData.GridVisualType = EGridVisualType::Warning;
			}
			else if (gridManager->HasAnyUnitOnGrid(resultGrid) || //������ ������
				!gridManager->IsWalkableGrid(resultGrid) || //���� �� ���� ��ġ?
				!gridManager->HasPath(unitGrid, resultGrid, false) || //���� �Ұ����� ��ġ?
				gridManager->GetPathLength(unitGrid, resultGrid) > MaxActionRange) 	//�ǵ��� �޸� �� �Ÿ�?
			{
				testData.GridVisualType = EGridVisualType::NO;
			}

			bool bisFriend = false;
			auto targetUnit = gridManager->GetUnitAtGrid(resultGrid);
			if (IsValid(targetUnit) && GetOwner()->Tags.Num()>0)
			{
				bisFriend = targetUnit->ActorHasTag(GetOwner()->Tags[0]);
				if (bisFriend) // ���� �Ʊ� ��ġ��� ��������� ������.
				{
					testData.GridVisualType = EGridVisualType::Warning;
				}
			}
			
			if (targetUnit == GetOwner())
			{
				testData.GridVisualType = EGridVisualType::Move;
			}


			//����ϸ� ���������� validArray�� �߰�

			validVisualDataArray.Add(testData);
		}
	}


	return validVisualDataArray;
}

void UUnitMoveActionComponent::TakeAction(const FGrid& Grid)
{
	AGridManager* gridManager = AGridManager::GetGridManager();
	if (!IsValid(gridManager))
	{
		return;
	}

	int32 pathLength;
	TArray<FGrid> pathArray = gridManager->FindPath(Unit->GetGrid(), Grid, pathLength);

	if (pathLength > MaxActionRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("it's over MaxRange to Move here."));
		return;
	}

	if (pathLength == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't reach to the Target Grid. there is no Path"));
		return;
	}

	if (!pathArray.Contains(Grid))
	{
		UE_LOG(LogTemp, Warning, TEXT("You Can't Move to Invalid Grid"));
		return;
	}


	//~~ Move Debug.

	for (int i = 0; i < pathArray.Num(); i++)
	{
		DrawDebugSphere(GetWorld(), gridManager->GridToWorld(pathArray[i]), 10, 12, FColor::Blue, false, 1.5f, 0, 2.0f);
	}

	FGrid dest = pathArray.Last();

	if (!gridManager->IsValidGrid(dest))
	{
		return;
	}
	
	Destination = dest;
	Path = pathArray;

	if (OnActionStart.IsBound())
	{
		OnActionStart.Broadcast();
	}


}

void UUnitMoveActionComponent::DealWithGridBeforeAction(const FGrid& Grid)
{
	TakeAction(Grid);
}

void UUnitMoveActionComponent::OnActionStartFunc()
{
	Super::OnActionStartFunc();

	bMoveActivate = true;
}

void UUnitMoveActionComponent::OnActionEndFunc()
{
	Super::OnActionEndFunc();

	AGridManager* gridManager = AGridManager::GetGridManager();

	//Update Grid Data
	gridManager->MoveUnitGrid(Unit, Unit->GetGrid(), Destination);
	gridManager->RemoveAllGridVisual();

	bMoveActivate = false;


}

void UUnitMoveActionComponent::OnActionSelectedFunc()
{
	Super::OnActionSelectedFunc();
}
