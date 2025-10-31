// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaItemSlotWidget.h"
#include "GaiaItemDragDropOperation.h"
#include "GaiaItemContextMenu.h"
#include "Gameplay/Inventory/GaiaInventorySubsystem.h"
#include "Gameplay/Inventory/GaiaInventoryRPCComponent.h"
#include "UI/GaiaUIManagerSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "GaiaLogChannels.h"
#include "GameplayTagContainer.h"

void UGaiaItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 检查组件绑定（调试用）
	UE_LOG(LogGaia, Warning, TEXT("=== [物品槽位] 组件绑定检查 ==="));
	UE_LOG(LogGaia, Warning, TEXT("  Image_Icon: %s"), Image_Icon ? TEXT("✅ 已绑定") : TEXT("❌ nullptr - 检查UMG中的Image_Icon组件"));
	UE_LOG(LogGaia, Warning, TEXT("  Text_Quantity: %s"), Text_Quantity ? TEXT("✅ 已绑定") : TEXT("❌ nullptr - 检查UMG中的Text_Quantity组件"));
	UE_LOG(LogGaia, Warning, TEXT("  Border_Background: %s"), Border_Background ? TEXT("✅ 已绑定") : TEXT("❌ nullptr - 检查UMG中的Border_Background组件"));
	UE_LOG(LogGaia, Warning, TEXT("  SizeBox_Slot: %s"), SizeBox_Slot ? TEXT("✅ 已绑定") : TEXT("❌ nullptr (可选)"));
	
	// 检查Image_Icon的初始状态
	if (Image_Icon)
	{
		UE_LOG(LogGaia, Warning, TEXT("  Image_Icon 初始 Visibility: %d"), (int32)Image_Icon->GetVisibility());
		UE_LOG(LogGaia, Warning, TEXT("  Image_Icon 初始 RenderOpacity: %f"), Image_Icon->GetRenderOpacity());
		UE_LOG(LogGaia, Warning, TEXT("  Image_Icon 初始 Brush Size: %s"), *Image_Icon->GetBrush().GetImageSize().ToString());
	}
	
	// 设置槽位大小
	if (SizeBox_Slot)
	{
		SizeBox_Slot->SetWidthOverride(SlotSize);
		SizeBox_Slot->SetHeightOverride(SlotSize);
	}
	
	// 初始化为空槽位
	SetEmpty();
}

void UGaiaItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	
	if (!bIsSelected)
	{
		SetHighlighted(true);
	}
	
	// TODO: 显示Tooltip
}

void UGaiaItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	if (!bIsSelected)
	{
		SetHighlighted(false);
	}
	
	// TODO: 隐藏Tooltip
}

FReply UGaiaItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] MouseButtonDown: SlotID=%d, IsEmpty=%d, LeftButton=%d, RightButton=%d"),
		SlotID, IsEmpty(), InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton), InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton));
	
	// 右键点击 - 显示上下文菜单
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (!IsEmpty())  // 只有非空槽位才显示菜单
		{
			ShowContextMenu(InMouseEvent.GetScreenSpacePosition());
		}
		return FReply::Handled();
	}
	
	// 左键点击
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		// Shift + 左键 - 快速移动
		if (InMouseEvent.IsShiftDown())
		{
			OnShiftClick();
			return FReply::Handled();
		}
		// Ctrl + 左键 - 拆分
		else if (InMouseEvent.IsControlDown())
		{
			OnCtrlClick();
			return FReply::Handled();
		}
		
		// 普通左键点击 - 可能触发拖动
		if (!IsEmpty())  // 只有非空槽位才能拖动
		{
			UE_LOG(LogGaia, Log, TEXT("[物品槽位] DetectDrag: SlotID=%d, ItemUID=%s"), SlotID, *ItemUID.ToString());
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UGaiaItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ⭐ NativeOnDragDetected 被调用: SlotID=%d, IsEmpty=%d"),
		SlotID, IsEmpty());
	
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	// 只有非空槽位才能拖动
	if (IsEmpty())
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ❌ 槽位为空，取消拖动"));
		return;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ✅ 开始拖动: Item=%s, Container=%s, Slot=%d"),
		*ItemUID.ToString(), *ContainerUID.ToString(), SlotID);
	
	// 创建DragDropOperation
	OutOperation = UGaiaItemDragDropOperation::CreateDragDropOperation(this);
	
	if (OutOperation)
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ✅ DragDropOperation 创建成功"));
		// 设置拖放视觉反馈
		SetHighlighted(true);
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[物品槽位] ❌ DragDropOperation 创建失败！"));
	}
}

