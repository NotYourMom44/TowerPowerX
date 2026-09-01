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

	// Generate a new random noise offset each time the game starts.
	NoiseOffsetX = FMath::FRandRange(0.0f, 10000.0f);
	NoiseOffsetY = FMath::FRandRange(0.0f, 10000.0f);

	GenerateTerrain();
	GeneratePaths();
	SpawnTower();
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
			const float NoiseX = X * NoiseScale + NoiseOffsetX;
			const float NoiseY = Y * NoiseScale + NoiseOffsetY;

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

	// Number of sections used to make the path follow the terrain.
	const int32 NumSections = 20;

	// Remember where this path's vertices start in the shared array.
	const int32 VertexStart = Vertices.Num();

	for (int32 Section = 0; Section <= NumSections; Section++)
	{
		const float Alpha =
			static_cast<float>(Section) / NumSections;

		const FVector Centre =
			FMath::Lerp(Start, End, Alpha);

		// Get the actual procedural terrain height at this position.
		const float TerrainHeight =
			GetTerrainHeight(Centre.X, Centre.Y);

		const float PathZ =
			TerrainHeight + PathHeightOffset;

		const FVector LeftPoint =
			FVector(
				Centre.X + WidthOffset.X,
				Centre.Y + WidthOffset.Y,
				PathZ
			);

		const FVector RightPoint =
			FVector(
				Centre.X - WidthOffset.X,
				Centre.Y - WidthOffset.Y,
				PathZ
			);

		Vertices.Add(LeftPoint);
		Vertices.Add(RightPoint);
	}

	// Connect each pair of path points with two triangles.
	for (int32 Section = 0; Section < NumSections; Section++)
	{
		const int32 CurrentLeft =
			VertexStart + Section * 2;

		const int32 CurrentRight =
			CurrentLeft + 1;

		const int32 NextLeft =
			CurrentLeft + 2;

		const int32 NextRight =
			CurrentLeft + 3;

		// First triangle
		Triangles.Add(CurrentLeft);
		Triangles.Add(NextLeft);
		Triangles.Add(CurrentRight);

		// Second triangle
		Triangles.Add(CurrentRight);
		Triangles.Add(NextLeft);
		Triangles.Add(NextRight);
	}
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
		TerrainOriginX * NoiseScale
		+ NoiseOffsetX;

	const float NoiseY =
		TerrainOriginY * NoiseScale
		+ NoiseOffsetY;

	return FMath::PerlinNoise2D(
		FVector2D(NoiseX, NoiseY)
	) * HeightScale;
}

void AProceduralMapGenerator::SpawnTower()
{
	if (!TowerClass)
	{
		return;
	}

	const float TowerX = 0.0f;
	const float TowerY = 0.0f;

	const float TerrainHeight =
		GetTerrainHeight(TowerX, TowerY);

	const FVector TowerLocation(
		TowerX,
		TowerY,
		TerrainHeight
	);

	GetWorld()->SpawnActor<AActor>(
		TowerClass,
		TowerLocation,
		FRotator::ZeroRotator
	);
}