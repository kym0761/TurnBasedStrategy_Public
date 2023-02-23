// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grid.generated.h"

/**
 * Grid ����
 */

USTRUCT(BlueprintType, Blueprintable)
struct FGrid
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Y;

	FGrid();
	FGrid(int32 _X, int32 _Y);
	FString ToString() const;
	bool operator==(const FGrid& Other) const;
	bool operator!= (const FGrid& Other) const;
	FGrid operator+(const FGrid& b) const;
	FGrid operator-(const FGrid& b) const;
	FGrid& operator=(const FGrid& b);
	FGrid& operator+=(const FGrid& b);
	FGrid& operator-=(const FGrid& b);
};



UENUM(BlueprintType)
enum class EGridVisualType : uint8
{
	Move UMETA(DisplayName = "Move"), //�̵�?
	OK UMETA(DisplayName = "OK"), //�Ʊ�����?
	NO UMETA(DisplayName = "NO"), // ������?
	NO_Soft UMETA(DisplayName = "No_Soft"), //���� ������ ĭ
	Warning UMETA(DisplayName = "Warning") // ���� ������ ����?

};


USTRUCT(BlueprintType)
struct FGridVisualData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
		FGrid Grid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
		EGridVisualType GridVisualType;

	static TArray<FGrid> ParseToGridArray(TArray<FGridVisualData> GridVisualArray);
};