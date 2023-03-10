// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "Grid/GridObject.h"
#include "Grid/PathNode.h"
#include "Grid/InstancedGridVisualComponent.h"
#include "Grid/PathFindingSystem.h"
#include "Grid/GridSystem.h"

#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UnitCore/UnitCharacter.h"

// Sets default values
AGridManager::AGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bRunConstructionScriptOnDrag = false;

	X_Length = 10;
	Y_Length = 10;
	CellSize = 100.0f;

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillBoardComponent"));
	SetRootComponent(BillboardComponent);

	GridVisual_Move = CreateDefaultSubobject<UInstancedGridVisualComponent>(TEXT("GridVisual_Move"));
	GridVisual_Move->SetupAttachment(GetRootComponent());

	GridVisual_OK = CreateDefaultSubobject<UInstancedGridVisualComponent>(TEXT("GridVisual_OK"));
	GridVisual_OK->SetupAttachment(GetRootComponent());

	GridVisual_NO = CreateDefaultSubobject<UInstancedGridVisualComponent>(TEXT("GridVisual_NO"));
	GridVisual_NO->SetupAttachment(GetRootComponent());

	GridVisual_Warning = CreateDefaultSubobject<UInstancedGridVisualComponent>(TEXT("GridVisual_Warning"));
	GridVisual_Warning->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	CreateGridSystem();

}

//void AGridManager::OnConstruction(const FTransform& Transform)
//{
//	Super::OnConstruction(Transform);
//
//
//
//}

// Called every frame
void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AGridManager::IsValidGrid(const FGrid& Grid) const
{
	return Grid.X >= 0 && Grid.Y >= 0 && Grid.X < X_Length&& Grid.Y < Y_Length;
}

void AGridManager::CreateGridSystem()
{
	GridSystem = NewObject<UGridSystem>();

	GridSystem->SetGridSystem(
		X_Length, 
		Y_Length,
		[](UGridSystem* gs, FGrid grid) {
		UGridObject* gridObj = NewObject<UGridObject>();
		gridObj->SetGrid(grid);
		gridObj->SetGridSystem(gs);
		return gridObj;
		});

	PathFindingSystem = NewObject<UPathFindingSystem>();

	PathFindingSystem->SetPathFindingSystem(
		X_Length, 
		Y_Length,
		[](UPathFindingSystem* pfs, FGrid grid)
		{
			UPathNode* pathNode = NewObject<UPathNode>();
			pathNode->SetGrid(grid);
			return pathNode;
		}
	);
}

void AGridManager::RemoveAllGridVisual()
{
	GridVisual_Move->RemoveGridVisuals();
	GridVisual_OK->RemoveGridVisuals();
	GridVisual_NO->RemoveGridVisuals();
	GridVisual_Warning->RemoveGridVisuals();
}

FGrid AGridManager::WorldToGrid(const FVector& WorldPosition) const
{
	FGrid grid;
	grid.X = FMath::RoundToInt(WorldPosition.X / CellSize);
	grid.Y = FMath::RoundToInt(WorldPosition.Y / CellSize);

	return grid;
}

FVector AGridManager::GridToWorld(const FGrid& Grid) const
{
	FVector worldPosition = FVector::ZeroVector;
	worldPosition.X = Grid.X * CellSize;
	worldPosition.Y = Grid.Y * CellSize;

	return worldPosition;
}

UGridObject* AGridManager::GetValidGridObject(const FGrid& Grid) const
{
	UGridObject* gridObj = GridSystem->GetValidGridObject(Grid);

	if (!IsValid(gridObj))
	{
		return nullptr;
	}

	if (gridObj->GetGrid() == Grid)
	{
		return gridObj;
	}

	return nullptr;
}

void AGridManager::ShowGridRange(const FGrid& Grid, int32 Range, EGridVisualType GridVisualType)
{

	TArray<FGrid> gridList;

	for (int32 x = -Range; x <= Range; x++)
	{
		for (int32 y = -Range; y <= Range; y++)
		{
			FGrid temp = Grid + FGrid(x, y);

			if (!IsValidGrid(temp))
			{
				continue;
			}

			int32 distance = FMath::Abs(x) + FMath::Abs(y);
			if (distance > Range)
			{
				continue;
			}

			gridList.Add(temp);
		}
	}

	ShowFromGridArray(gridList, GridVisualType);
}

