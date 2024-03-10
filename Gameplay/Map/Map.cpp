#include "Map.h"
#include <VikingVillage/Datas/Map/MapBlockDataRow.h>
#include <Engine/StaticMeshActor.h>

void FGridCellData::Initialize(const FVector2D& position, const float& max, const float& min, AMap* map)
{
	_position = position;
	
	TMap<ERessourceType, int8> ressources;
	bool buildable = true;

	auto castForInfos = [&](const FVector& pos)
	{
		FVector end = pos;
		end.Z = min;
		FHitResult outHit;
		if (map->GetWorld()->LineTraceSingleByProfile(outHit, pos, end, "WorldStatic"))
		{
			if (UPhysicalMaterial* material = outHit.PhysMaterial.Get())
			{
				ERessourceType type = map->GetRessourceTypeByMaterial(material);
				if (ressources.Find(type))
				{
					ressources[type]++;
				}
				else
				{
					ressources.Add(type, 1);
				}
				if (map->IsMaterialBuildable(material) == false)
				{
					buildable = false;
				}
			}
		}
		else
		{
			buildable = false;
		}
	};

	castForInfos(FVector(_position.X, _position.Y, max));
	castForInfos(FVector(_position.X + map->_gridCellSize, _position.Y, max));
	castForInfos(FVector(_position.X, _position.Y + map->_gridCellSize, max));
	castForInfos(FVector(_position.X + map->_gridCellSize, _position.Y + map->_gridCellSize, max));

	ERessourceType bestRessource = ERessourceType::RT_Nothing;
	int8 quantity = 0;
	for (auto ressourceInfos : ressources)
	{
		if (ressourceInfos.Key != ERessourceType::RT_Nothing)
		{
			if (quantity < ressourceInfos.Value)
			{
				bestRessource = ressourceInfos.Key;
			}
		}
	}

	_isBuildable = buildable;
	_ressource = bestRessource;
}


void FGridCellData::DebugDraw(bool boundings, bool buildable, bool ressources, AMap* map) const
{
	float size = map->_gridCellSize / 2.f - 5.f;
	FVector center = FVector(_position.X + map->_gridCellSize / 2.f, _position.Y + map->_gridCellSize / 2.f, map->GetActorLocation().Z);
	FVector TL = center + FVector(-size, -size, 0);
	FVector BL = center + FVector(-size, size, 0);
	FVector TR = center + FVector(size, -size, 0);
	FVector BR = center + FVector(size, size, 0);

	bool persistent = true;
	float duration = -1;

	if (boundings)
	{
		DrawDebugLine(map->GetWorld(), TL, TR, FColor::White, persistent, duration, 255);
		DrawDebugLine(map->GetWorld(), TR, BR, FColor::White, persistent, duration, 255);
		DrawDebugLine(map->GetWorld(), BR, BL, FColor::White, persistent, duration, 255);
		DrawDebugLine(map->GetWorld(), BL, TL, FColor::White, persistent, duration, 255);
	}

	if (buildable)
	{
		if (_isBuildable == false)
		{
			DrawDebugLine(map->GetWorld(), BL, TR, FColor::Red, persistent, duration, 255);
			DrawDebugLine(map->GetWorld(), BR, TL, FColor::Red, persistent, duration, 255);
		}
	}

	if (ressources)
	{
		FColor color;
		switch (_ressource)
		{
		case ERessourceType::RT_Nothing:
			return;
		case ERessourceType::RT_Forest:
			color = FColor::Green;
			break;
		case ERessourceType::RT_Farming:
			color = FColor::Yellow;
			break;
		default:
			color = FColor(255, 0, 255, 255);
			break;
		}

		DrawDebugSphere(map->GetWorld(), center, map->_gridCellSize / 5.f, 12, color, persistent, duration, 255);
	}
}

// Sets default values
AMap::AMap()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ClearSpawnedMeshes();
	CreateMeshes();
}

// Called when the game starts or when spawned
void AMap::BeginPlay()
{
	Super::BeginPlay();

	CreateGrid();
	for (auto& cell : _gridData)
	{
		cell.DebugDraw(true, true, true, this);
	}
}

void AMap::ClearSpawnedMeshes()
{
	for (AActor* actor : _spawnedMeshes)
	{
		actor->Destroy();
	}

	_spawnedMeshes.Empty();
}

