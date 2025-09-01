#include "GaiaItemBase.h"

#include "GaiaLogChannels.h"

AGaiaItemBase::AGaiaItemBase() : ItemInfo()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGaiaItemBase::InitializeItem()
{
	if (ItemInfo.ItemId == NAME_None)
	{
		UE_LOG(LogGaiaContainer, Warning, TEXT("%s : Item initialize failed, Invalid ItemId."), *GetName());
	}
	FGaiaItemConfig ItemConfig;
}

void AGaiaItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}


