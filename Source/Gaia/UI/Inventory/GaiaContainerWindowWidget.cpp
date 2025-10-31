#include "GaiaContainerWindowWidget.h"
#include "GaiaContainerGridWidget.h"
#include "GaiaContainerDebugInfoWidget.h"
#include "Gameplay/Inventory/GaiaInventorySubsystem.h"
#include "UI/GaiaUIManagerSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "GaiaLogChannels.h"

void UGaiaContainerWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定关闭按钮
	if (Button_Close)
	{
		Button_Close->OnClicked.AddDynamic(this, &UGaiaContainerWindowWidget::OnCloseButtonClicked);
	}
}

void UGaiaContainerWindowWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	UE_LOG(LogGaia, Verbose, TEXT("[容器窗口] 激活: %s"), *ContainerUID.ToString());
	
	// 窗口激活时刷新一次调试信息
	if (bDebugMode)
	{
		RefreshDebugInfo();
	}
}

void UGaiaContainerWindowWidget::NativeOnDeactivated()
{
	UE_LOG(LogGaia, Warning, TEXT("[容器窗口] ⭐ 停用: %s"), *ContainerUID.ToString());
	
	Super::NativeOnDeactivated();
	
	// 通知 UIManager 从映射表移除窗口
	UGaiaUIManagerSubsystem* UIManager = UGaiaUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UE_LOG(LogGaia, Warning, TEXT("[容器窗口] 通知 UIManager 关闭窗口"));
		UIManager->CloseContainerWindow(this);
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[容器窗口] ❌ 无法获取 UIManager，窗口可能无法正确清理"));
	}
}

UWidget* UGaiaContainerWindowWidget::NativeGetDesiredFocusTarget() const
{
	// 焦点设置到关闭按钮（用于手柄/键盘导航）
	return Button_Close ? Button_Close : Super::NativeGetDesiredFocusTarget();
}

void UGaiaContainerWindowWidget::InitializeWindow(const FGuid& InContainerUID)
{
	ContainerUID = InContainerUID;

	// 更新标题
	if (Text_Title)
	{
		if (UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(GetWorld()))
		{
			FGaiaContainerInstance Container;
			if (InvSys->FindContainerByUID(ContainerUID, Container))
			{
				FString TitleText = Container.DebugDisplayName.IsEmpty()
					? Container.ContainerDefinitionID.ToString()
					: Container.DebugDisplayName;
				
				Text_Title->SetText(FText::FromString(TitleText));
			}
		}
	}

	// 初始化容器网格
	if (ContainerGrid)
	{
		ContainerGrid->InitializeContainer(ContainerUID);
	}

	// 刷新调试信息
	if (bDebugMode)
	{
		RefreshDebugInfo();
	}

	UE_LOG(LogGaia, Log, TEXT("[容器窗口] 初始化: %s"), *ContainerUID.ToString());
}

void UGaiaContainerWindowWidget::RequestClose()
{
	// 停用Widget（Layer System会自动处理移除）
	DeactivateWidget();
	
	UE_LOG(LogGaia, Log, TEXT("[容器窗口] 请求关闭: %s"), *ContainerUID.ToString());
}

void UGaiaContainerWindowWidget::SetDebugMode(bool bEnabled)
{
	bDebugMode = bEnabled;

	if (DebugInfoPanel)
	{
		DebugInfoPanel->SetVisibility(bEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (bEnabled)
	{
		RefreshDebugInfo();
	}
}

void UGaiaContainerWindowWidget::RefreshDebugInfo()
{
	if (!bDebugMode || !DebugInfoPanel)
	{
		return;
	}

	if (UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(GetWorld()))
	{
		FContainerUIDebugInfo DebugInfo = InvSys->GetContainerDebugInfo(ContainerUID);
		DebugInfoPanel->UpdateDebugInfo(DebugInfo);
	}
}

void UGaiaContainerWindowWidget::OnCloseButtonClicked()
{
	RequestClose();
}

