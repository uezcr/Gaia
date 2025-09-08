#include "GaiaContainerSlotWidget.h"

#include "GaiaItemSlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "Gameplay/Container/GaiaContainerLibrary.h"
#include "Gameplay/Container/GaiaContainerSubSystem.h"

void UGaiaContainerSlotWidget::SetContainerUID(const int64& InContainerUID)
{
	ContainerUID = InContainerUID;
}

void UGaiaContainerSlotWidget::CreateItemSlots() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr) return;
	UGaiaContainerSubSystem* ContainerSubSystem = GameInstance->GetSubsystem<UGaiaContainerSubSystem>();
	if (ContainerSubSystem == nullptr) return;
	FGaiaContainerInfo ContainerInfo;
	if (!ContainerSubSystem->GetContainerInfo(ContainerUID, ContainerInfo)) return;
	FGaiaContainerConfig ContainerConfig;
	if (!UGaiaContainerLibrary::GetContainerConfig(ContainerInfo.ContainerName, ContainerConfig)) return;
	for (const TPair<int32, int64>& SlotInfo : ContainerInfo.Items)
	{
		if (UGaiaItemSlotWidget* NewSlot = CreateWidget<UGaiaItemSlotWidget>(GetOwningPlayer(), ItemSlotClass))
		{
			GridPanel->AddChildToUniformGrid(NewSlot,SlotInfo.Key / ContainerConfig.GridNums.X,SlotInfo.Key % ContainerConfig.GridNums.Y);
			NewSlot->InitItemSlot(SlotInfo.Value);
		}
	}
}
