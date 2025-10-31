// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaContainerDebugInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "GaiaLogChannels.h"

void UGaiaContainerDebugInfoWidget::UpdateDebugInfo(const FContainerUIDebugInfo& DebugInfo)
{
	// 更新容器UID
	if (Text_ContainerUID)
	{
		Text_ContainerUID->SetText(FText::FromString(FString::Printf(
			TEXT("容器UID: %s | 定义: %s"),
			*DebugInfo.ContainerUID.ToString(),
			*DebugInfo.ContainerDefID.ToString()
		)));
	}
	
	// 更新槽位使用情况
	if (Text_SlotUsage)
	{
		Text_SlotUsage->SetText(FText::FromString(FString::Printf(
			TEXT("%s | %s | %s"),
			*DebugInfo.SlotUsage,
			*DebugInfo.WeightInfo,
			*DebugInfo.VolumeInfo
		)));
	}
	
	// 更新物品列表
	if (ScrollBox_ItemList)
	{
		// 清空现有物品
		ScrollBox_ItemList->ClearChildren();
		
		// 添加物品
		for (const FString& ItemInfo : DebugInfo.ItemList)
		{
			UTextBlock* ItemText = NewObject<UTextBlock>(ScrollBox_ItemList);
			if (ItemText)
			{
				ItemText->SetText(FText::FromString(ItemInfo));
				ItemText->SetAutoWrapText(true);
				
				// 设置字体大小
				FSlateFontInfo FontInfo = ItemText->GetFont();
				FontInfo.Size = 16;
				ItemText->SetFont(FontInfo);
				
				ScrollBox_ItemList->AddChild(ItemText);
			}
		}
	}
	
	UE_LOG(LogGaia, Verbose, TEXT("[调试信息] 刷新: Container=%s, Items=%d"),
		*DebugInfo.ContainerUID.ToString(), DebugInfo.ItemList.Num());
}
