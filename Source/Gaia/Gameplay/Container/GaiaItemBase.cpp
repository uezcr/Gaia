#include "GaiaItemBase.h"

#include "GaiaContainerLibrary.h"
#include "GaiaLogChannels.h"

AGaiaItemBase::AGaiaItemBase() : ItemInfo()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGaiaItemBase::NativePreInitializeItem(const FGaiaItemInfo& InItemInfo)
{
	if (InItemInfo.ItemId == NAME_None)
	{
		UE_LOG(LogGaiaContainer,Warning,TEXT("AGaiaItemBase::PreInitializeItem Failed, Invalid Id"));
		return;
	}
	FGaiaItemConfig DefaultConfig;
	if (!UGaiaContainerLibrary::GetItemConfigByItemId(InItemInfo.ItemId,DefaultConfig))
	{
		UE_LOG(LogGaiaContainer,Warning,TEXT("AGaiaItemBase::PreInitializeItem Failed, Invalid ItemConfig"));
		return;
	}
	ItemInfo.ItemId = InItemInfo.ItemId;
	ItemInfo.ItemQuantity = FMath::Clamp(InItemInfo.ItemQuantity,0,DefaultConfig.MaximumStack);
	PreInitializeItem();
	NativeInitializeItem(DefaultConfig);
}

void AGaiaItemBase::NativeInitializeItem(const FGaiaItemConfig& InItemConfig)
{
	InitializeItem(InItemConfig);
}

void AGaiaItemBase::NativePostInitializeItem()
{
	PostInitializeItem();
}


void AGaiaItemBase::BeginPlay()
{
	Super::BeginPlay();
}


