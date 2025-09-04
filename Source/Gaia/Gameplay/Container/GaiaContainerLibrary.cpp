#include "GaiaContainerLibrary.h"

#include "DataRegistrySubsystem.h"

bool UGaiaContainerLibrary::GetItemConfig(const FName InItemId, FGaiaItemConfig& OutItemConfig)
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

bool UGaiaContainerLibrary::GetContainerConfig(const FName InContainerName, FGaiaContainerConfig& OutContainerConfig)
{
	if (InContainerName != NAME_None)
	{
		if (UDataRegistrySubsystem * DataRegistrySubsystem = GEngine->GetEngineSubsystem<UDataRegistrySubsystem>())
		{
			OutContainerConfig = *DataRegistrySubsystem->GetCachedItem<FGaiaContainerConfig>(FDataRegistryId(TEXT("ContainerConfig"),InContainerName));
			return true;
		}
	}
	return false;
}
