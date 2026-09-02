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

	UFUNCTION(BlueprintCallable, Category = "Paths")
	FVector GetPathSpawnPoint(int32 PathIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Terrain")
	float GetTerrainHeight(float WorldX, float WorldY) const;

protected:
	virtual void BeginPlay() override;

private:

	// Main procedural terrain mesh
	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	UProceduralMeshComponent* TerrainMesh;

	// Procedural pathway mesh
	UPROPERTY(VisibleAnywhere, Category = "Paths")
	UProceduralMeshComponent* PathMesh;

	// -------------------------
	// Terrain settings
	// -------------------------

	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 GridSize = 30;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float HeightScale = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float NoiseScale = 0.08f;

	float NoiseOffsetX;
	float NoiseOffsetY;

	// -------------------------
	// Path settings
	// -------------------------

	UPROPERTY(EditAnywhere, Category = "Paths")
	float PathWidth = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Paths")
	float PathHeightOffset = 25.0f;

	// -------------------------
	// Tower settings
	// -------------------------

	UPROPERTY(EditAnywhere, Category = "Tower")
	TSubclassOf<AActor> TowerClass;

	// Generates the procedural terrain
	void GenerateTerrain();

	// Generates the three procedural pathways
	void GeneratePaths();

	// Spawns the central tower
	void SpawnTower();

	// Adds a rectangular path section between two points
	void AddPathSection(
		TArray<FVector>& Vertices,
		TArray<int32>& Triangles,
		const FVector& Start,
		const FVector& End
	);
};