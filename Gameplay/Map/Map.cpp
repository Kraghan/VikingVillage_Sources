#include "Map.h"
#include <VikingVillage/Datas/Map/MapBlockDataRow.h>
#include <Engine/StaticMeshActor.h>

// Sets default values
AMap::AMap()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AMap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FRandomStream random(FMath::Rand());
	if (_useSeed)
	{
		random = FRandomStream(_randomSeed);
	}

	ClearSpawnedMeshes();

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
}

// Called when the game starts or when spawned
void AMap::BeginPlay()
{
	Super::BeginPlay();
}

void AMap::ClearSpawnedMeshes()
{
	for (AActor* actor : _spawnedMeshes)
	{
		actor->Destroy();
	}

	_spawnedMeshes.Empty();
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