bool UGaiaItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ⭐ NativeOnDrop 被调用: SlotID=%d"), SlotID);
	
	SetDropTarget(false);
	
	// 检查是否是物品拖放操作
	UGaiaItemDragDropOperation* ItemDragDrop = Cast<UGaiaItemDragDropOperation>(InOperation);
	if (!ItemDragDrop)
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ❌ 不是物品拖放操作"));
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ✅ 接收拖放: Container=%s, Slot=%d, IsEmpty=%d"),
		*ContainerUID.ToString(), SlotID, IsEmpty());
	
	// 执行拖放操作
	bool bSuccess = ItemDragDrop->ExecuteDropToSlot(this);
	
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 拖放操作结果: %s"), bSuccess ? TEXT("✅ 成功") : TEXT("❌ 失败"));
	
	return bSuccess;
}

void UGaiaItemSlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	
	// 检查是否是物品拖放操作
	UGaiaItemDragDropOperation* ItemDragDrop = Cast<UGaiaItemDragDropOperation>(InOperation);
	if (ItemDragDrop)
	{
		// 检查是否可以接收拖放
		FText ErrorMessage;
		if (ItemDragDrop->CanDropToSlot(this, ErrorMessage))
		{
			SetDropTarget(true);
		}
		else
		{
			// TODO: 显示错误提示
			UE_LOG(LogGaia, Verbose, TEXT("[物品槽位] 不能接收拖放: %s"), *ErrorMessage.ToString());
		}
	}
}

void UGaiaItemSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	
	SetDropTarget(false);
}

void UGaiaItemSlotWidget::InitializeSlot(const FGuid& InContainerUID, int32 InSlotID)
{
	ContainerUID = InContainerUID;
	SlotID = InSlotID;
	
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ⭐ InitializeSlot 被调用: Container=%s, Slot=%d"),
		*ContainerUID.ToString(), SlotID);
	
	// 刷新显示
	RefreshSlot();
}

void UGaiaItemSlotWidget::RefreshSlot()
{
	if (!ContainerUID.IsValid())
	{
		UE_LOG(LogGaia, Verbose, TEXT("[物品槽位] RefreshSlot: Container无效"));
		SetEmpty();
		return;
	}
	
	// 获取 RPC 组件（客户端使用缓存数据）
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 无法获取 PlayerController"));
		SetEmpty();
		return;
	}
	
	UGaiaInventoryRPCComponent* RPCComp = PC->FindComponentByClass<UGaiaInventoryRPCComponent>();
	if (!RPCComp)
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 无法获取 RPC 组件"));
		SetEmpty();
		return;
	}
	
	// 从 RPC 组件的缓存中获取容器数据
	FGaiaContainerInstance Container;
	if (!RPCComp->GetCachedContainer(ContainerUID, Container))
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 容器不存在于缓存: %s"), *ContainerUID.ToString());
		SetEmpty();
		return;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] RefreshSlot: Container=%s, SlotID=%d, TotalSlots=%d"),
		*ContainerUID.ToString(), SlotID, Container.Slots.Num());
	
	// 查找槽位
	int32 SlotIndex = Container.GetSlotIndexByID(SlotID);
	if (SlotIndex == INDEX_NONE || SlotIndex >= Container.Slots.Num())
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 槽位索引无效: SlotID=%d, SlotIndex=%d, TotalSlots=%d"),
			SlotID, SlotIndex, Container.Slots.Num());
		SetEmpty();
		return;
	}
	
	const FGaiaSlotInfo& SlotInfo = Container.Slots[SlotIndex];
	
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] 槽位信息: SlotID=%d, ItemUID=%s, IsEmpty=%d"),
		SlotInfo.SlotID, *SlotInfo.ItemInstanceUID.ToString(), !SlotInfo.ItemInstanceUID.IsValid());
	
	// 检查槽位是否为空
	if (!SlotInfo.ItemInstanceUID.IsValid())
	{
		UE_LOG(LogGaia, Verbose, TEXT("[物品槽位] 槽位为空: SlotID=%d"), SlotID);
		SetEmpty();
		return;
	}
	
	// 从 RPC 组件的缓存中获取物品数据
	FGaiaItemInstance ItemInstance;
	if (!RPCComp->GetCachedItem(SlotInfo.ItemInstanceUID, ItemInstance))
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 物品不存在于缓存: %s"), *SlotInfo.ItemInstanceUID.ToString());
		SetEmpty();
		return;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] 找到物品: UID=%s, Def=%s, Qty=%d"),
		*ItemInstance.InstanceUID.ToString(), *ItemInstance.ItemDefinitionID.ToString(), ItemInstance.Quantity);
	
	// 设置槽位数据
	SetSlotData(ItemInstance);
}

