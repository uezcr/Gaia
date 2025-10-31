#include "GaiaItemContextMenu.h"
#include "Components/VerticalBox.h"
#include "GaiaContextMenuButton.h"
#include "Gameplay/Inventory/GaiaInventoryRPCComponent.h"
#include "UI/GaiaUIManagerSubsystem.h"
#include "GaiaLogChannels.h"
#include "GameFramework/PlayerController.h"

void UGaiaItemContextMenu::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGaiaItemContextMenu::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 设置输入模式
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UGaiaItemContextMenu::NativeOnDeactivated()
{
	// 恢复输入模式
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	Super::NativeOnDeactivated();
}

FReply UGaiaItemContextMenu::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void UGaiaItemContextMenu::InitializeMenu(const FGuid& InItemUID, const FGaiaItemDefinition& ItemDef)
{
	CurrentItemUID = InItemUID;
	BuildMenuItems(ItemDef);
}

void UGaiaItemContextMenu::SetMenuPosition(FVector2D ScreenPosition)
{
	// TODO: 实现智能定位（避免超出屏幕边界）
	// 暂时由UMG层处理定位
}

void UGaiaItemContextMenu::CloseMenu()
{
	DeactivateWidget();
}

void UGaiaItemContextMenu::BuildMenuItems(const FGaiaItemDefinition& ItemDef)
{
	if (!MenuItemsContainer)
	{
		UE_LOG(LogGaia, Error, TEXT("MenuItemsContainer not bound in GaiaItemContextMenu"));
		return;
	}

	if (!MenuItemButtonClass)
	{
		UE_LOG(LogGaia, Error, TEXT("MenuItemButtonClass not set in GaiaItemContextMenu"));
		return;
	}

	// 清空现有菜单项
	MenuItemsContainer->ClearChildren();

	// 获取菜单项配置
	TArray<FItemContextMenuItem> MenuItems;
	if (ItemDef.ContextMenuType == EItemContextMenuType::Custom)
	{
		MenuItems = ItemDef.CustomMenuItems;
	}
	else
	{
		MenuItems = GetPredefinedMenuItems(ItemDef.ContextMenuType);
	}

	// 创建菜单项按钮
	for (const FItemContextMenuItem& MenuItem : MenuItems)
	{
		if (!MenuItem.bEnabled)
		{
			continue;
		}

		UGaiaContextMenuButton* Button = CreateWidget<UGaiaContextMenuButton>(
			GetOwningPlayer(), MenuItemButtonClass);

		if (Button)
		{
			Button->SetMenuItemData(MenuItem);
			// 绑定到我们的公开事件
			Button->OnMenuButtonClicked.AddDynamic(this, &UGaiaItemContextMenu::OnMenuItemClicked);
			MenuItemsContainer->AddChild(Button);
		}
	}
}