void AGridManager::ShowFromGridArray(const TArray<FGrid>& GridArray, EGridVisualType GridVisualType)
{
	UInstancedGridVisualComponent* toDraw = nullptr;

	switch (GridVisualType)
	{
	case EGridVisualType::Move:
		toDraw = GridVisual_Move;
		break;
	case EGridVisualType::NO:
		toDraw = GridVisual_NO;
		break;
	case EGridVisualType::OK:
		toDraw = GridVisual_OK;
		break;
	case EGridVisualType::Warning:
		toDraw = GridVisual_Warning;
		break;
	default:
		break;
	}

	if (IsValid(toDraw))
	{
		toDraw->DrawGridVisualswithGridArray(GridArray);
	}

}

void AGridManager::ShowFromGridVisualDataArray(const TArray<FGridVisualData>& GridVisualDataArray)
{
	for (const FGridVisualData& visualData : GridVisualDataArray)
	{
		UInstancedGridVisualComponent* toDraw = nullptr;

		switch (visualData.GridVisualType)
		{
		case EGridVisualType::Move:
			toDraw = GridVisual_Move;
			break;
		case EGridVisualType::NO:
			toDraw = GridVisual_NO;
			break;
		case EGridVisualType::OK:
			toDraw = GridVisual_OK;
			break;
		case EGridVisualType::Warning:
			toDraw = GridVisual_Warning;
			break;
		default:
			break;
		}

		if (IsValid(toDraw))
		{
			toDraw->DrawGridVisualsWithGridVisualData(visualData);
		}
	}


}

TArray<FGrid> AGridManager::FindPath(const FGrid& Start, const FGrid& End, int32& PathLength, bool bCanIgnoreUnit)
{
	//!????! return ???? ???? PathLength?? ????????????.

	//openList = ???? ?????? ???? / closeList = ???? ?????????? ?????? ????.
	TArray<UPathNode*> openList;
	TArray<UPathNode*> closeList;

	//Heap?? == PriorityQueue
	Algo::Heapify(openList, [](UPathNode* A, UPathNode* B)
		{
			return A->GetFCost() < B->GetFCost();
		});

	//???? ????
	UPathNode* startNode = PathFindingSystem->GetValidPathNode(Start);
	if (!IsValid(startNode))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartNode is Not Valid"));
		PathLength = -1;
		return TArray<FGrid>();
	}

	//???? ????
 	UPathNode* endNode = PathFindingSystem->GetValidPathNode(End);
	if (!IsValid(endNode))
	{
		UE_LOG(LogTemp, Warning, TEXT("EndNode is Not Valid"));
		PathLength = -1;
		return TArray<FGrid>();
	}

	//???? PathNode?? ?????? ?? ???? ??????.
	InitAllPathFindingNodes();

	startNode->SetGCost(0);
	startNode->SetHCost(CalculateGridDistance(Start, End));
	startNode->CalculateFCost();

	openList.Add(startNode);

	while (openList.Num() > 0)
	{
		UPathNode* currentNode = GetLowestFCostNode(openList);

		if (currentNode == endNode) //Break Point.
		{
			FGrid currentGrid = currentNode->GetGrid();
			AUnitCharacter* currentUnit = GetUnitAtGrid(currentGrid);
			AUnitCharacter* startUnit = GetUnitAtGrid(Start);
			
			//???????? ???????? ???????? ??????????, ???? Grid?? ?????? ?? ????????
			//?????? ?????? ?? ??????.
			//????, ???? ?????????? ??????.
			if (IsValid(currentUnit) && currentUnit != startUnit)
			{
				PathLength = -1;
				return TArray<FGrid>();
			}

			PathLength = endNode->GetFCost();
			return CalculatePath(endNode);
		}

		openList.Remove(currentNode);
		closeList.Add(currentNode);

		//UE_LOG(LogTemp, Warning, TEXT("Node Position : %d, %d"), currentNode->GetGrid().X, currentNode->GetGrid().Y);

		TArray<UPathNode*> nearNodeArray = GetNearNodeArray(currentNode);
		for (UPathNode* nearNode : nearNodeArray)
		{
			//close List ???? ???? ?????? ????.
			if (closeList.Contains(nearNode))
			{
				continue;
			}

			//???? ?? ???? ???? ????.
			if (nearNode->GetIsWalkable() == false)
			{
				closeList.Add(nearNode);
				continue;
			}

			//bCanIgnoreUnit?? true?? ??, ?? ?????? ?????? ?? ????.
			//GridVisual?? ?????? ?????? MoveActionComponent???? ValidGridVisual?? ??????.
			//???? ?????? GridSystem?? ?????? ??????.
			UGridObject* gridObj = GridSystem->GetValidGridObject(nearNode->GetGrid());
			if (!bCanIgnoreUnit && IsValid(gridObj) && gridObj->HasAnyUnit())
			{
				AUnitCharacter* currentUnit = gridObj->GetUnit();
				AUnitCharacter* startUnit = GetUnitAtGrid(Start);
				if (IsValid(currentUnit) && IsValid(startUnit))
				{
					if (currentUnit->ActorHasTag(startUnit->Tags[0]))
					{
						//???????? ???? ?????? ??????.
					}
					else
					{
						closeList.Add(nearNode);
						continue;
					}
				}
			}

			int tempGCost = currentNode->GetGCost() + CalculateGridDistance(currentNode->GetGrid(), nearNode->GetGrid());

			if (tempGCost < nearNode->GetGCost())
			{
				nearNode->SetParentNode(currentNode);
				nearNode->SetGCost(tempGCost);
				nearNode->SetHCost(CalculateGridDistance(nearNode->GetGrid(), End));
				nearNode->CalculateFCost();

				if (!openList.Contains(nearNode))
				{
					openList.Add(nearNode);
				}
			}
		}
	}


	//openList.Num() > 0?? ?????? ???? ?????? ?????? ?????? ?????????? ????.
	PathLength = -1;
	return TArray<FGrid>();
}

