#pragma once

#include "GaiaInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EGaiaInteractionType : uint8
{
	EPoint,
	EPickup,
	EEquip,
	EOpenContainer
};

USTRUCT(BlueprintType)
struct FGaiaInteractionContext
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "InteractionContext")
	AActor* Instigator = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = "InteractionContext")
	EGaiaInteractionType InteractionType = EGaiaInteractionType::EPoint;
};
