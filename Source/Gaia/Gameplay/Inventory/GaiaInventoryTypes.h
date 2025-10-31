#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "GaiaInventoryTypes.generated.h"

class UGaiaContainerWindowWidget;

// ========================================
// 网络同步事件委托
// ========================================

/** 库存数据更新事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

/** 库存操作失败事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationFailed, int32, ErrorCode, const FString&, ErrorMessage);

/** 容器打开事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerOpened, const FGuid&, ContainerUID);

/** 容器关闭事件 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerClosed, const FGuid&, ContainerUID);

// ========================================
// 枚举定义
// ========================================

/**
 * 物品右键菜单类型
 * 定义不同类型物品的默认菜单配置
 */
UENUM(BlueprintType)
enum class EItemContextMenuType : uint8
{
	/** 无菜单 */
	None UMETA(DisplayName = "None"),
	
	/** 消耗品 - 使用、丢弃、销毁 */
	Consumable UMETA(DisplayName = "Consumable"),
	
	/** 装备 - 装备、丢弃、销毁 */
	Equipment UMETA(DisplayName = "Equipment"),
	
	/** 容器 - 打开、清空、丢弃 */
	Container UMETA(DisplayName = "Container"),
	
	/** 材料 - 拆分、丢弃、销毁 */
	Material UMETA(DisplayName = "Material"),
	
	/** 任务物品 - 查看详情 */
	QuestItem UMETA(DisplayName = "Quest Item"),
	
	/** 自定义 - 使用自定义菜单配置 */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * 菜单操作类型
 * 定义右键菜单中可执行的操作
 */
UENUM(BlueprintType)
enum class EItemContextAction : uint8
{
	/** 使用物品 */
	Use UMETA(DisplayName = "Use"),
	
	/** 装备物品 */
	Equip UMETA(DisplayName = "Equip"),
	
	/** 卸下装备 */
	Unequip UMETA(DisplayName = "Unequip"),
	
	/** 打开容器 */
	OpenContainer UMETA(DisplayName = "Open Container"),
	
	/** 清空容器 */
	EmptyContainer UMETA(DisplayName = "Empty Container"),
	
	/** 拆分物品 */
	Split UMETA(DisplayName = "Split"),
	
	/** 丢弃物品 */
	Drop UMETA(DisplayName = "Drop"),
	
	/** 销毁物品 */
	Destroy UMETA(DisplayName = "Destroy"),
	
	/** 查看详情 */
	Inspect UMETA(DisplayName = "Inspect"),
	
	/** 自定义操作 */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * 移动物品操作的结果类型
 */
UENUM(BlueprintType)
enum class EMoveItemResult : uint8
{
	Success,                    // 完全成功
	PartialSuccess,            // 部分成功（部分堆叠）
	Failed,                    // 完全失败
	InvalidDefinition,         // 无效定义
	InvalidTarget,             // 目标无效
	VolumeExceeded,            // 体积超限
	CycleDetected,             // 循环引用
	StackLimitReached,         // 堆叠上限
	ContainerFull,             // 容器已满
	TypeMismatch,              // 类型不匹配
	SwapPerformed,             // 执行了交换
	ContainerRejected          // 容器拒绝（放入失败）
};

/**
 * 菜单项配置
 * 定义单个右键菜单项的显示和行为
 */
USTRUCT(BlueprintType)
struct FItemContextMenuItem
{
	GENERATED_USTRUCT_BODY()

public:
	/** 菜单操作类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	EItemContextAction Action = EItemContextAction::Use;

	/** 显示文本（如果为空，使用默认文本） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	FText DisplayText;

	/** 图标（可选） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	bool bEnabled = true;

	/** 自定义事件名（当Action为Custom时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu")
	FName CustomEventName;
};

/**
 * 添加物品操作的结果
 */
USTRUCT(BlueprintType)
struct FAddItemResult
{
	GENERATED_USTRUCT_BODY()

public:
	/** 结果 */
	UPROPERTY(BlueprintReadOnly, Category = "Add Result")
	EMoveItemResult ResultType = EMoveItemResult::Failed;

	/** 详细错误信息 */
	UPROPERTY(BlueprintReadOnly, Category = "Add Result")
	FString ErrorMessage;

