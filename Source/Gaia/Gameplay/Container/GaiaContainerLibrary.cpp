#include "GaiaContainerLibrary.h"

#include "DataRegistrySubsystem.h"

bool UGaiaContainerLibrary::GetItemConfigByItemId(const FName InItemId, FGaiaItemConfig& OutItemConfig)
{
	if (InItemId != NAME_None)
	{
		if (UDataRegistrySubsystem * DataRegistrySubsystem = GEngine->GetEngineSubsystem<UDataRegistrySubsystem>())
		{
			OutItemConfig = *DataRegistrySubsystem->GetCachedItem<FGaiaItemConfig>(FDataRegistryId(TEXT("ItemConfig"),InItemId));
			return true;
		}
	}
	return false;
}
