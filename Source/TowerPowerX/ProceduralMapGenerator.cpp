// Fill out your copyright notice in the Description page of Project Settings.

#include "ProceduralMapGenerator.h"

AProceduralMapGenerator::AProceduralMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create terrain mesh
	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;

	// Create pathway mesh
	PathMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PathMesh"));
	PathMesh->SetupAttachment(RootComponent);
}

void AProceduralMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateTerrain();
	GeneratePaths();
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

	// -------------------------
	// Generate terrain vertices
	// -------------------------

	for (int32 Y = 0; Y <= GridSize; Y++)
	{
		for (int32 X = 0; X <= GridSize; X++)
		{
			const float NoiseX = X * NoiseScale;
			const float NoiseY = Y * NoiseScale;

			const float Height =
				FMath::PerlinNoise2D(
					FVector2D(NoiseX, NoiseY)
				) * HeightScale;

			const float WorldX =
				(X - GridSize / 2.0f) * CellSize;

			const float WorldY =
				(Y - GridSize / 2.0f) * CellSize;

			Vertices.Add(
				FVector(
					WorldX,
					WorldY,
					Height
				)
			);

			UV0.Add(
				FVector2D(
					static_cast<float>(X) / GridSize,
					static_cast<float>(Y) / GridSize
				)
			);
		}
	}

	// -------------------------
	// Generate terrain triangles
	// -------------------------

	for (int32 Y = 0; Y < GridSize; Y++)
	{
		for (int32 X = 0; X < GridSize; X++)
		{
			const int32 BottomLeft =
				Y * VertexCount + X;

			const int32 BottomRight =
				BottomLeft + 1;

			const int32 TopLeft =
				BottomLeft + VertexCount;

			const int32 TopRight =
				TopLeft + 1;

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

void AProceduralMapGenerator::GeneratePaths()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;

	const float HalfSize = (GridSize * CellSize) / 2.0f;

	// Put the paths well above the terrain for this test.
	const float PathZ = 300.0f;

	const FVector CentrePoint(0.0f, 0.0f, PathZ);

	// Path 1: Top → Centre
	AddPathSection(
		Vertices,
		Triangles,
		FVector(0.0f, HalfSize, PathZ),
		CentrePoint
	);

	// Path 2: Left → Centre
	AddPathSection(
		Vertices,
		Triangles,
		FVector(-HalfSize, 0.0f, PathZ),
		CentrePoint
	);

	// Path 3: Bottom → Centre
	AddPathSection(
		Vertices,
		Triangles,
		FVector(0.0f, -HalfSize, PathZ),
		CentrePoint
	);

	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	PathMesh->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UV0,
		VertexColors,
		Tangents,
		true
	);

	PathMesh->bUseAsyncCooking = true;
}

void AProceduralMapGenerator::AddPathSection(
	TArray<FVector>& Vertices,
	TArray<int32>& Triangles,
	const FVector& Start,
	const FVector& End
)
{
	const FVector Direction =
		(End - Start).GetSafeNormal2D();

	const FVector Perpendicular(
		-Direction.Y,
		Direction.X,
		0.0f
	);

	const FVector WidthOffset =
		Perpendicular * (PathWidth / 2.0f);

	const int32 StartIndex =
		Vertices.Num();

	Vertices.Add(Start + WidthOffset);
	Vertices.Add(Start - WidthOffset);
	Vertices.Add(End - WidthOffset);
	Vertices.Add(End + WidthOffset);

	Triangles.Add(StartIndex);
	Triangles.Add(StartIndex + 2);
	Triangles.Add(StartIndex + 1);

	Triangles.Add(StartIndex);
	Triangles.Add(StartIndex + 3);
	Triangles.Add(StartIndex + 2);
}

float AProceduralMapGenerator::GetTerrainHeight(
	float WorldX,
	float WorldY
) const
{
	const float TerrainOriginX =
		WorldX / CellSize
		+ GridSize / 2.0f;

	const float TerrainOriginY =
		WorldY / CellSize
		+ GridSize / 2.0f;

	const float NoiseX =
		TerrainOriginX * NoiseScale;

	const float NoiseY =
		TerrainOriginY * NoiseScale;

	return FMath::PerlinNoise2D(
		FVector2D(NoiseX, NoiseY)
	) * HeightScale;
}