	/** 实际添加到的槽位ID */
	UPROPERTY(BlueprintReadOnly, Category = "Add Result")
	int32 SlotID = INDEX_NONE;

	/** 构造函数 */
	FAddItemResult() = default;
	
	explicit FAddItemResult(EMoveItemResult InResult, const FString& InErrorMessage = TEXT(""), int32 InSlotID = INDEX_NONE)
		: ResultType(InResult)
		, ErrorMessage(InErrorMessage)
		, SlotID(InSlotID)
	{}

	/** 创建成功结果 */
	static FAddItemResult Success(int32 InSlotID)
	{
		return FAddItemResult(EMoveItemResult::Success, TEXT(""), InSlotID);
	}

	/** 创建失败结果 */
	static FAddItemResult Failure(const FString& InErrorMessage)
	{
		return FAddItemResult(EMoveItemResult::Failed, InErrorMessage, INDEX_NONE);
	}

	bool IsSuccess() const
	{
		return ResultType == EMoveItemResult::Success;
	}
};

/** 移动物品结果 */
USTRUCT(BlueprintType)
struct FMoveItemResult
{
	GENERATED_USTRUCT_BODY()

public:
	/** 移动结果类型 */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	EMoveItemResult Result = EMoveItemResult::Failed;

	/** 实际移动的数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	int32 MovedQuantity = 0;

	/** 剩余未移动的数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	int32 RemainingQuantity = 0;

	/** 详细错误信息 */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	FString ErrorMessage;

	/** 新创建的物品UID（堆叠时） */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	FGuid NewItemUID;

	/** 是否执行了交换 */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	bool bWasSwapped = false;

	/** 被交换的物品UID */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	FGuid SwappedItemUID;

	/** 是否移动到了容器内部 */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	bool bMovedToContainer = false;

	/** 实际移动到的容器UID */
	UPROPERTY(BlueprintReadOnly, Category = "Move Result")
	FGuid TargetContainerUID;

public:
	FMoveItemResult()
		: Result(EMoveItemResult::Failed)
		, MovedQuantity(0)
		, RemainingQuantity(0)
		, ErrorMessage(TEXT(""))
		, NewItemUID()
		, bWasSwapped(false)
		, SwappedItemUID()
		, bMovedToContainer(false)
		, TargetContainerUID()
	{
	}

	/** 是否成功（完全成功或部分成功） */
	bool IsSuccess() const
	{
		return Result == EMoveItemResult::Success || Result == EMoveItemResult::PartialSuccess || Result == EMoveItemResult::SwapPerformed;
	}

