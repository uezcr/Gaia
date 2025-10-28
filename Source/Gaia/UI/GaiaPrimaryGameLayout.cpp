// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaPrimaryGameLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameplayTagContainer.h"
#include "GaiaLogChannels.h"

UGaiaPrimaryGameLayout::UGaiaPrimaryGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGaiaPrimaryGameLayout::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 注册所有Layer到系统
	RegisterAllLayers();
	
	UE_LOG(LogGaia, Log, TEXT("[主布局] 初始化完成，已注册所有Layer"));
}

void UGaiaPrimaryGameLayout::RegisterAllLayers()
{
	// 注册游戏层
	if (GameLayer)
	{
		RegisterLayer(GetGameLayerTag(), GameLayer);
		UE_LOG(LogGaia, Verbose, TEXT("[主布局] 注册游戏层: %s"), *GetGameLayerTag().ToString());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[主布局] GameLayer未绑定！"));
	}

	// 注册容器层
	if (ContainerLayer)
	{
		RegisterLayer(GetContainerLayerTag(), ContainerLayer);
		UE_LOG(LogGaia, Verbose, TEXT("[主布局] 注册容器层: %s"), *GetContainerLayerTag().ToString());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[主布局] ContainerLayer未绑定！"));
	}

	// 注册菜单层
	if (MenuLayer)
	{
		RegisterLayer(GetMenuLayerTag(), MenuLayer);
		UE_LOG(LogGaia, Verbose, TEXT("[主布局] 注册菜单层: %s"), *GetMenuLayerTag().ToString());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[主布局] MenuLayer未绑定！"));
	}

	// 注册模态层
	if (ModalLayer)
	{
		RegisterLayer(GetModalLayerTag(), ModalLayer);
		UE_LOG(LogGaia, Verbose, TEXT("[主布局] 注册模态层: %s"), *GetModalLayerTag().ToString());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[主布局] ModalLayer未绑定！"));
	}
}

FGameplayTag UGaiaPrimaryGameLayout::GetGameLayerTag()
{
	return FGameplayTag::RequestGameplayTag(FName("UI.Layer.Game"));
}

FGameplayTag UGaiaPrimaryGameLayout::GetContainerLayerTag()
{
	return FGameplayTag::RequestGameplayTag(FName("UI.Layer.Container"));
}

FGameplayTag UGaiaPrimaryGameLayout::GetMenuLayerTag()
{
	return FGameplayTag::RequestGameplayTag(FName("UI.Layer.Menu"));
}

FGameplayTag UGaiaPrimaryGameLayout::GetModalLayerTag()
{
	return FGameplayTag::RequestGameplayTag(FName("UI.Layer.Modal"));
}

