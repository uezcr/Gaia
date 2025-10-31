#include "GaiaContextMenuButton.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GaiaLogChannels.h"

void UGaiaContextMenuButton::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UGaiaContextMenuButton::NativeConstruct()
{
	Super::NativeConstruct();
	// HandleButtonClicked 是虚函数，会被父类自动调用，无需手动绑定
}

void UGaiaContextMenuButton::HandleButtonClicked()
{
	// 先调用父类实现（可能有重要逻辑）
	Super::HandleButtonClicked();
	
	// 然后广播我们的公开事件
	OnMenuButtonClicked.Broadcast(this);
}

void UGaiaContextMenuButton::SetMenuItemData(const FItemContextMenuItem& MenuItem)
{
	Action = MenuItem.Action;
	CustomEventName = MenuItem.CustomEventName;

	// 设置文本
	if (Text)
	{
		// 如果有自定义文本，使用自定义文本；否则使用默认文本
		FText DisplayText = MenuItem.DisplayText.IsEmpty() ? 
			FText::FromString(UEnum::GetDisplayValueAsText(Action).ToString()) : 
			MenuItem.DisplayText;
		
		Text->SetText(DisplayText);
	}

	// 设置图标（如果有）
	if (Icon && !MenuItem.Icon.IsNull())
	{
		Icon->SetVisibility(ESlateVisibility::Visible);
		// TODO: 加载并设置图标纹理
		// Icon->SetBrushFromSoftTexture(MenuItem.Icon);
	}
	else if (Icon)
	{
		Icon->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 设置启用状态
	SetIsEnabled(MenuItem.bEnabled);
}