	/** 是否完全成功 */
	bool IsCompleteSuccess() const
	{
		return Result == EMoveItemResult::Success || Result == EMoveItemResult::SwapPerformed;
	}
};

/**
 * 物品定义
 * 定义物品的静态属性，存储在DataRegistry中
 * 用于描述"什么是木材"、"什么是背包"
 */
USTRUCT(BlueprintType)
struct FGaiaItemDefinition : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	/** 物品名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	FText ItemName;

	/** 物品描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	FText ItemDescription;

	/** 物品图标 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Info")
	TSoftObjectPtr<UTexture2D> ItemIcon;

	/** 物品重量（单位：重量点数） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (ClampMin = "0"))
	int32 ItemWeight = 1;

	/** 物品体积（单位：体积点数） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (ClampMin = "1"))
	int32 ItemVolume = 1;

	/** 是否可堆叠 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	bool bStackable = true;

	/** 最大堆叠数量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (EditCondition = "bStackable", ClampMin = "1"))
	int32 MaxStackSize = 99;

	/** 物品标签（用于分类和过滤） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	FGameplayTagContainer ItemTags;

	/** 是否有容器功能 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	bool bHasContainer = false;

	/** 容器定义ID（如果有容器） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (EditCondition = "bHasContainer"))
	FName ContainerDefinitionID = NAME_None;

	/** 右键菜单类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EItemContextMenuType ContextMenuType = EItemContextMenuType::Material;

	/** 自定义菜单项（仅当ContextMenuType为Custom时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (EditCondition = "ContextMenuType == EItemContextMenuType::Custom"))
	TArray<FItemContextMenuItem> CustomMenuItems;

public:
	FGaiaItemDefinition()
	{
	}
	
	/**
	 * 检查此物品定义是否允许堆叠
	 * 注意：即使此函数返回true，物品实例如果包含容器，仍然不可堆叠
	 * @return 是否可堆叠
	 */
	bool IsStackable() const
	{
		// 规则1：如果物品定义本身就不可堆叠，直接返回false
		if (!bStackable)
		{
			return false;
		}
		
		// 规则2：如果物品定义声明了容器功能，强制不可堆叠
		// 原因：带容器的物品每个都有独立的存储空间，不应该堆叠
		if (bHasContainer)
		{
			return false;
		}
		
		// 其他情况，可以堆叠
		return true;
	}
};

/**
 * 容器定义
 * 定义容器的静态属性，存储在DataRegistry中
 * 用于描述"什么是一个背包"、"什么是一个箱子"
 */
USTRUCT(BlueprintType)
struct FGaiaContainerDefinition : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	/** 容器名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Info")
	FText ContainerName;

	/** 容器描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Info")
	FText ContainerDescription;

	/** 容器图标 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Info")
	TSoftObjectPtr<UTexture2D> ContainerIcon;

	/** 槽位数量（固定） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Capacity", meta = (ClampMin = "1", ClampMax = "100"))
	int32 SlotCount = 20;

	/** 是否启用体积限制 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Capacity")
	bool bEnableVolumeLimit = false;

	/** 最大容积（单位：体积点数，如果启用体积限制） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Capacity", meta = (EditCondition = "bEnableVolumeLimit", ClampMin = "0"))
	int32 MaxVolume = 100;

	/** 允许存放的物品标签（为空表示允许所有物品） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Filter")
	FGameplayTagContainer AllowedItemTags;

	/** 是否允许嵌套容器 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container Filter")
	bool bAllowNestedContainers = false;

public:
	FGaiaContainerDefinition()
	{
	}

	/** 检查是否允许指定标签的物品 */
	bool HasAllowedTag(const FGameplayTag& InItemTag) const
	{
		// 如果没有设置任何标签，不允许任何物品
		if (AllowedItemTags.IsEmpty())
		{
			return false;
		}
		// 检查物品标签是否匹配
		return AllowedItemTags.HasTag(InItemTag);
	}

	/** 检查是否允许指定标签容器中的任一标签 */
	bool HasAnyAllowedTag(const FGameplayTagContainer& InItemTags) const
	{
		// 如果没有设置任何标签，不允许任何物品
		if (AllowedItemTags.IsEmpty())
		{
			return false;
		}
		// 检查是否有任何匹配的标签
		return AllowedItemTags.HasAny(InItemTags);
	}
};

/**
 * 槽位信息
 * 描述容器中一个槽位的状态
 */
USTRUCT(BlueprintType)
struct FGaiaSlotInfo
{
	GENERATED_USTRUCT_BODY()

public:
	/** 槽位ID */
	UPROPERTY(BlueprintReadWrite, Category = "Slot Info")
	int32 SlotID = INDEX_NONE;

	/** 此槽位的物品实例UID */
	UPROPERTY(BlueprintReadWrite, Category = "Slot Info")
	FGuid ItemInstanceUID;

public:
	FGaiaSlotInfo()
		: SlotID(INDEX_NONE)
		, ItemInstanceUID()
	{
	}

	FGaiaSlotInfo(int32 InSlotID)
		: SlotID(InSlotID)
		, ItemInstanceUID()
	{
	}

	/** 槽位是否为空 */
	bool IsEmpty() const
	{
		return !ItemInstanceUID.IsValid();
	}
};

/**
 * 物品实例
 * 运行时物品数据，代表具体的一个或一堆物品
 */
USTRUCT(BlueprintType)
struct FGaiaItemInstance
{
	GENERATED_USTRUCT_BODY()

public:
	/** 物品实例唯一ID（全局唯一标识符） */
	UPROPERTY(BlueprintReadOnly, Category = "Item Instance")
	FGuid InstanceUID;

	/** 物品定义ID（引用FGaiaItemDefinition） */
	UPROPERTY(BlueprintReadWrite, Category = "Item Instance")
	FName ItemDefinitionID = NAME_None;

	/** 数量（可堆叠物品） */
	UPROPERTY(BlueprintReadWrite, Category = "Item Instance")
	int32 Quantity = 1;