void UGaiaItemSlotWidget::SetEmpty()
{
	UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ⭐ SetEmpty 被调用: SlotID=%d"), SlotID);
	
	ItemUID = FGuid();
	ItemDefinitionID = NAME_None;
	Quantity = 0;
	
	// 更新视觉
	if (Image_Icon)
	{
		Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位]   → Image_Icon 设置为 Collapsed"));
	}
	
	if (Text_Quantity)
	{
		Text_Quantity->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	UpdateSlotVisuals();
	
	UE_LOG(LogGaia, Verbose, TEXT("[物品槽位] 设置为空: Container=%s, Slot=%d"),
		*ContainerUID.ToString(), SlotID);
}

void UGaiaItemSlotWidget::SetSlotData(const FGaiaItemInstance& ItemInstance)
{
	ItemUID = ItemInstance.InstanceUID;
	ItemDefinitionID = ItemInstance.ItemDefinitionID;
	Quantity = ItemInstance.Quantity;
	
	// 获取物品定义
	UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(GetWorld());
	if (InvSys)
	{
		FGaiaItemDefinition ItemDef;
		if (UGaiaInventorySubsystem::GetItemDefinition(ItemDefinitionID, ItemDef))
		{
			// 加载图标
			LoadItemIcon(ItemDef);
			
			// 更新数量文本
			if (Text_Quantity)
			{
				if (Quantity > 1)
				{
					Text_Quantity->SetText(FText::AsNumber(Quantity));
					Text_Quantity->SetVisibility(ESlateVisibility::Visible);
				}
				else
				{
					Text_Quantity->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
	
	UpdateSlotVisuals();
	
	UE_LOG(LogGaia, Verbose, TEXT("[物品槽位] 设置数据: Item=%s, Def=%s, Qty=%d"),
		*ItemUID.ToString(), *ItemDefinitionID.ToString(), Quantity);
}

void UGaiaItemSlotWidget::SetHighlighted(bool bHighlight)
{
	if (bIsHighlighted != bHighlight)
	{
		bIsHighlighted = bHighlight;
		UpdateSlotVisuals();
	}
}

void UGaiaItemSlotWidget::SetSelected(bool bSelect)
{
	if (bIsSelected != bSelect)
	{
		bIsSelected = bSelect;
		UpdateSlotVisuals();
	}
}

void UGaiaItemSlotWidget::SetDropTarget(bool bIsTarget)
{
	if (bIsDropTarget != bIsTarget)
	{
		bIsDropTarget = bIsTarget;
		UpdateSlotVisuals();
	}
}

void UGaiaItemSlotWidget::OnRightClick_Implementation()
{
	// 注意：现在右键菜单由 ShowContextMenu 处理
	// 这个函数保留是为了向后兼容和蓝图扩展
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] OnRightClick (Legacy): Item=%s"), *ItemUID.ToString());
}

void UGaiaItemSlotWidget::OnShiftClick_Implementation()
{
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] Shift+点击: Item=%s"), *ItemUID.ToString());
	
	// TODO: 快速移动物品到其他容器
}

void UGaiaItemSlotWidget::OnCtrlClick_Implementation()
{
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] Ctrl+点击: Item=%s"), *ItemUID.ToString());
	
	// TODO: 拆分物品
}

void UGaiaItemSlotWidget::UpdateSlotVisuals()
{
	if (!Border_Background)
	{
		return;
	}
	
	// 根据状态选择颜色
	FLinearColor TargetColor = EmptySlotColor;
	
	if (bIsDropTarget)
	{
		TargetColor = DropTargetSlotColor;
	}
	else if (bIsSelected)
	{
		TargetColor = SelectedSlotColor;
	}
	else if (bIsHighlighted)
	{
		TargetColor = HighlightSlotColor;
	}
	else if (!IsEmpty())
	{
		TargetColor = NormalSlotColor;
	}
	
	// 设置边框颜色
	Border_Background->SetBrushColor(TargetColor);
}

void UGaiaItemSlotWidget::LoadItemIcon(const FGaiaItemDefinition& ItemDef)
{
	if (!Image_Icon)
	{
		UE_LOG(LogGaia, Error, TEXT("[物品槽位] ❌ Image_Icon为nullptr！UMG组件未绑定！"));
		return;
	}
	
	// 如果有图标，加载并显示
	if (ItemDef.ItemIcon.IsNull())
	{
		Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 物品没有图标: %s"), *ItemDefinitionID.ToString());
		return;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[物品槽位] 加载图标: 路径=%s"), *ItemDef.ItemIcon.ToString());
	
	// TODO: 异步加载图标
	// 现在先简单处理
	UTexture2D* IconTexture = ItemDef.ItemIcon.LoadSynchronous();
	if (IconTexture)
	{
		UE_LOG(LogGaia, Log, TEXT("[物品槽位] ✅ 图标加载成功: %s, Size=%dx%d"), 
			*IconTexture->GetName(), IconTexture->GetSizeX(), IconTexture->GetSizeY());
		
		// 设置图标
		Image_Icon->SetBrushFromTexture(IconTexture);
		Image_Icon->SetVisibility(ESlateVisibility::Visible);
		
		// 确保不透明度正确
		Image_Icon->SetRenderOpacity(1.0f);
		
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] ✅ 设置完成:"));
		UE_LOG(LogGaia, Warning, TEXT("  - Visibility: %d (0=Visible)"), (int32)Image_Icon->GetVisibility());
		UE_LOG(LogGaia, Warning, TEXT("  - RenderOpacity: %f"), Image_Icon->GetRenderOpacity());
		UE_LOG(LogGaia, Warning, TEXT("  - Brush Size: %s"), *Image_Icon->GetBrush().GetImageSize().ToString());
		UE_LOG(LogGaia, Warning, TEXT("  - Brush Resource: %s"), Image_Icon->GetBrush().GetResourceObject() ? TEXT("✅ 有效") : TEXT("❌ nullptr"));
		
		// 检查父Widget
		UWidget* Parent = Image_Icon->GetParent();
		if (Parent)
		{
			UE_LOG(LogGaia, Warning, TEXT("  - Parent: %s, Visibility=%d"), *Parent->GetName(), (int32)Parent->GetVisibility());
		}
	}
	else
	{
		Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogGaia, Error, TEXT("[物品槽位] ❌ 无法加载图标: %s"), *ItemDefinitionID.ToString());
	}
}

