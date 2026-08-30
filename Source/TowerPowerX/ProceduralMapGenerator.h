// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMapGenerator.generated.h"

UCLASS()
class TOWERPOWERX_API AProceduralMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AProceduralMapGenerator();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* TerrainMesh;

	// Terrain settings
	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 GridSize = 30;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float HeightScale = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float NoiseScale = 0.08f;

	// Generates the terrain mesh
	void GenerateTerrain();
};