	/** 拥有的容器UID（如果这个物品有容器功能） */
	UPROPERTY(BlueprintReadWrite, Category = "Item Instance")
	FGuid OwnedContainerUID;

	/** 当前所在容器UID（无效表示游离状态：掉落、装备等） */
	UPROPERTY(BlueprintReadWrite, Category = "Item Location")
	FGuid CurrentContainerUID;

	/** 当前所在槽位ID（-1 表示未分配槽位） */
	UPROPERTY(BlueprintReadWrite, Category = "Item Location")
	int32 CurrentSlotID = -1;

	/** 调试显示名称（用于日志输出） */
	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FString DebugDisplayName;

public:
	FGaiaItemInstance()
		: InstanceUID()
		, ItemDefinitionID(NAME_None)
		, Quantity(1)
		, OwnedContainerUID()
		, CurrentContainerUID()
		, CurrentSlotID(-1)
		, DebugDisplayName(TEXT(""))
	{
	}

	/** 是否有容器 */
	bool HasContainer() const
	{
		return OwnedContainerUID.IsValid();
	}

	/** 是否有效 */
	bool IsValid() const
	{
		return InstanceUID.IsValid() && ItemDefinitionID != NAME_None && Quantity > 0;
	}

	/** 是否在容器中 */
	bool IsInContainer() const
	{
		return CurrentContainerUID.IsValid();
	}

	/** 是否游离状态（掉落、装备等） */
	bool IsOrphan() const
	{
		return !CurrentContainerUID.IsValid();
	}

	/** 获取简短的调试名称 */
	FString GetDebugName() const
	{
		if (!DebugDisplayName.IsEmpty())
		{
			return FString::Printf(TEXT("%s(x%d)"), *DebugDisplayName, Quantity);
		}
		return FString::Printf(TEXT("%s(x%d)"), *ItemDefinitionID.ToString(), Quantity);
	}

	/** 获取简短的UID（前8位） */
	FString GetShortUID() const
	{
		return InstanceUID.ToString().Left(8);
	}
};

/**
 * 容器实例
 * 运行时容器数据，管理槽位
 * 注意：物品数据存储在全局AllItems池中，容器只保存槽位引用
 */
USTRUCT(BlueprintType)
struct FGaiaContainerInstance
{
	GENERATED_USTRUCT_BODY()

public:
	/** 容器唯一ID（全局唯一标识符） */
	UPROPERTY(BlueprintReadOnly, Category = "Container Instance")
	FGuid ContainerUID;

	/** 容器定义ID（引用FGaiaContainerDefinition） */
	UPROPERTY(BlueprintReadWrite, Category = "Container Instance")
	FName ContainerDefinitionID = NAME_None;

	/** 拥有此容器的物品UID（无效表示不属于任何物品，如玩家基础背包） */
	UPROPERTY(BlueprintReadWrite, Category = "Container Instance")
	FGuid OwnerItemUID;

	/** 父容器UID（如果这个容器嵌套在另一个容器中，无效表示顶层容器） */
	UPROPERTY(BlueprintReadWrite, Category = "Container Instance")
	FGuid ParentContainerUID;

	/** 槽位信息列表（只保存引用，不保存物品数据） */
	UPROPERTY(BlueprintReadWrite, Category = "Container Instance")
	TArray<FGaiaSlotInfo> Slots;

	/** 缓存的总重量（优化用） */
	UPROPERTY(BlueprintReadOnly, Category = "Container Instance")
	int32 CachedTotalWeight = 0;

	/** 缓存的总体积（优化用） */
	UPROPERTY(BlueprintReadOnly, Category = "Container Instance")
	int32 CachedTotalVolume = 0;

	/** 是否需要重新计算缓存 */
	UPROPERTY(BlueprintReadOnly, Category = "Container Instance")
	bool bNeedRecalculate = true;

	/** 调试显示名称（用于日志输出） */
	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FString DebugDisplayName;

public:
	FGaiaContainerInstance()
		: ContainerUID()
		, ContainerDefinitionID(NAME_None)
		, OwnerItemUID()
		, ParentContainerUID()
		, CachedTotalWeight(0)
		, CachedTotalVolume(0)
		, bNeedRecalculate(true)
		, DebugDisplayName(TEXT(""))
	{
	}

