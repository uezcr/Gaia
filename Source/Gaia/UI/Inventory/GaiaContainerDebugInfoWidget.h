#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Gameplay/Inventory/GaiaInventoryTypes.h"
#include "GaiaContainerDebugInfoWidget.generated.h"

class UTextBlock;
class UScrollBox;
class UBorder;

/**
 * 容器调试信息Widget
 * 
 * 职责：
 * - 显示容器的详细调试信息
 * - 仅在测试模式下显示
 * 
 * 显示内容：
 * - 容器UID
 * - 所有权类型
 * - 所有者信息
 * - 授权玩家列表
 * - 当前访问者
 * - 槽位使用情况
 * - 物品列表
 * 
 * 注意：作为子Widget，不需要完整的Activatable逻辑
 */
UCLASS()
class GAIAGAME_API UGaiaContainerDebugInfoWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * 更新调试信息
	 * @param DebugInfo 调试信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void UpdateDebugInfo(const FContainerUIDebugInfo& DebugInfo);

protected:
	// ========================================
	// Widget绑定（需要在UMG中绑定）
	// ========================================

	/** 容器UID文本 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ContainerUID;

	/** 所有权类型文本 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_OwnershipType;

	/** 所有者文本 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Owner;

	/** 授权玩家列表 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_AuthorizedPlayers;

	/** 当前访问者 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CurrentAccessor;

	/** 槽位使用情况 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SlotUsage;

	/** 物品列表 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox_ItemList;

	/** 整个调试面板 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_DebugPanel;
};

