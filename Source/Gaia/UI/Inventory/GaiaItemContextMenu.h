#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Gameplay/Inventory/GaiaInventoryTypes.h"
#include "GaiaItemContextMenu.generated.h"

class UVerticalBox;
class UGaiaContextMenuButton;
class UGaiaInventoryRPCComponent;

/** 自定义菜单操作事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomMenuAction, FGuid, ItemUID, FName, ActionName);

/**
 * 物品右键菜单Widget
 * 显示物品的可用操作列表
 */
UCLASS()
class GAIAGAME_API UGaiaItemContextMenu : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * 初始化菜单
	 * @param InItemUID 物品UID
	 * @param ItemDef 物品定义
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI")
	void InitializeMenu(const FGuid& InItemUID, const FGaiaItemDefinition& ItemDef);

	/**
	 * 设置菜单位置（屏幕坐标）
	 * @param ScreenPosition 屏幕位置
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI")
	void SetMenuPosition(FVector2D ScreenPosition);

	/**
	 * 关闭菜单
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI")
	void CloseMenu();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	/**
	 * 构建菜单项
	 * @param ItemDef 物品定义
	 */
	void BuildMenuItems(const FGaiaItemDefinition& ItemDef);

	/**
	 * 获取预定义菜单项
	 * @param MenuType 菜单类型
	 * @return 菜单项列表
	 */
	TArray<FItemContextMenuItem> GetPredefinedMenuItems(EItemContextMenuType MenuType) const;

	/**
	 * 处理菜单项点击
	 * @param Button 被点击的按钮
	 */
	UFUNCTION()
	void OnMenuItemClicked(UGaiaContextMenuButton* Button);

	/**
	 * 执行菜单操作
	 * @param Action 操作类型
	 * @param CustomEventName 自定义事件名
	 */
	void ExecuteMenuAction(EItemContextAction Action, FName CustomEventName);

	/**
	 * 获取RPC组件
	 */
	UGaiaInventoryRPCComponent* GetRPCComponent() const;

protected:
	/** 当前物品UID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI")
	FGuid CurrentItemUID;

	/** 菜单项容器（UMG绑定） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* MenuItemsContainer;

	/** 菜单项按钮类 */
	UPROPERTY(EditDefaultsOnly, Category = "Gaia|UI")
	TSubclassOf<UGaiaContextMenuButton> MenuItemButtonClass;

	/** 自定义操作事件 */
	UPROPERTY(BlueprintAssignable, Category = "Gaia|UI")
	FOnCustomMenuAction OnCustomAction;
};

