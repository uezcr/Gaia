#include "GaiaUIManagerSubsystem.h"
#include "UI/GaiaPrimaryGameLayout.h"
#include "PrimaryGameLayout.h"
#include "GaiaLogChannels.h"
#include "GameUIPolicy.h"
#include "Inventory/GaiaContainerWindowWidget.h"
#include "Inventory/GaiaContainerGridWidget.h"
#include "Gameplay/Inventory/GaiaInventoryRPCComponent.h"
#include "Player/GaiaPlayerController.h"

UGaiaUIManagerSubsystem::UGaiaUIManagerSubsystem()
{
}

void UGaiaUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogGaia, Log, TEXT("[Gaia UI管理器] 初始化"));
	
	// 检查 UIPolicy 是否创建成功
	if (UGameUIPolicy* Policy = GetCurrentUIPolicy())
	{
		UE_LOG(LogGaia, Log, TEXT("[Gaia UI管理器] ✅ UIPolicy已创建: %s"), *Policy->GetName());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI管理器] ❌ UIPolicy为null！检查Config/DefaultGame.ini中的DefaultUIPolicyClass配置"));
	}
	
	// 注意：OnInventoryUpdated 事件在 RPC 组件中，由 PlayerController 管理
	// 我们会在 NotifyPlayerAdded 时绑定到具体的玩家
	UE_LOG(LogGaia, Log, TEXT("[Gaia UI管理器] UI管理器初始化完成，等待玩家添加"));
}

void UGaiaUIManagerSubsystem::Deinitialize()
{
	// 清理所有打开的容器窗口
	CloseAllContainerWindows();
	
	UE_LOG(LogGaia, Log, TEXT("[Gaia UI管理器] 反初始化"));
	
	Super::Deinitialize();
}

bool UGaiaUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 不是专用服务器就创建
	if (UGameInstance* GameInstance = Cast<UGameInstance>(Outer))
	{
		return !GameInstance->IsDedicatedServerInstance();
	}
	return false;
}

UGaiaUIManagerSubsystem* UGaiaUIManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UGaiaUIManagerSubsystem>();
		}
	}
	
	return nullptr;
}

UGaiaContainerWindowWidget* UGaiaUIManagerSubsystem::OpenContainerWindow(
	const FGuid& ContainerUID,
	bool bSuspendInputUntilComplete)
{
	if (!ContainerUID.IsValid())
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI] 打开容器窗口失败：容器UID无效"));
		return nullptr;
	}

	// 检查是否已打开
	if (OpenContainerWindows.Contains(ContainerUID))
	{
		UE_LOG(LogGaia, Warning, TEXT("[Gaia UI] 容器窗口已打开: %s"), *ContainerUID.ToString());
		return OpenContainerWindows[ContainerUID];
	}

	// 获取PrimaryGameLayout
	UGaiaPrimaryGameLayout* Layout = GetPrimaryGameLayout();
	if (!Layout)
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI] 无法获取PrimaryGameLayout"));
		return nullptr;
	}

	// 同步加载Widget类（如果需要异步，可以使用PushWidgetToLayerStackAsync）
	UClass* WidgetClass = ContainerWindowClass.LoadSynchronous();
	if (!WidgetClass)
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI] 无法加载ContainerWindowClass"));
		return nullptr;
	}

	// 推入到容器层
	UGaiaContainerWindowWidget* Window = Layout->PushWidgetToLayerStack<UGaiaContainerWindowWidget>(
		UGaiaPrimaryGameLayout::GetContainerLayerTag(),
		WidgetClass,
		[ContainerUID](UGaiaContainerWindowWidget& Widget)
		{
			// 初始化回调
			Widget.InitializeWindow(ContainerUID);
		}
	);

	if (Window)
	{
		// 记录到映射表
		OpenContainerWindows.Add(ContainerUID, Window);
		
		// 第一次打开窗口时，绑定RPC事件（只绑定一次）
		if (OpenContainerWindows.Num() == 1)
		{
			BindInventoryEvents();
		}
		
		UE_LOG(LogGaia, Log, TEXT("[Gaia UI] 打开容器窗口: %s"), *ContainerUID.ToString());
	}

	return Window;
}

