#include "GaiaItemBase.h"

#include "GaiaContainerComponent.h"
#include "GaiaContainerLibrary.h"
#include "GaiaLogChannels.h"

AGaiaItemBase::AGaiaItemBase() : ItemInfo()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGaiaItemBase::NativePreInitializeItem(const FGaiaItemInfo& InItemInfo)
{
	if (InItemInfo.ItemName == NAME_None)
	{
		UE_LOG(LogGaiaContainer,Warning,TEXT("AGaiaItemBase::PreInitializeItem Failed, Invalid Id"));
		return;
	}
	FGaiaItemConfig DefaultConfig;
	if (!UGaiaContainerLibrary::GetItemConfig(InItemInfo.ItemName,DefaultConfig))
	{
		UE_LOG(LogGaiaContainer,Warning,TEXT("AGaiaItemBase::PreInitializeItem Failed, Invalid ItemConfig"));
		return;
	}
	ItemInfo = InItemInfo;
	PreInitializeItem();
	NativeInitializeItem(DefaultConfig);
}

void AGaiaItemBase::NativeInitializeItem(const FGaiaItemConfig& InItemConfig)
{
	InitializeItem(InItemConfig);
}

void AGaiaItemBase::NativePostInitializeItem(const FGaiaItemConfig& InItemConfig)
{
	if (InItemConfig.bHasContainer)
	{
		if (FGaiaContainerConfig* LoadConfig = InItemConfig.ContainerRow.GetRow<FGaiaContainerConfig>(TEXT("Context")))
		{
			FGaiaContainerConfig ContainerConfig = *LoadConfig;
		}
	}
	PostInitializeItem(InItemConfig);
	bCanSelected = true;
}

void AGaiaItemBase::Selected_Implementation()
{
	bCanInteracted = true;
}

void AGaiaItemBase::EndSelected_Implementation()
{
	bCanInteracted = false;
}

void AGaiaItemBase::Interacted_Implementation(FGaiaInteractionContext& Context)
{
	if (Context.InteractionType == EGaiaInteractionType::EPoint)
	{
		
	}
}

void AGaiaItemBase::BeginPlay()
{
	Super::BeginPlay();
}


