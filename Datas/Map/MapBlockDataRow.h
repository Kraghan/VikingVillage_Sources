#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapBlockData.h"
#include "MapBlockDataRow.generated.h"

USTRUCT(BlueprintType)
struct VIKINGVILLAGE_API FMapBlockDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMapBlockData> _blockDatas;

};
