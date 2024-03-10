#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Map.generated.h"

UENUM(BlueprintType)
enum ERessourceType : uint8 
{
	RT_Nothing       UMETA(DisplayName = "Nothing"),
	RT_Forest        UMETA(DisplayName = "Forest"),
	RT_Farming       UMETA(DisplayName = "Farming")
};

USTRUCT(BlueprintType)
struct VIKINGVILLAGE_API FMapBlockPosition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ATargetPoint* _position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle _meshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _forceMeshIndex = -1;
};

USTRUCT(BlueprintType)
struct VIKINGVILLAGE_API FGridCellData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FVector2D _position;

	UPROPERTY(BlueprintReadWrite)
	bool _isBuildable;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<ERessourceType> _ressource;

	void Initialize(const FVector2D& pos, const float& max, const float& min, AMap* map); 
	
	void DebugDraw(bool boundings, bool buildable, bool ressources, AMap* map) const;
};

UCLASS()
class VIKINGVILLAGE_API AMap : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AMap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMapBlockPosition> _blockConfigurations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool _useSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "_useSeed", EditConditionHides))
	int32 _randomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _gridCellSize = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap< UPhysicalMaterial*, TEnumAsByte<ERessourceType>> _materialPerRessource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray< UPhysicalMaterial*> _materialConstructible;

private:
	UPROPERTY()
	TArray<AActor*> _spawnedMeshes;

	UPROPERTY()
	TArray<FGridCellData> _gridData;

	UPROPERTY()
	FVector2D _numberOfCells;

public:
	// Ressources
	ERessourceType GetRessourceTypeByMaterial(UPhysicalMaterial* material) const;
	bool IsMaterialBuildable(UPhysicalMaterial* material) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Map base
	void ClearSpawnedMeshes();
	void CreateMeshes();
	void SpawnStaticMeshActor(AActor* parent, const FTransform& transform, UStaticMesh* meshModel);
	
	// Grid
	void CreateGrid();
	FBox GetMapAABB();
};