TArray<FItemContextMenuItem> UGaiaItemContextMenu::GetPredefinedMenuItems(EItemContextMenuType MenuType) const
{
	TArray<FItemContextMenuItem> Items;

	switch (MenuType)
	{
	case EItemContextMenuType::Consumable:
		Items.Add({ EItemContextAction::Use, FText::FromString(TEXT("使用")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Drop, FText::FromString(TEXT("丢弃")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Destroy, FText::FromString(TEXT("销毁")), nullptr, true, NAME_None });
		break;

	case EItemContextMenuType::Equipment:
		Items.Add({ EItemContextAction::Equip, FText::FromString(TEXT("装备")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Drop, FText::FromString(TEXT("丢弃")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Destroy, FText::FromString(TEXT("销毁")), nullptr, true, NAME_None });
		break;

	case EItemContextMenuType::Container:
		Items.Add({ EItemContextAction::OpenContainer, FText::FromString(TEXT("打开")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::EmptyContainer, FText::FromString(TEXT("清空")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Drop, FText::FromString(TEXT("丢弃")), nullptr, true, NAME_None });
		break;

	case EItemContextMenuType::Material:
		Items.Add({ EItemContextAction::Split, FText::FromString(TEXT("拆分")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Drop, FText::FromString(TEXT("丢弃")), nullptr, true, NAME_None });
		Items.Add({ EItemContextAction::Destroy, FText::FromString(TEXT("销毁")), nullptr, true, NAME_None });
		break;

	case EItemContextMenuType::QuestItem:
		Items.Add({ EItemContextAction::Inspect, FText::FromString(TEXT("查看详情")), nullptr, true, NAME_None });
		break;

	case EItemContextMenuType::None:
	default:
		break;
	}

	return Items;
}

void UGaiaItemContextMenu::OnMenuItemClicked(UGaiaContextMenuButton* Button)
{
	if (!Button)
	{
		return;
	}

	ExecuteMenuAction(Button->GetAction(), Button->GetCustomEventName());
	CloseMenu();
}

void UGaiaItemContextMenu::ExecuteMenuAction(EItemContextAction Action, FName CustomEventName)
{
	UGaiaInventoryRPCComponent* RPCComp = GetRPCComponent();
	
	switch (Action)
	{
	case EItemContextAction::Use:
		UE_LOG(LogGaia, Log, TEXT("Use item: %s"), *CurrentItemUID.ToString());
		// TODO: 实现使用物品逻辑
		break;

	case EItemContextAction::Equip:
		UE_LOG(LogGaia, Log, TEXT("Equip item: %s"), *CurrentItemUID.ToString());
		// TODO: 实现装备物品逻辑
		break;

	case EItemContextAction::Unequip:
		UE_LOG(LogGaia, Log, TEXT("Unequip item: %s"), *CurrentItemUID.ToString());
		// TODO: 实现卸下装备逻辑
		break;

	case EItemContextAction::OpenContainer:
		UE_LOG(LogGaia, Log, TEXT("Open container: %s"), *CurrentItemUID.ToString());
		if (UGaiaUIManagerSubsystem* UIManager = UGaiaUIManagerSubsystem::Get(this))
		{
			UIManager->OpenContainerByItemUID(CurrentItemUID);
		}
		break;

	case EItemContextAction::EmptyContainer:
		UE_LOG(LogGaia, Log, TEXT("Empty container: %s"), *CurrentItemUID.ToString());
		// TODO: 实现清空容器逻辑
		break;

	case EItemContextAction::Split:
		UE_LOG(LogGaia, Log, TEXT("Split item: %s"), *CurrentItemUID.ToString());
		// TODO: 显示拆分对话框
		break;

	case EItemContextAction::Drop:
		UE_LOG(LogGaia, Log, TEXT("Drop item: %s"), *CurrentItemUID.ToString());
		// TODO: 实现丢弃物品逻辑
		if (RPCComp)
		{
			// RPCComp->RequestDropItem(CurrentItemUID);
		}
		break;

	case EItemContextAction::Destroy:
		UE_LOG(LogGaia, Log, TEXT("Destroy item: %s"), *CurrentItemUID.ToString());
		// TODO: 显示确认对话框
		break;

	case EItemContextAction::Inspect:
		UE_LOG(LogGaia, Log, TEXT("Inspect item: %s"), *CurrentItemUID.ToString());
		// TODO: 显示物品详情
		break;

	case EItemContextAction::Custom:
		UE_LOG(LogGaia, Log, TEXT("Custom action: %s for item: %s"), *CustomEventName.ToString(), *CurrentItemUID.ToString());
		OnCustomAction.Broadcast(CurrentItemUID, CustomEventName);
		break;

	default:
		break;
	}
}

UGaiaInventoryRPCComponent* UGaiaItemContextMenu::GetRPCComponent() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		return PC->FindComponentByClass<UGaiaInventoryRPCComponent>();
	}
	return nullptr;
}

