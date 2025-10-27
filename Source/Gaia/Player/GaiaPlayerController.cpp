#include "GaiaPlayerController.h"
#include "Gameplay/Inventory/GaiaInventoryRPCComponent.h"
#include "GaiaLogChannels.h"

AGaiaPlayerController::AGaiaPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 创建库存RPC组件
	InventoryRPCComponent = CreateDefaultSubobject<UGaiaInventoryRPCComponent>(TEXT("InventoryRPCComponent"));
}

void AGaiaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 验证组件是否创建成功
	if (InventoryRPCComponent)
	{
		UE_LOG(LogGaia, Log, TEXT("PlayerController [%s] 库存RPC组件初始化成功"), *GetName());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("PlayerController [%s] 库存RPC组件初始化失败！"), *GetName());
	}
}
