#pragma once

#include "GaiaContainerTypes.h"
#include "Components/ActorComponent.h"
#include "GaiaContainerComponent.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAIAGAME_API UGaiaContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGaiaContainerComponent();
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//UFUNCTION(BlueprintCallable)
	//bool AddItemToContainer(const FItem)
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Configs")
	FDataTableRowHandle ContainerConfigRowHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Container|RuntimeParameter")
	FGaiaContainerConfig ContainerConfig;
	
};
