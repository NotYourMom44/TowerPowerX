// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMapGenerator.h"

AProceduralMapGenerator::AProceduralMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;
}

void AProceduralMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateTerrain();
}

void AProceduralMapGenerator::GenerateTerrain()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	const int32 VertexCount = GridSize + 1;

	Vertices.Reserve(VertexCount * VertexCount);
	UV0.Reserve(VertexCount * VertexCount);

	// Generate vertices
	for (int32 Y = 0; Y <= GridSize; Y++)
	{
		for (int32 X = 0; X <= GridSize; X++)
		{
			const float NoiseX = X * NoiseScale;
			const float NoiseY = Y * NoiseScale;

			const float Height = FMath::PerlinNoise2D(FVector2D(NoiseX, NoiseY)) * HeightScale;

			const float WorldX = (X - GridSize / 2.0f) * CellSize;
			const float WorldY = (Y - GridSize / 2.0f) * CellSize;

			Vertices.Add(FVector(WorldX, WorldY, Height));

			UV0.Add(FVector2D(
				static_cast<float>(X) / GridSize,
				static_cast<float>(Y) / GridSize
			));
		}
	}

	// Generate triangles
	for (int32 Y = 0; Y < GridSize; Y++)
	{
		for (int32 X = 0; X < GridSize; X++)
		{
			const int32 BottomLeft = Y * VertexCount + X;
			const int32 BottomRight = BottomLeft + 1;
			const int32 TopLeft = BottomLeft + VertexCount;
			const int32 TopRight = TopLeft + 1;

			Triangles.Add(BottomLeft);
			Triangles.Add(TopLeft);
			Triangles.Add(TopRight);

			Triangles.Add(BottomLeft);
			Triangles.Add(TopRight);
			Triangles.Add(BottomRight);
		}
	}

	TerrainMesh->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UV0,
		VertexColors,
		Tangents,
		true
	);

	TerrainMesh->bUseAsyncCooking = true;
}