// ========================================
// 右键菜单实现
// ========================================

void UGaiaItemSlotWidget::ShowContextMenu(FVector2D ScreenPosition)
{
	UE_LOG(LogGaia, Log, TEXT("[右键菜单] ShowContextMenu: SlotID=%d, ItemUID=%s, ScreenPos=(%.1f, %.1f)"),
		SlotID, *ItemUID.ToString(), ScreenPosition.X, ScreenPosition.Y);

	// 检查是否为空槽位
	if (IsEmpty())
	{
		UE_LOG(LogGaia, Warning, TEXT("[右键菜单] 槽位为空，取消显示菜单"));
		return;
	}

	// 获取物品定义
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(ItemDef))
	{
		UE_LOG(LogGaia, Warning, TEXT("[右键菜单] 无法获取物品定义"));
		return;
	}

	// 检查菜单类型 - 如果是None则不显示菜单
	if (ItemDef.ContextMenuType == EItemContextMenuType::None)
	{
		UE_LOG(LogGaia, Log, TEXT("[右键菜单] 物品菜单类型为None，不显示菜单: %s"), *ItemDefinitionID.ToString());
		return;
	}

	// 检查菜单类是否设置
	if (!ContextMenuClass)
	{
		UE_LOG(LogGaia, Error, TEXT("[右键菜单] ContextMenuClass 未设置！请在 WBP_ItemSlot 蓝图中配置"));
		return;
	}

	// 获取UIManager
	UGaiaUIManagerSubsystem* UIManager = UGaiaUIManagerSubsystem::Get(this);
	if (!UIManager)
	{
		UE_LOG(LogGaia, Error, TEXT("[右键菜单] 无法获取 UIManager！菜单无法显示"));
		return;
	}

	// 捕获变量用于Lambda
	FGuid CapturedItemUID = ItemUID;
	FGaiaItemDefinition CapturedItemDef = ItemDef;
	FVector2D CapturedScreenPos = ScreenPosition;

	UE_LOG(LogGaia, Log, TEXT("[右键菜单] 使用CommonUI方式创建菜单..."));

	// 使用CommonUI的正确方式：让Layer创建Widget，在回调中初始化
	UGaiaItemContextMenu* ContextMenu = UIManager->PushWidgetToLayerWithInit<UGaiaItemContextMenu>(
		FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu")),
		ContextMenuClass,
		[CapturedItemUID, CapturedItemDef, CapturedScreenPos](UGaiaItemContextMenu& Menu)
		{
			// 在Widget创建后、激活前初始化
			Menu.InitializeMenu(CapturedItemUID, CapturedItemDef);
			Menu.SetMenuPosition(CapturedScreenPos);
		}
	);

	if (ContextMenu)
	{
		UE_LOG(LogGaia, Log, TEXT("[右键菜单] 菜单创建并推送成功"));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[右键菜单] 菜单创建失败"));
	}
}

bool UGaiaItemSlotWidget::GetItemDefinition(FGaiaItemDefinition& OutItemDef) const
{
	if (IsEmpty())
	{
		return false;
	}

	// 从库存子系统获取物品
	UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(GetWorld());
	if (!InvSys)
	{
		UE_LOG(LogGaia, Error, TEXT("[物品槽位] 无法获取 InventorySubsystem"));
		return false;
	}

	FGaiaItemInstance Item;
	if (!InvSys->FindItemByUID(ItemUID, Item))
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 无法找到物品: %s"), *ItemUID.ToString());
		return false;
	}

	// 获取物品定义
	if (!UGaiaInventorySubsystem::GetItemDefinition(Item.ItemDefinitionID, OutItemDef))
	{
		UE_LOG(LogGaia, Warning, TEXT("[物品槽位] 无法获取物品定义: %s"), *Item.ItemDefinitionID.ToString());
		return false;
	}

	UE_LOG(LogGaia, Verbose, TEXT("[物品槽位] 成功获取物品定义: %s, MenuType=%d"),
		*Item.ItemDefinitionID.ToString(), (int32)OutItemDef.ContextMenuType);

	return true;
}