int32 AGridManager::CalculateGridDistance(const FGrid& a, const FGrid& b) const
{
	return FMath::Abs(a.X - b.X) + FMath::Abs(a.Y - b.Y);
}

UPathNode* AGridManager::GetLowestFCostNode(TArray<UPathNode*>& PathNodeList)
{
	if (PathNodeList.Num() == 0)
	{
		return nullptr;
	}

	Algo::HeapSort(PathNodeList, [](UPathNode* A, UPathNode* B)
		{
			return A->GetFCost() < B->GetFCost();
		});

	//HeapSort?? ???? ?????? ?????? ???? F ???? ???? Node??.
	UPathNode* lowestNode = PathNodeList[0];

	return lowestNode;
}

TArray<FGrid> AGridManager::CalculatePath(UPathNode* EndNode) const
{
	if (!IsValid(EndNode))
	{
		return TArray<FGrid>();
	}

	TArray<FGrid> gridArray;

	gridArray.Add(EndNode->GetGrid());
	UPathNode* current = EndNode;
	while (IsValid(current))
	{
		gridArray.Add(current->GetGrid());
		current = current->GetParentNode();
	}

	//?????? reverse???? ?????? ?????? ?????? ????.
	Algo::Reverse(gridArray);

	return gridArray;
}

TArray<UPathNode*> AGridManager::GetNearNodeArray(UPathNode* CurrentNode) const
{
	TArray<UPathNode*> nearNodeList;

	FGrid grid = CurrentNode->GetGrid();

	TArray<int32> dx{ 1,-1,0,0 };
	TArray<int32> dy{ 0,0,1,-1 };

	for (int32 i = 0; i<dx.Num();i++)
	{
		FGrid near = grid;
		near.X += dx[i];
		near.Y += dy[i];

		if (IsValidGrid(near))
		{
			UPathNode* nearNode = PathFindingSystem->GetValidPathNode(near);

			if (!IsValid(nearNode))
			{
				continue;
			}

			nearNodeList.Add(nearNode);
		}
	}

	return nearNodeList;
}

TArray<AUnitCharacter*> AGridManager::GetUnitArrayAtGrid(const FGrid& GridValue) const
{

	UGridObject* gridObject = GridSystem->GetValidGridObject(GridValue);
	if (!IsValid(gridObject))
	{
		return TArray<AUnitCharacter*>();
	}


	return gridObject->GetUnitArray();

}

AUnitCharacter* AGridManager::GetUnitAtGrid(const FGrid& GridValue) const
{
	TArray<AUnitCharacter*> gridArray = GetUnitArrayAtGrid(GridValue);

	if (gridArray.Num() == 0)
	{
		return nullptr;
	}

	return gridArray[0];
}

bool AGridManager::HasAnyUnitOnGrid(const FGrid& GridValue) const
{

	UGridObject* gridObj = GridSystem->GetValidGridObject(GridValue);
	if (IsValid(gridObj))
	{
		return gridObj->HasAnyUnit();
	}
	
	return false;
}

bool AGridManager::HasPath(const FGrid& Start, const FGrid& End, bool bCanIgnoreUnit)
{
	int32 pathLength = 0;
	FindPath(Start, End, pathLength, bCanIgnoreUnit);

	return pathLength != -1;
}

