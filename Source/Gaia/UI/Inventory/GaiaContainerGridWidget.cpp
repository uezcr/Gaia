// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaContainerGridWidget.h"
#include "GaiaItemSlotWidget.h"
#include "Gameplay/Inventory/GaiaInventorySubsystem.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "GaiaLogChannels.h"

void UGaiaContainerGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGaiaContainerGridWidget::InitializeContainer(const FGuid& InContainerUID)
{
	if (!InContainerUID.IsValid())
	{
		UE_LOG(LogGaia, Error, TEXT("[容器网格] 无效的容器UID"));
		return;
	}
	
	ContainerUID = InContainerUID;
	
	// 清空旧槽位
	DestroySlotWidgets();
	
	// 创建新槽位
	CreateSlotWidgets();
	
	UE_LOG(LogGaia, Log, TEXT("[容器网格] 初始化完成: Container=%s, Slots=%d"),
		*ContainerUID.ToString(), SlotWidgets.Num());
}

void UGaiaContainerGridWidget::RefreshAllSlots()
{
	UE_LOG(LogGaia, Warning, TEXT("[容器网格] ⭐ RefreshAllSlots 被调用: Container=%s, SlotCount=%d"),
		*ContainerUID.ToString(), SlotWidgets.Num());
	
	int32 RefreshedCount = 0;
	for (UGaiaItemSlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->RefreshSlot();
			RefreshedCount++;
		}
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[容器网格] ✅ 刷新完成: 刷新了 %d 个槽位"), RefreshedCount);
}

void UGaiaContainerGridWidget::RefreshSlot(int32 SlotID)
{
	UGaiaItemSlotWidget* SlotWidget = GetSlotWidget(SlotID);
	if (SlotWidget)
	{
		SlotWidget->RefreshSlot();
		UE_LOG(LogGaia, Verbose, TEXT("[容器网格] 刷新槽位: Slot=%d"), SlotID);
	}
	else
	{
		UE_LOG(LogGaia, Warning, TEXT("[容器网格] 槽位不存在: SlotID=%d"), SlotID);
	}
}

void UGaiaContainerGridWidget::ClearAllSlots()
{
	DestroySlotWidgets();
	UE_LOG(LogGaia, Log, TEXT("[容器网格] 清空所有槽位"));
}

UGaiaItemSlotWidget* UGaiaContainerGridWidget::GetSlotWidget(int32 SlotID) const
{
	// 槽位ID从0开始
	if (SlotID >= 0 && SlotID < SlotWidgets.Num())
	{
		return SlotWidgets[SlotID];
	}
	return nullptr;
}

void UGaiaContainerGridWidget::CreateSlotWidgets()
{
	UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(GetWorld());
	if (!InvSys || !ContainerUID.IsValid())
	{
		return;
	}
	
	// 获取容器信息
	FGaiaContainerInstance Container;
	if (!InvSys->FindContainerByUID(ContainerUID, Container))
	{
		UE_LOG(LogGaia, Error, TEXT("[容器网格] 容器不存在: %s"), *ContainerUID.ToString());
		return;
	}
	
	// 获取容器定义（获取SlotCount）
	FGaiaContainerDefinition ContainerDef;
	if (!UGaiaInventorySubsystem::GetContainerDefinition(Container.ContainerDefinitionID, ContainerDef))
	{
		UE_LOG(LogGaia, Error, TEXT("[容器网格] 容器定义不存在: %s"), *Container.ContainerDefinitionID.ToString());
		return;
	}
	
	int32 MaxSlots = ContainerDef.SlotCount;
	
	// 检查Widget类
	if (!ItemSlotWidgetClass)
	{
		UE_LOG(LogGaia, Error, TEXT("[容器网格] ItemSlotWidgetClass未设置"));
		return;
	}
	
	// 检查布局容器
	if (!GridPanel && !WrapBox)
	{
		UE_LOG(LogGaia, Error, TEXT("[容器网格] 没有可用的布局容器（GridPanel或WrapBox）"));
		return;
	}
	
	// 创建槽位
	SlotWidgets.Reserve(MaxSlots);
	
	for (int32 SlotID = 0; SlotID < MaxSlots; ++SlotID)
	{
		// 创建槽位Widget
		UGaiaItemSlotWidget* SlotWidget = CreateWidget<UGaiaItemSlotWidget>(this, ItemSlotWidgetClass);
		if (!SlotWidget)
		{
			UE_LOG(LogGaia, Error, TEXT("[容器网格] 无法创建槽位Widget: SlotID=%d"), SlotID);
			continue;
		}
		
		// 先添加到布局容器（触发 NativeConstruct）
		if (GridPanel)
		{
			// 使用网格布局
			int32 Row = SlotID / GridColumns;
			int32 Column = SlotID % GridColumns;
			
			UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(SlotWidget, Row, Column);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);
			}
		}
		else if (WrapBox)
		{
			// 使用自动换行布局
			UWrapBoxSlot* WrapSlot = WrapBox->AddChildToWrapBox(SlotWidget);
			if (WrapSlot)
			{
				WrapSlot->SetPadding(FMargin(SlotPadding.X, SlotPadding.Y));
				WrapSlot->SetHorizontalAlignment(HAlign_Left);
				WrapSlot->SetVerticalAlignment(VAlign_Top);
			}
		}
		
		// 添加到数组
		SlotWidgets.Add(SlotWidget);
		
		// 在添加到布局后再初始化槽位（避免 NativeConstruct 中的 SetEmpty 覆盖数据）
		SlotWidget->InitializeSlot(ContainerUID, SlotID);
	}
	
	UE_LOG(LogGaia, Log, TEXT("[容器网格] 创建槽位: 数量=%d"), SlotWidgets.Num());
}

void UGaiaContainerGridWidget::DestroySlotWidgets()
{
	// 从布局容器移除
	if (GridPanel)
	{
		GridPanel->ClearChildren();
	}
	if (WrapBox)
	{
		WrapBox->ClearChildren();
	}
	
	// 清空数组（Widget会自动销毁）
	SlotWidgets.Empty();
	
	UE_LOG(LogGaia, Verbose, TEXT("[容器网格] 销毁所有槽位"));
}
