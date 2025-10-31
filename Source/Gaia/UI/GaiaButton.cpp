#include "GaiaButton.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "GaiaLogChannels.h"

UGaiaButton::UGaiaButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 默认启用 Tick（用于动画）
	bIsVariable = true;
	SetVisibility(ESlateVisibility::Visible);
}

void UGaiaButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 在编辑器中预览样式
	UpdateAppearance();
}

void UGaiaButton::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化按钮状态
	UpdateAppearance();

	// 隐藏加载指示器
	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 隐藏徽章
	if (Badge)
	{
		Badge->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGaiaButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 更新脉冲动画
	if (bIsPlayingPulse && Background)
	{
		PulseTime += InDeltaTime * PulseSpeed;
		float Scale = 1.0f + (FMath::Sin(PulseTime * 2.0f * PI) * 0.5f + 0.5f) * (PulseScale - 1.0f);
		Background->SetRenderScale(FVector2D(Scale, Scale));
	}
}

// ========================================
// 样式管理
// ========================================

void UGaiaButton::SetButtonStyle(EGaiaButtonStyle InStyle)
{
	CurrentStyle = InStyle;
	UpdateAppearance();
}

void UGaiaButton::SetButtonSize(EGaiaButtonSize Size)
{
	CurrentSize = Size;
	FGaiaButtonSizeData SizeData = GetPresetSizeData(Size);
	ApplySizeData(SizeData);
}

void UGaiaButton::SetCustomColors(FLinearColor Normal, FLinearColor Hovered, FLinearColor Pressed, FLinearColor Disabled)
{
	CustomStyleData.NormalColor = Normal;
	CustomStyleData.HoveredColor = Hovered;
	CustomStyleData.PressedColor = Pressed;
	CustomStyleData.DisabledColor = Disabled;

	if (CurrentStyle == EGaiaButtonStyle::Custom)
	{
		UpdateAppearance();
	}
}

// ========================================
// 内容管理
// ========================================

void UGaiaButton::SetButtonText(const FText& Text)
{
	if (ButtonText)
	{
		ButtonText->SetText(Text);
	}
}

FText UGaiaButton::GetButtonText() const
{
	if (ButtonText)
	{
		return ButtonText->GetText();
	}
	return FText::GetEmpty();
}

