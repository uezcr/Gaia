// Fill out your copyright notice in the Description page of Project Settings.


#include "GaiaContainerComponent.h"

#include "GaiaLogChannels.h"


// Sets default values for this component's properties
UGaiaContainerComponent::UGaiaContainerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UGaiaContainerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (FGaiaContainerConfig* LoadConfig = ContainerConfigRowHandle.GetRow<FGaiaContainerConfig>(TEXT("Context")))
	{
		ContainerConfig = *LoadConfig;
	}
	else
	{
		UE_LOG(LogGaiaContainer, Error, TEXT("ContainerConfig not found !"));
	}
}


// Called when the game starts
void UGaiaContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGaiaContainerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// ...
}

