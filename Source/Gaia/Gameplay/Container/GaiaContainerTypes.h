#pragma once
#include "GameplayTagContainer.h"

#include "GaiaContainerTypes.generated.h"

USTRUCT(BlueprintType)
struct FGaiaSlotAddress
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "GaiaSlotAddress")
	int32 ContainerUID;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaSlotAddress")
	int32 SlotID;

	FGaiaSlotAddress()
	{
		ContainerUID = INDEX_NONE;
		SlotID = INDEX_NONE;
	}
	FGaiaSlotAddress(const int32& InContainerUID, const int32& InSlotID)
	{
		ContainerUID = InContainerUID;
		SlotID = InSlotID;
	}
	bool operator == (const FGaiaSlotAddress& InOther) const
	{
		return ContainerUID == InOther.ContainerUID && SlotID == InOther.SlotID;
	}
};

USTRUCT(BlueprintType)
struct FGaiaSlotInfo
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "GaiaSlotAddress")
	int32 SlotID;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaSlotAddress")
	int32 ItemSlotID;
};

USTRUCT(BlueprintType)
struct FGaiaItemDefault : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	FText ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	FText ItemDescription;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	TSoftObjectPtr<UTexture2D> ItemIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	FGameplayTag ItemTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	bool bStackable;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	int32 MaxStackSize;

	int32 GetMaxStackSize() const
	{
		return bStackable ? MaxStackSize : 1;
	}
};

USTRUCT(BlueprintType)
struct FGaiaItemInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	FName ItemID;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	FGaiaSlotAddress ItemAddress;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	int32 ItemAmount;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemInfo")
	int32 OwnContainerUID;

	FGaiaItemInfo()
	{
		ItemID = NAME_None;
		ItemAddress = FGaiaSlotAddress();
		ItemAmount = 0;
		OwnContainerUID = INDEX_NONE;
	}
};

USTRUCT(BlueprintType)
struct FGaiaContainerDefault : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	FText ContainerName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	FText ItemDescription;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	TSoftObjectPtr<UTexture2D> ContainerIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GaiaItemDefaults")
	FGameplayTagContainer ContainerTags;
	
	bool HasTag(const FGameplayTag& InTag) const
	{
		return ContainerTags.HasTag(InTag);
	}
};

USTRUCT(BlueprintType)
struct FGaiaContainerInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerInfo")
	int32 ContainerUID;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerInfo")
	FName ContainerID;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerInfo")
	TArray<FGaiaItemInfo> Items;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerInfo")
	TArray<FGaiaSlotInfo> Slots;

	FGaiaContainerInfo()
	{
		ContainerUID = INDEX_NONE;
		ContainerID = NAME_None;
	}
};
