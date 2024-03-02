#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "Map.generated.h"

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

private:
	UPROPERTY()
	TArray<AActor*> _spawnedMeshes;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ClearSpawnedMeshes();
	void SpawnStaticMeshActor(AActor* parent, const FTransform& transform, UStaticMesh* meshModel);
};
