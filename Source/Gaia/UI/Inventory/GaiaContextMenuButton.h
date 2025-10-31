#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Gameplay/Inventory/GaiaInventoryTypes.h"
#include "GaiaContextMenuButton.generated.h"

class UImage;
class UTextBlock;

// 菜单按钮点击事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuButtonClicked, UGaiaContextMenuButton*, Button);

/**
 * 右键菜单项按钮
 * 显示单个菜单操作选项
 */
UCLASS()
class GAIAGAME_API UGaiaContextMenuButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	/**
	 * 设置菜单项数据
	 * @param MenuItem 菜单项配置
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI")
	void SetMenuItemData(const FItemContextMenuItem& MenuItem);

	/** 获取操作类型 */
	EItemContextAction GetAction() const { return Action; }

	/** 获取自定义事件名 */
	FName GetCustomEventName() const { return CustomEventName; }

	/** 菜单按钮点击事件（公开的） */
	UPROPERTY(BlueprintAssignable, Category = "Gaia|UI")
	FOnMenuButtonClicked OnMenuButtonClicked;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	/** 处理基础按钮点击（重写父类虚函数，转发到公开事件） */
	virtual void HandleButtonClicked() override;

protected:
	/** 操作类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI")
	EItemContextAction Action = EItemContextAction::Use;

	/** 自定义事件名 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI")
	FName CustomEventName;

	/** 图标（可选，UMG绑定） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UImage* Icon;

	/** 文本（UMG绑定） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Text;
};

