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
	
	// 更新所有权类型
	if (Text_OwnershipType)
	{
		FString OwnershipTypeStr;
		switch (DebugInfo.OwnershipType)
		{
		case EContainerOwnershipType::Private:
			OwnershipTypeStr = TEXT("私有");
			break;
		case EContainerOwnershipType::World:
			OwnershipTypeStr = TEXT("世界");
			break;
		case EContainerOwnershipType::Shared:
			OwnershipTypeStr = TEXT("共享");
			break;
		case EContainerOwnershipType::Trade:
			OwnershipTypeStr = TEXT("交易");
			break;
		default:
			OwnershipTypeStr = TEXT("未知");
			break;
		}
		
		Text_OwnershipType->SetText(FText::FromString(FString::Printf(
			TEXT("类型: %s"), *OwnershipTypeStr
		)));
	}
	
	// 更新所有者
	if (Text_Owner)
	{
		FString OwnerStr = DebugInfo.OwnerPlayerName.IsEmpty()
			? TEXT("无")
			: DebugInfo.OwnerPlayerName;
		
		Text_Owner->SetText(FText::FromString(FString::Printf(
			TEXT("所有者: %s"), *OwnerStr
		)));
	}
	
	// 更新授权玩家列表
	if (Text_AuthorizedPlayers)
	{
		FString AuthorizedPlayersStr;
		if (DebugInfo.AuthorizedPlayerNames.Num() == 0)
		{
			AuthorizedPlayersStr = TEXT("无");
		}
		else
		{
			for (int32 i = 0; i < DebugInfo.AuthorizedPlayerNames.Num(); ++i)
			{
				AuthorizedPlayersStr += DebugInfo.AuthorizedPlayerNames[i];
				if (i < DebugInfo.AuthorizedPlayerNames.Num() - 1)
				{
					AuthorizedPlayersStr += TEXT(", ");
				}
			}
		}
		
		Text_AuthorizedPlayers->SetText(FText::FromString(FString::Printf(
			TEXT("授权玩家: %s"), *AuthorizedPlayersStr
		)));
	}
	
	// 更新当前访问者
	if (Text_CurrentAccessor)
	{
		FString AccessorStr = DebugInfo.CurrentAccessorName.IsEmpty()
			? TEXT("无")
			: DebugInfo.CurrentAccessorName;
		
		Text_CurrentAccessor->SetText(FText::FromString(FString::Printf(
			TEXT("访问者: %s"), *AccessorStr
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