void UGaiaUIManagerSubsystem::CloseContainerWindow(UGaiaContainerWindowWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	FGuid ContainerUID = Widget->GetContainerUID();
	
	// 从映射表移除
	OpenContainerWindows.Remove(ContainerUID);

	// 从Layer移除
	UGaiaPrimaryGameLayout* Layout = GetPrimaryGameLayout();
	if (Layout)
	{
		Layout->FindAndRemoveWidgetFromLayer(Widget);
	}

	UE_LOG(LogGaia, Log, TEXT("[Gaia UI] 关闭容器窗口: %s"), *ContainerUID.ToString());
}

void UGaiaUIManagerSubsystem::CloseAllContainerWindows()
{
	if (OpenContainerWindows.Num() == 0)
	{
		return;
	}

	UE_LOG(LogGaia, Log, TEXT("[Gaia UI] 关闭所有容器窗口 (%d个)"), OpenContainerWindows.Num());

	// 复制UID列表（避免在迭代时修改容器）
	TArray<FGuid> ContainerUIDs;
	OpenContainerWindows.GetKeys(ContainerUIDs);

	// 逐个关闭
	for (const FGuid& UID : ContainerUIDs)
	{
		if (UGaiaContainerWindowWidget* Window = OpenContainerWindows[UID])
		{
			CloseContainerWindow(Window);
		}
	}
}

bool UGaiaUIManagerSubsystem::IsContainerWindowOpen(const FGuid& ContainerUID) const
{
	return OpenContainerWindows.Contains(ContainerUID);
}

UCommonActivatableWidget* UGaiaUIManagerSubsystem::PushWidgetToLayer(
	FGameplayTag LayerTag,
	TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	UGaiaPrimaryGameLayout* Layout = GetPrimaryGameLayout();
	if (!Layout || !WidgetClass)
	{
		return nullptr;
	}

	return Layout->PushWidgetToLayerStack<UCommonActivatableWidget>(LayerTag, WidgetClass);
}

void UGaiaUIManagerSubsystem::RemoveWidgetFromLayer(UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	UGaiaPrimaryGameLayout* Layout = GetPrimaryGameLayout();
	if (Layout)
	{
		Layout->FindAndRemoveWidgetFromLayer(Widget);
	}
}

UGaiaPrimaryGameLayout* UGaiaUIManagerSubsystem::GetPrimaryGameLayout() const
{
	// 获取主玩家的PrimaryGameLayout
	return Cast<UGaiaPrimaryGameLayout>(
		UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this)
	);
}

void UGaiaUIManagerSubsystem::BindInventoryEvents()
{
	UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] ⭐ 尝试绑定库存事件..."));
	
	// 获取主玩家的 PlayerController
	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI管理器] ❌ 无法获取World"));
		return;
	}
	
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI管理器] ❌ 无法获取PlayerController"));
		return;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] PlayerController类型: %s"), *PC->GetClass()->GetName());
	
	AGaiaPlayerController* GaiaPC = Cast<AGaiaPlayerController>(PC);
	if (!GaiaPC)
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI管理器] ❌ 无法转换为GaiaPlayerController"));
		return;
	}
	
	UGaiaInventoryRPCComponent* RPCComp = GaiaPC->GetInventoryRPCComponent();
	if (!RPCComp)
	{
		UE_LOG(LogGaia, Error, TEXT("[Gaia UI管理器] ❌ 无法获取RPC组件"));
		return;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] RPC组件地址: %p"), RPCComp);
	
	// 检查是否已经绑定过
	if (RPCComp->OnInventoryUpdated.IsBound())
	{
		UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] ⚠️ 事件已有其他绑定"));
	}
	
	// 绑定事件
	RPCComp->OnInventoryUpdated.AddDynamic(this, &UGaiaUIManagerSubsystem::OnInventoryUpdated);
	UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] ✅ 已绑定库存更新事件到RPC组件"));
}

void UGaiaUIManagerSubsystem::OnInventoryUpdated()
{
	UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] ⭐ OnInventoryUpdated: 刷新所有打开的容器窗口"));
	
	// 刷新所有打开的容器窗口
	for (const auto& Pair : OpenContainerWindows)
	{
		if (UGaiaContainerWindowWidget* Window = Pair.Value)
		{
			// 刷新容器网格
			if (UGaiaContainerGridWidget* GridWidget = Window->GetContainerGrid())
			{
				UE_LOG(LogGaia, Warning, TEXT("[Gaia UI管理器] 刷新容器: %s"), *Pair.Key.ToString());
				GridWidget->RefreshAllSlots();
			}
			
			// 刷新调试信息面板（如果启用）
			Window->RefreshDebugInfo();
		}
	}
}

