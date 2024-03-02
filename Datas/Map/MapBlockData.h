#pragma once

#include "CoreMinimal.h"
#include "MapBlockData.generated.h"

USTRUCT(BlueprintType)
struct VIKINGVILLAGE_API FMapBlockData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* _mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform _transform;
};
