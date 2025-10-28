// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameUIPolicy.h"
#include "GaiaUIPolicy.generated.h"

class UCommonLocalPlayer;
class UPrimaryGameLayout;

/**
 * Gaia游戏的UI策略
 * 
 * 职责：
 * - 管理每个玩家的PrimaryGameLayout
 * - 处理玩家添加/移除时的UI创建/销毁
 * - 支持多人分屏（如果需要）
 */
UCLASS(Blueprintable, Within = GameUIManagerSubsystem)
class GAIAGAME_API UGaiaUIPolicy : public UGameUIPolicy
{
	GENERATED_BODY()

public:
	UGaiaUIPolicy() = default;

	// 获取指定玩家的主布局
	template <typename GameLayoutClass = UPrimaryGameLayout>
	GameLayoutClass* GetRootLayoutAs(const UCommonLocalPlayer* LocalPlayer) const
	{
		return Cast<GameLayoutClass>(GetRootLayout(LocalPlayer));
	}

protected:
	// 当Layout添加到Viewport时调用
	virtual void OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout) override;
	
	// 当Layout从Viewport移除时调用
	virtual void OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout) override;
	
	// 创建Layout Widget（添加日志来诊断 PlayerController 为 null 的问题）
	virtual void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer) override;
};

