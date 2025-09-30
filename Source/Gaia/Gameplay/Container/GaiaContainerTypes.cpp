#include "GaiaContainerTypes.h"

int32 FGaiaContainerInfo::GetItemIndexBySlotID(const int32 InSlotID)
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].ItemAddress.SlotID == InSlotID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}
