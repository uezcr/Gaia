// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaUIPolicy.h"
#include "PrimaryGameLayout.h"
#include "CommonLocalPlayer.h"
#include "GaiaLogChannels.h"

void UGaiaUIPolicy::OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
	Super::OnRootLayoutAddedToViewport(LocalPlayer, Layout);
	
	UE_LOG(LogGaia, Log, TEXT("[UI Policy] ✅ 玩家布局已添加到Viewport: Player=%s, Layout=%s"), 
		*GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
}

void UGaiaUIPolicy::OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
	UE_LOG(LogGaia, Log, TEXT("[UI Policy] 玩家布局已从Viewport移除: Player=%s"), 
		*GetNameSafe(LocalPlayer));
	
	Super::OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
}

void UGaiaUIPolicy::CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer)
{
	UE_LOG(LogGaia, Log, TEXT("[UI Policy] CreateLayoutWidget 被调用: Player=%s"), *GetNameSafe(LocalPlayer));
	
	// 检查 PlayerController
	APlayerController* PC = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (!PC)
	{
		UE_LOG(LogGaia, Error, TEXT("[UI Policy] ❌ PlayerController为null！"));
		UE_LOG(LogGaia, Error, TEXT("    可能原因："));
		UE_LOG(LogGaia, Error, TEXT("    1. GameMode的PlayerControllerClass未设置"));
		UE_LOG(LogGaia, Error, TEXT("    2. 当前关卡没有GameMode"));
		UE_LOG(LogGaia, Error, TEXT("    解决方案：检查World Settings -> GameMode Override -> Player Controller Class"));
		// Epic的代码在这里也会直接返回，等待 OnPlayerControllerSet 回调触发
		return;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[UI Policy] ✅ PlayerController存在: %s，继续创建Layout"), *GetNameSafe(PC));
	
	// 调用父类创建 Layout
	Super::CreateLayoutWidget(LocalPlayer);
}