bool AGridManager::IsWalkableGrid(const FGrid& GridValue) const
{
	UPathNode* pathNode = PathFindingSystem->GetValidPathNode(GridValue);
	if (IsValid(pathNode))
	{
		return pathNode->GetIsWalkable();
	}

	return false;
}

int32 AGridManager::GetPathLength(const FGrid& Start, const FGrid& End)
{
	int32 pathLength = 0;
	FindPath(Start,End, pathLength);
	return pathLength;
}

void AGridManager::InitAllPathFindingNodes()
{
	//PathFindingGridSystem?? Grid ???? ???? ??????.

	TArray<UPathNode*> pathNodes = PathFindingSystem->GetPathNodeArray();

	for (UPathNode* pathNode : pathNodes)
	{
		if (!IsValid(pathNode))
		{
			//UE_LOG(LogTemp, Warning, TEXT("PathNode is Not Valid"));
			continue;
		}

		pathNode->SetGCost(TNumericLimits<int32>::Max());
		pathNode->SetHCost(0);
		pathNode->CalculateFCost();
		pathNode->SetParentNode(nullptr);
	}
}

AGridManager* AGridManager::GetGridManager()
{
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport))
	{
		return nullptr;
	}

	UWorld* world = GEngine->GameViewport->GetWorld();
	AGridManager* gridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(world, AGridManager::StaticClass()));

	return gridManager;
}

void AGridManager::AddUnitAtGrid(AUnitCharacter* Unit, const FGrid& GridValue)
{
	UGridObject* gridObject = GridSystem->GetValidGridObject(GridValue);
	if (IsValid(gridObject))
	{
		gridObject->AddUnit(Unit);
	}
}

void AGridManager::RemoveUnitAtGrid(AUnitCharacter* Unit, const FGrid& GridValue)
{
	UGridObject* gridObject = GridSystem->GetValidGridObject(GridValue);
	if (IsValid(gridObject))
	{
		gridObject->RemoveUnit(Unit);
	}
}

void AGridManager::MoveUnitGrid(AUnitCharacter* Unit, const FGrid& From, const FGrid& to)
{
	RemoveUnitAtGrid(Unit, From);
	AddUnitAtGrid(Unit, to);

	//It is right?
	Unit->SetGrid(to);

	if (OnAnyUnitMoved.IsBound())
	{
		OnAnyUnitMoved.Broadcast();
	}
}

TArray<UGridObject*> AGridManager::GetAllGridObjectThatHasUnit() const
{
	if (!IsValid(GridSystem))
	{
		return TArray<UGridObject*>();
	}

	return GridSystem->GetAllGridObjectThatHasUnit();
}

int32 AGridManager::CalculatePositionValue_ToMove(AUnitCharacter* Unit, const FGrid& Grid)
{
	//Unit : ?? ???? ????.
	//Grid : Test?? ????.

	if (Unit->Tags.Num() == 0)
	{
		//Test ??????.
		return 0.0f;
	}

	FName teamTag = Unit->Tags[0];
	int32 resultDistance = TNumericLimits<int32>::Max();

	TArray<UGridObject*> gridObjectArray = GetAllGridObjectThatHasUnit();
	
	//???? ?????? ????????, ?? ???????? ?????? ????.
	for (UGridObject* gridObj : gridObjectArray)
	{
		//???? Grid ?????? ???????? ??????.
		if (gridObj->GetGrid() == Grid)
		{
			continue;
		}

		auto CurrentUnit = gridObj->GetUnit();

		//?????? ??????, ???? ???? ?????? ????.
		if (!IsValid(CurrentUnit) || CurrentUnit->ActorHasTag(teamTag))
		{
			continue;
		}

		FGrid currentGrid = gridObj->GetGrid();

		int32 distance = CalculateGridDistance(Grid, currentGrid);
		if (resultDistance > distance)
		{
			resultDistance = distance;
		}
	}

	int32 reverseValueOffset = 10000;

	//?????? ?????? 1????, 10000-1 = 9999
	//?????? ?????? 5??, 10000-5 = 9995
	//??, ?????? ?????? ?????? ???? Value?? ????.
	//????, ?????????? ???? ???? ???? ???? ?????? 10000???? ???? ???? ?????? ???? ???? ??????
	//Grid SRPG ?????? ?????????? 40 ???????? ???? ?????? ?????? ?????? ???? ???????? ?????????? ?????? ???? ??????.
	return reverseValueOffset - resultDistance;
}
