#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "GaiaButton.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UProgressBar;
class UOverlay;
class USoundBase;
class UDataTable;

/**
 * Gaia 按钮样式预设
 */
UENUM(BlueprintType)
enum class EGaiaButtonStyle : uint8
{
	Primary		UMETA(DisplayName = "主要按钮（蓝色）"),
	Secondary	UMETA(DisplayName = "次要按钮（灰色）"),
	Success		UMETA(DisplayName = "成功按钮（绿色）"),
	Danger		UMETA(DisplayName = "危险按钮（红色）"),
	Warning		UMETA(DisplayName = "警告按钮（黄色）"),
	Info		UMETA(DisplayName = "信息按钮（青色）"),
	Transparent	UMETA(DisplayName = "透明按钮"),
	Custom		UMETA(DisplayName = "自定义样式")
};

/**
 * Gaia 按钮尺寸预设
 */
UENUM(BlueprintType)
enum class EGaiaButtonSize : uint8
{
	Small	UMETA(DisplayName = "小（24px）"),
	Medium	UMETA(DisplayName = "中（32px）"),
	Large	UMETA(DisplayName = "大（40px）"),
	XLarge	UMETA(DisplayName = "超大（48px）")
};

/**
 * Gaia 按钮样式数据（用于 DataTable）
 */
USTRUCT(BlueprintType)
struct FGaiaButtonStyleData : public FTableRowBase
{
	GENERATED_BODY()

	/** 正常状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor NormalColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);

	/** 悬停状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor HoveredColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

	/** 按下状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor PressedColor = FLinearColor(0.1f, 0.4f, 0.9f, 1.0f);

	/** 禁用状态颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor DisabledColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);

	/** 文本颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor TextColor = FLinearColor::White;

	/** 字体信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Font")
	FSlateFontInfo FontInfo;

	/** 内边距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FMargin Padding = FMargin(16.0f, 8.0f);

	/** 圆角半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	float CornerRadius = 4.0f;
};

/**
 * Gaia 按钮尺寸数据
 */
USTRUCT(BlueprintType)
struct FGaiaButtonSizeData
{
	GENERATED_BODY()

	/** 按钮高度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height = 32.0f;

	/** 最小宽度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinWidth = 80.0f;

	/** 字体大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FontSize = 14;

	/** 图标大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IconSize = 16.0f;

	/** 内边距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin Padding = FMargin(16.0f, 8.0f);
};

/**
 * Gaia 通用按钮基类
 * 
 * 提供开箱即用的按钮样式和功能：
 * - 预设样式（Primary, Secondary, Success, Danger 等）
 * - 尺寸管理（Small, Medium, Large, XLarge）
 * - 加载状态动画
 * - 徽章/通知数量显示
 * - 声音效果
 * - 工具提示
 * 
 * UMG 层级结构（需要在蓝图中创建）：
 * [Overlay] Root
 * ├─ [Border] Background (必须，命名为 Background)
 * │   └─ [Image] BackgroundImage (可选)
 * ├─ [Overlay] Content (必须，命名为 Content)
 * │   ├─ [Image] Icon (可选，命名为 Icon)
 * │   ├─ [TextBlock] ButtonText (可选，命名为 ButtonText)
 * │   └─ [ProgressBar] LoadingIndicator (可选，命名为 LoadingIndicator)
 * └─ [Border] Badge (可选，命名为 Badge)
 *     └─ [TextBlock] BadgeText (可选，命名为 BadgeText)
 */
UCLASS(Abstract, Blueprintable)
class GAIAGAME_API UGaiaButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UGaiaButton(const FObjectInitializer& ObjectInitializer);

	//~Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~End UUserWidget Interface

	// ========================================
	// 样式管理
	// ========================================