void UGaiaButton::SetButtonIcon(UTexture2D* IconTexture)
{
	if (Icon && IconTexture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(IconTexture);
		Brush.ImageSize = FVector2D(16.0f, 16.0f);
		Icon->SetBrush(Brush);
		Icon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UGaiaButton::SetIconVisible(bool bVisible)
{
	if (Icon)
	{
		Icon->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

// ========================================
// 状态管理
// ========================================

void UGaiaButton::SetLoading(bool bIsLoading)
{
	bLoading = bIsLoading;

	if (LoadingIndicator)
	{
		LoadingIndicator->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 加载时禁用按钮
	SetIsEnabled(!bLoading);

	// 隐藏文本和图标
	if (ButtonText)
	{
		ButtonText->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (Icon)
	{
		Icon->SetVisibility(bLoading ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

void UGaiaButton::SetBadgeCount(int32 Count)
{
	BadgeCount = Count;

	if (Badge && BadgeText)
	{
		if (Count > 0)
		{
			Badge->SetVisibility(ESlateVisibility::Visible);
			
			// 如果数量大于99，显示 "99+"
			FText DisplayText = Count > 99 ? 
				FText::FromString(TEXT("99+")) : 
				FText::AsNumber(Count);
			
			BadgeText->SetText(DisplayText);
		}
		else
		{
			Badge->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UGaiaButton::SetButtonTooltip(const FText& InTooltipText)
{
	SetToolTipText(InTooltipText);
}

// ========================================
// 动画
// ========================================

void UGaiaButton::PlayPulseAnimation()
{
	bIsPlayingPulse = true;
	PulseTime = 0.0f;
}

void UGaiaButton::StopPulseAnimation()
{
	bIsPlayingPulse = false;
	
	// 恢复原始缩放
	if (Background)
	{
		Background->SetRenderScale(FVector2D(1.0f, 1.0f));
	}
}

void UGaiaButton::PlayClickAnimation()
{
	if (Background)
	{
		// 简单的缩放动画
		Background->SetRenderScale(FVector2D(0.95f, 0.95f));
		
		// TODO: 使用 UMG 动画或 Tween 系统实现更平滑的动画
		// 这里需要在下一帧恢复缩放
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (Background)
			{
				Background->SetRenderScale(FVector2D(1.0f, 1.0f));
			}
		}, 0.1f, false);
	}
}

// ========================================
// CommonButtonBase 事件重写
// ========================================

void UGaiaButton::NativeOnHovered()
{
	Super::NativeOnHovered();
	
	UpdateAppearance();
	PlaySound(HoverSound);
}

void UGaiaButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	
	UpdateAppearance();
}

void UGaiaButton::NativeOnPressed()
{
	Super::NativeOnPressed();
	
	UpdateAppearance();
}

void UGaiaButton::NativeOnReleased()
{
	Super::NativeOnReleased();
	
	UpdateAppearance();
}

void UGaiaButton::NativeOnClicked()
{
	Super::NativeOnClicked();
	
	PlayClickAnimation();
	PlaySound(ClickSound);
}

// ========================================
// 外观更新
// ========================================

void UGaiaButton::UpdateAppearance()
{
	if (!Background)
	{
		return;
	}

	// 获取样式数据
	FGaiaButtonStyleData StyleData = CurrentStyle == EGaiaButtonStyle::Custom ? 
		CustomStyleData : 
		GetPresetStyleData(CurrentStyle);

	// 应用样式
	ApplyStyleData(StyleData);
}

void UGaiaButton::ApplyStyleData(const FGaiaButtonStyleData& StyleData)
{
	if (!Background)
	{
		return;
	}

	// 根据按钮状态选择颜色
	FLinearColor TargetColor = StyleData.NormalColor;

	if (!GetIsEnabled())
	{
		TargetColor = StyleData.DisabledColor;
	}
	else if (IsPressed())
	{
		TargetColor = StyleData.PressedColor;
	}
	else if (IsHovered())
	{
		TargetColor = StyleData.HoveredColor;
	}

	// 设置背景颜色
	Background->SetBrushColor(TargetColor);

	// 设置文本颜色
	if (ButtonText)
	{
		ButtonText->SetColorAndOpacity(StyleData.TextColor);
		ButtonText->SetFont(StyleData.FontInfo);
	}

	// 设置内边距
	Background->SetPadding(StyleData.Padding);
}

void UGaiaButton::ApplySizeData(const FGaiaButtonSizeData& SizeData)
{
	// 设置按钮大小
	if (Background)
	{
		// 注意：这里需要在 UMG 蓝图中设置 SizeBox 来控制最小宽度和高度
		// 或者使用 SetDesiredSizeScale
	}

	// 设置字体大小
	if (ButtonText)
	{
		FSlateFontInfo FontInfo = ButtonText->GetFont();
		FontInfo.Size = SizeData.FontSize;
		ButtonText->SetFont(FontInfo);
	}

	// 设置图标大小
	if (Icon)
	{
		Icon->SetDesiredSizeOverride(FVector2D(SizeData.IconSize, SizeData.IconSize));
	}

	// 设置内边距
	if (Background)
	{
		Background->SetPadding(SizeData.Padding);
	}
}

// ========================================
// 预设数据获取
// ========================================

FGaiaButtonStyleData UGaiaButton::GetPresetStyleData(EGaiaButtonStyle InStyle) const
{
	FGaiaButtonStyleData Data;

	switch (InStyle)
	{
	case EGaiaButtonStyle::Primary:
		Data.NormalColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);   // 蓝色
		Data.HoveredColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
		Data.PressedColor = FLinearColor(0.1f, 0.4f, 0.9f, 1.0f);
		Data.DisabledColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);
		Data.TextColor = FLinearColor::White;
		break;

	case EGaiaButtonStyle::Secondary:
		Data.NormalColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);   // 灰色
		Data.HoveredColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
		Data.PressedColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);
		Data.DisabledColor = FLinearColor(0.25f, 0.25f, 0.25f, 0.5f);
		Data.TextColor = FLinearColor::White;
		break;

	case EGaiaButtonStyle::Success:
		Data.NormalColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);   // 绿色
		Data.HoveredColor = FLinearColor(0.3f, 0.9f, 0.3f, 1.0f);
		Data.PressedColor = FLinearColor(0.1f, 0.7f, 0.1f, 1.0f);
		Data.DisabledColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);
		Data.TextColor = FLinearColor::White;
		break;

	case EGaiaButtonStyle::Danger:
		Data.NormalColor = FLinearColor(0.9f, 0.2f, 0.2f, 1.0f);   // 红色
		Data.HoveredColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);
		Data.PressedColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
		Data.DisabledColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);
		Data.TextColor = FLinearColor::White;
		break;

	case EGaiaButtonStyle::Warning:
		Data.NormalColor = FLinearColor(1.0f, 0.7f, 0.0f, 1.0f);   // 黄色
		Data.HoveredColor = FLinearColor(1.0f, 0.8f, 0.1f, 1.0f);
		Data.PressedColor = FLinearColor(0.9f, 0.6f, 0.0f, 1.0f);
		Data.DisabledColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);
		Data.TextColor = FLinearColor::Black;
		break;

	case EGaiaButtonStyle::Info:
		Data.NormalColor = FLinearColor(0.2f, 0.8f, 0.8f, 1.0f);   // 青色
		Data.HoveredColor = FLinearColor(0.3f, 0.9f, 0.9f, 1.0f);
		Data.PressedColor = FLinearColor(0.1f, 0.7f, 0.7f, 1.0f);
		Data.DisabledColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);
		Data.TextColor = FLinearColor::White;
		break;

	case EGaiaButtonStyle::Transparent:
		Data.NormalColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);   // 透明
		Data.HoveredColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.1f);
		Data.PressedColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.2f);
		Data.DisabledColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		Data.TextColor = FLinearColor::White;
		break;

	case EGaiaButtonStyle::Custom:
	default:
		Data = CustomStyleData;
		break;
	}

	// 设置字体
	Data.FontInfo.FontObject = nullptr; // 使用默认字体
	Data.FontInfo.Size = 14;

	return Data;
}

