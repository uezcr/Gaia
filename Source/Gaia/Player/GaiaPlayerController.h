#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "GaiaPlayerController.generated.h"

class UGaiaInventoryRPCComponent;

/**
 * Gaia 玩家控制器
 * 
 * 包含库存系统的网络同步组件
 */
UCLASS()
class GAIAGAME_API AGaiaPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	AGaiaPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface

public:
	/**
	 * 获取库存RPC组件
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UGaiaInventoryRPCComponent* GetInventoryRPCComponent() const { return InventoryRPCComponent; }

protected:
	/**
	 * 库存系统网络同步组件
	 * 用于客户端和服务器之间的库存操作同步
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UGaiaInventoryRPCComponent> InventoryRPCComponent;
};
