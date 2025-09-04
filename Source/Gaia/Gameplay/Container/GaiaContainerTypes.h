#pragma once
#include "GameplayTagContainer.h"

#include "GaiaContainerTypes.generated.h"

class AGaiaItemBase;
//物品稀有度.
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Gray        = 0  UMETA(DisplayName = "垃圾"),
	White       = 1  UMETA(DisplayName = "寻常"),
	Blue        = 2  UMETA(DisplayName = "稀有"),
	Green       = 3  UMETA(DisplayName = "罕见"),
	Orange      = 4  UMETA(DisplayName = "宝物"),
	LightRed    = 5  UMETA(DisplayName = "宝物+"),
	Pink        = 6  UMETA(DisplayName = "史诗"),
	LightPurple = 7  UMETA(DisplayName = "史诗+"),
	Lime        = 8  UMETA(DisplayName = "特殊"),
	Yellow      = 9  UMETA(DisplayName = "传说"),
	Cyan        = 10 UMETA(DisplayName = "传说+"),
	Red         = 11 UMETA(DisplayName = "神话"),
	Purple      = 12 UMETA(DisplayName = "神话+"),
	Amber       = 13 UMETA(DisplayName = "任务物品"),
	Rainbow     = 14 UMETA(DisplayName = "专家"),
	FieryRed    = 15 UMETA(DisplayName = "纪念品")
};

USTRUCT(BlueprintType)
struct FGaiaItemConfig : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	FName ItemName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	FText ItemDescription;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	EItemRarity ItemRarity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	FGameplayTagContainer ItemTags;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	float ItemVolume;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	float ItemWeight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	bool bHasContainer = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem", meta = (EditCondition = "bHasContainer"))
	FDataTableRowHandle ContainerRow;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	TSoftClassPtr<AGaiaItemBase> ItemClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	TSoftObjectPtr<UStaticMesh> SceneMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaItem")
	int32 MaximumStack = 1;
};

USTRUCT(BlueprintType)
struct FGaiaItemInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	int64 ItemUID;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemInfo")
	FName ItemName;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	int64 ContainerUID;

	FGaiaItemInfo()
	{
		ItemUID = INDEX_NONE;
		ItemName = NAME_None;
		ContainerUID = INDEX_NONE;
	}

	FGaiaItemInfo(const FName& InItemName)
	{
		ItemName = InItemName;
		ItemUID = INDEX_NONE;
		ContainerUID = INDEX_NONE;
	}

	void Reset()
	{
		ItemUID = INDEX_NONE;
		ContainerUID = INDEX_NONE;
	}
};

USTRUCT(BlueprintType)
struct FGaiaContainerConfig : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaContainer")
	FName ContainerName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaContainer")
	FText Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaContainer", DisplayName = "可容纳物品类型", meta=(ShortTooltip = "可容纳物品类型"))
	FGameplayTagContainer ContainerTags;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaContainer", DisplayName = "格子数量", meta=(ShortTooltip = "格子数量,用于限制携带上限和UI显示"))
	FIntPoint GridNums;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaContainer", DisplayName = "容量", meta=(ShortTooltip = "容量"))
	float Capacity;

	bool operator > (const FGameplayTagContainer& ObjectTags) const
	{
		return ContainerTags.HasAnyExact(ObjectTags);
	}

	bool operator > (const FGaiaItemConfig& ItemConfig) const
	{
		return ContainerTags.HasAnyExact(ItemConfig.ItemTags);
	}
};

USTRUCT(BlueprintType)
struct FGaiaContainerInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	int64 ContainerUID;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemInfo")
	FName ContainerName;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	TArray<int32> ItemUIDs;
	
	FGaiaContainerInfo& operator = (const FGaiaContainerInfo& Other)
	{
		ItemUIDs = Other.ItemUIDs;
		return *this;
	}
};