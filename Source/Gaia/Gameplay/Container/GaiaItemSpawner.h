#pragma once

#include "GaiaContainerTypes.h"
#include "GameFramework/Actor.h"
#include "GaiaItemSpawner.generated.h"

#define UE_API GAIAGAME_API

UCLASS(Abstract)
class AGaiaItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	AGaiaItemSpawner();
	
	UFUNCTION(Blueprintable, Category = "GaiaItemSpawner")
	void SpawnItem();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "要生成的物品")
	FDataTableRowHandle SpawnItemRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "数量")
	int32 SpawnNum = 1;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaItemBase|Components")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaItemBase|Components")
	TObjectPtr<UStaticMeshComponent> SceneMesh;
};
#undef UE_API