	/** 标记需要重新计算 */
	void MarkDirty()
	{
		bNeedRecalculate = true;
	}

	/** 获取槽位在数组中的索引 */
	int32 GetSlotIndexByID(int32 SlotID) const
	{
		for (int32 i = 0; i < Slots.Num(); i++)
		{
			if (Slots[i].SlotID == SlotID)
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	/** 查找第一个空槽位 */
	int32 FindEmptySlotID() const
	{
		for (const FGaiaSlotInfo& Slot : Slots)
		{
			if (Slot.IsEmpty())
			{
				return Slot.SlotID;
			}
		}
		return INDEX_NONE;
	}

	/** 获取已使用的槽位数量 */
	int32 GetUsedSlotCount() const
	{
		int32 Count = 0;
		for (const FGaiaSlotInfo& Slot : Slots)
		{
			if (!Slot.IsEmpty())
			{
				Count++;
			}
		}
		return Count;
	}

	/** 获取简短的调试名称 */
	FString GetDebugName() const
	{
		if (!DebugDisplayName.IsEmpty())
		{
			return DebugDisplayName;
		}
		return ContainerDefinitionID.ToString();
	}

	/** 获取简短的UID（前8位） */
	FString GetShortUID() const
	{
		return ContainerUID.ToString().Left(8);
	}
};

// ========================================
// UI相关枚举和结构
// ========================================

/** UI层级类型 */
UENUM(BlueprintType)
enum class EInventoryUILayer : uint8
{
	/** 玩家背包（基础层） */
	PlayerBackpack UMETA(DisplayName = "PlayerBackpack"),
	
	/** 容器窗口（可多个） */
	ContainerWindow UMETA(DisplayName = "ContainerWindow"),
	
	/** 右键菜单 */
	ContextMenu UMETA(DisplayName = "ContextMenu"),
	
	/** 数量输入对话框 */
	QuantityDialog UMETA(DisplayName = "QuantityDialog"),
	
	/** 确认对话框 */
	ConfirmDialog UMETA(DisplayName = "ConfirmDialog")
};

/** UI栈元素 */
USTRUCT(BlueprintType)
struct FInventoryUIStackElement
{
	GENERATED_BODY()
	
	/** UI层级类型 */
	UPROPERTY(BlueprintReadOnly)
	EInventoryUILayer LayerType = EInventoryUILayer::ContainerWindow;
	
	/** Widget引用 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> Widget = nullptr;
	
	/** 关联的容器UID（如果是容器窗口） */
	UPROPERTY(BlueprintReadOnly)
	FGuid ContainerUID;
	
	/** 打开时间戳（用于排序） */
	UPROPERTY(BlueprintReadOnly)
	float OpenTimestamp = 0.0f;
};

/** 容器窗口信息 */
USTRUCT(BlueprintType)
struct FContainerWindowInfo
{
	GENERATED_BODY()
	
	/** 容器UID */
	UPROPERTY(BlueprintReadWrite)
	FGuid ContainerUID;
	
	/** 窗口Widget */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UGaiaContainerWindowWidget> WindowWidget = nullptr;
	
	/** 窗口位置 */
	UPROPERTY(BlueprintReadWrite)
	FVector2D Position = FVector2D::ZeroVector;
	
	/** 窗口大小 */
	UPROPERTY(BlueprintReadWrite)
	FVector2D Size = FVector2D::ZeroVector;
	
	/** Z-Order（越大越在上面） */
	UPROPERTY(BlueprintReadWrite)
	int32 ZOrder = 0;
};

/** 容器UI调试信息 */
USTRUCT(BlueprintType)
struct FContainerUIDebugInfo
{
	GENERATED_BODY()
	
	/** 容器UID */
	UPROPERTY(BlueprintReadOnly)
	FGuid ContainerUID;
	
	/** 容器定义ID */
	UPROPERTY(BlueprintReadOnly)
	FName ContainerDefID = NAME_None;
	
	/** 槽位使用情况 */
	UPROPERTY(BlueprintReadOnly)
	FString SlotUsage;
	
	/** 重量信息 */
	UPROPERTY(BlueprintReadOnly)
	FString WeightInfo;
	
	/** 体积信息 */
	UPROPERTY(BlueprintReadOnly)
	FString VolumeInfo;
	
	/** 物品列表（调试用） */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> ItemList;
};
