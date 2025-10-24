#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "GaiaInventoryTypes.generated.h"

/**
 * 移动物品操作的结果类型
 */
UENUM(BlueprintType)
enum class EMoveItemResult : uint8
{
	Success,                    // 完全成功
	PartialSuccess,            // 部分成功（部分堆叠）
	Failed,                    // 完全失败
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
 * 移动物品操作的结果
 */
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

public:
	FGaiaItemDefinition()
	{
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

public:
	FGaiaItemInstance()
		: InstanceUID()
		, ItemDefinitionID(NAME_None)
		, Quantity(1)
		, OwnedContainerUID()
		, CurrentContainerUID()
		, CurrentSlotID(-1)
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

public:
	FGaiaContainerInstance()
		: ContainerUID()
		, ContainerDefinitionID(NAME_None)
		, OwnerItemUID()
		, ParentContainerUID()
		, CachedTotalWeight(0)
		, CachedTotalVolume(0)
		, bNeedRecalculate(true)
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
};