	/** 设置按钮样式 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetButtonStyle(EGaiaButtonStyle InStyle);

	/** 获取当前样式 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Button")
	EGaiaButtonStyle GetButtonStyle() const { return CurrentStyle; }

	/** 设置按钮尺寸 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetButtonSize(EGaiaButtonSize Size);

	/** 获取当前尺寸 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Button")
	EGaiaButtonSize GetButtonSize() const { return CurrentSize; }

	/** 设置自定义颜色（仅当样式为 Custom 时有效） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetCustomColors(FLinearColor Normal, FLinearColor Hovered, FLinearColor Pressed, FLinearColor Disabled);

	// ========================================
	// 内容管理
	// ========================================

	/** 设置按钮文本 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetButtonText(const FText& Text);

	/** 获取按钮文本 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Button")
	FText GetButtonText() const;

	/** 设置按钮图标 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetButtonIcon(UTexture2D* IconTexture);

	/** 设置图标可见性 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetIconVisible(bool bVisible);

	// ========================================
	// 状态管理
	// ========================================

	/** 设置加载状态 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetLoading(bool bIsLoading);

	/** 是否正在加载 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Button")
	bool IsLoading() const { return bLoading; }

	/** 设置徽章数量（0 为隐藏） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetBadgeCount(int32 Count);

	/** 获取徽章数量 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Button")
	int32 GetBadgeCount() const { return BadgeCount; }

	/** 设置工具提示文本（包装父类方法） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
	void SetButtonTooltip(const FText& InTooltipText);

	// ========================================
	// 动画
	// ========================================

	/** 播放脉冲动画（吸引注意力） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button|Animation")
	void PlayPulseAnimation();

	/** 停止脉冲动画 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button|Animation")
	void StopPulseAnimation();

	/** 播放点击反馈动画 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Button|Animation")
	void PlayClickAnimation();

protected:
	//~Begin UCommonButtonBase Interface
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void NativeOnPressed() override;
	virtual void NativeOnReleased() override;
	virtual void NativeOnClicked() override;
	//~End UCommonButtonBase Interface

	/** 更新按钮外观（根据当前状态和样式） */
	virtual void UpdateAppearance();

	/** 应用样式数据 */
	virtual void ApplyStyleData(const FGaiaButtonStyleData& StyleData);

	/** 应用尺寸数据 */
	virtual void ApplySizeData(const FGaiaButtonSizeData& SizeData);

	/** 获取预设样式数据 */
	FGaiaButtonStyleData GetPresetStyleData(EGaiaButtonStyle InStyle) const;

	/** 获取预设尺寸数据 */
	FGaiaButtonSizeData GetPresetSizeData(EGaiaButtonSize Size) const;

	/** 播放声音 */
	void PlaySound(USoundBase* Sound);

	// ========================================
	// UMG 组件（通过 BindWidget 或 BindWidgetOptional 绑定）
	// ========================================

	/** 背景边框 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> Background;

	/** 内容容器 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> Content;

	/** 按钮文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ButtonText;

	/** 按钮图标 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	/** 加载指示器 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> LoadingIndicator;

	/** 徽章容器 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Badge;

	/** 徽章文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BadgeText;

	// ========================================
	// 配置属性
	// ========================================

	/** 当前样式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Style")
	EGaiaButtonStyle CurrentStyle = EGaiaButtonStyle::Primary;

	/** 当前尺寸 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Style")
	EGaiaButtonSize CurrentSize = EGaiaButtonSize::Medium;

	/** 样式数据表（可选，用于从外部加载样式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Style")
	TObjectPtr<UDataTable> StyleDataTable;

	/** 自定义样式数据（当 CurrentStyle 为 Custom 时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Style")
	FGaiaButtonStyleData CustomStyleData;

	/** 悬停声音 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Sound")
	TObjectPtr<USoundBase> HoverSound;

	/** 点击声音 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Sound")
	TObjectPtr<USoundBase> ClickSound;

	/** 是否启用脉冲动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Animation")
	bool bEnablePulseAnimation = false;

	/** 脉冲动画速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Animation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PulseSpeed = 1.0f;

	/** 脉冲动画缩放范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Button|Animation", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float PulseScale = 1.1f;

	// ========================================
	// 内部状态
	// ========================================

	/** 是否正在加载 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|Button|State")
	bool bLoading = false;

	/** 徽章数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|Button|State")
	int32 BadgeCount = 0;

	/** 是否正在播放脉冲动画 */
	bool bIsPlayingPulse = false;

	/** 脉冲动画时间累积 */
	float PulseTime = 0.0f;
};