FGaiaButtonSizeData UGaiaButton::GetPresetSizeData(EGaiaButtonSize Size) const
{
	FGaiaButtonSizeData Data;

	switch (Size)
	{
	case EGaiaButtonSize::Small:
		Data.Height = 24.0f;
		Data.MinWidth = 64.0f;
		Data.FontSize = 12;
		Data.IconSize = 12.0f;
		Data.Padding = FMargin(12.0f, 4.0f);
		break;

	case EGaiaButtonSize::Medium:
		Data.Height = 32.0f;
		Data.MinWidth = 80.0f;
		Data.FontSize = 14;
		Data.IconSize = 16.0f;
		Data.Padding = FMargin(16.0f, 8.0f);
		break;

	case EGaiaButtonSize::Large:
		Data.Height = 40.0f;
		Data.MinWidth = 96.0f;
		Data.FontSize = 16;
		Data.IconSize = 20.0f;
		Data.Padding = FMargin(20.0f, 10.0f);
		break;

	case EGaiaButtonSize::XLarge:
		Data.Height = 48.0f;
		Data.MinWidth = 112.0f;
		Data.FontSize = 18;
		Data.IconSize = 24.0f;
		Data.Padding = FMargin(24.0f, 12.0f);
		break;

	default:
		break;
	}

	return Data;
}

// ========================================
// 声音播放
// ========================================

void UGaiaButton::PlaySound(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