void AMap::CreateMeshes()
{
	FRandomStream random(FMath::Rand());
	if (_useSeed)
	{
		random = FRandomStream(_randomSeed);
	}

	for (const FMapBlockPosition& positionData : _blockConfigurations)
	{
		if (FMapBlockDataRow* row = positionData._meshes.GetRow<FMapBlockDataRow>("AMap::OnConstruction"))
		{
			if (row->_blockDatas.Num() == 0)
			{
				continue;
			}

			const FTransform& transformPoint = positionData._position->GetActorTransform();

			int32 meshIndex = positionData._forceMeshIndex;
			if (meshIndex < 0 || meshIndex >= row->_blockDatas.Num())
			{
				meshIndex = random.RandRange(0, row->_blockDatas.Num() - 1);
			}

			SpawnStaticMeshActor(positionData._position, row->_blockDatas[meshIndex]._transform, row->_blockDatas[meshIndex]._mesh);
		}
	}

	CreateGrid();
}

void AMap::SpawnStaticMeshActor(AActor* parent, const FTransform& transform, UStaticMesh* meshModel)
{
	AStaticMeshActor* mesh = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass());
	mesh->AttachToActor(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	mesh->SetActorRelativeTransform(transform);
	mesh->SetMobility(EComponentMobility::Static);
	UStaticMeshComponent* MeshComponent = mesh->GetStaticMeshComponent();
	if (MeshComponent)
	{
		MeshComponent->SetStaticMesh(meshModel);
	}

	_spawnedMeshes.Add(mesh);
}

void AMap::CreateGrid()
{
	FBox gridVolume = GetMapAABB();
	_numberOfCells = FVector2D(gridVolume.GetSize().X / _gridCellSize, gridVolume.GetSize().Y / _gridCellSize);

	_gridData.Empty();
	_gridData.Reserve(_numberOfCells.X * _numberOfCells.Y);
	_gridData.Init(FGridCellData(), _numberOfCells.X * _numberOfCells.Y);

	for (int32 x = 0; x < _numberOfCells.X; x++)
	{
		for (int32 y = 0; y < _numberOfCells.Y; y++)
		{
			_gridData[x * _numberOfCells.Y + y].Initialize(FVector2D(gridVolume.Min.X + x * _gridCellSize, gridVolume.Min.Y + y * _gridCellSize), gridVolume.Max.Z, gridVolume.Min.Z, this);
		}
	}
}

FBox AMap::GetMapAABB()
{
	FBox gridVolume = FBox();
	for (AActor* actor : _spawnedMeshes)
	{
		FVector center, bounds;
		actor->GetActorBounds(false, center, bounds);
		FBox meshAabb = FBox::BuildAABB(center, bounds);
		if (gridVolume.IsInside(meshAabb) == false)
		{
			if (gridVolume.Max.X < meshAabb.Max.X)
			{
				gridVolume.Max.X = meshAabb.Max.X;
			}

			if (gridVolume.Min.X > meshAabb.Min.X)
			{
				gridVolume.Min.X = meshAabb.Min.X;
			}

			if (gridVolume.Max.Y < meshAabb.Max.Y)
			{
				gridVolume.Max.Y = meshAabb.Max.Y;
			}

			if (gridVolume.Min.Y > meshAabb.Min.Y)
			{
				gridVolume.Min.Y = meshAabb.Min.Y;
			}

			if (gridVolume.Max.Z < meshAabb.Max.Z)
			{
				gridVolume.Max.Z = meshAabb.Max.Z;
			}

			if (gridVolume.Min.Z > meshAabb.Min.Z)
			{
				gridVolume.Min.Z = meshAabb.Min.Z;
			}
		}
	}

	FVector center = gridVolume.GetCenter();
	FVector extent = gridVolume.GetExtent();

	int32 boundingX = FMath::RoundToInt32(extent.X);
	gridVolume.Max.X = center.X + boundingX;
	gridVolume.Min.X = center.X - boundingX;
	int32 boundingY = FMath::RoundToInt32(extent.Y);
	gridVolume.Max.Y = center.Y + boundingY;
	gridVolume.Min.Y = center.Y - boundingY;
	int32 modX = boundingX % _gridCellSize;
	int32 modY = boundingY % _gridCellSize;
	if (modX != 0)
	{
		gridVolume.Max.X += modX;
		gridVolume.Min.X -= modX;
	}

	if (modY != 0)
	{
		gridVolume.Max.Y += modY;
		gridVolume.Min.Y -= modY;
	}

	return gridVolume;
}

ERessourceType AMap::GetRessourceTypeByMaterial(UPhysicalMaterial* material) const
{
	if (const TEnumAsByte<ERessourceType>* ressource = _materialPerRessource.Find(material))
	{
		return ressource->GetValue();
	}
	return ERessourceType::RT_Nothing;
}

bool AMap::IsMaterialBuildable(UPhysicalMaterial* material) const
{
	return _materialConstructible.Contains(material);
}