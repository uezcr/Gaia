#include "GaiaItemSpawner.h"

#include "GaiaContainerLibrary.h"
#include "GaiaContainerSubSystem.h"
#include "GaiaItemBase.h"
#include "IDetailTreeNode.h"
#include "Kismet/GameplayStatics.h"

AGaiaItemSpawner::AGaiaItemSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	Scene = CreateDefaultSubobject<USceneComponent>("Scene");
	SetRootComponent(Scene);
	SceneMesh = CreateDefaultSubobject<UStaticMeshComponent>("SceneMesh");
	SceneMesh->SetupAttachment(Scene);
}

void AGaiaItemSpawner::SpawnItem()
{
	if (FGaiaItemConfig* LoadConfig = SpawnItemRow.GetRow<FGaiaItemConfig>(TEXT("Context")))
	{
		FGaiaItemConfig SpawnConfig = *LoadConfig;
		FGaiaItemInfo NewItemInfo = FGaiaItemInfo(SpawnConfig.ItemName);
		UGameInstance* GameInstance = GetGameInstance();
		if (UGaiaContainerSubSystem* ContainerSubSystem = GameInstance->GetSubsystem<UGaiaContainerSubSystem>())
		{
			if (ContainerSubSystem->RequestNewItem(NewItemInfo))
			{
				if (UClass* SpawnClass = SpawnConfig.ItemClass.LoadSynchronous())
				{
					if (AGaiaItemBase* SpawnItem = GetWorld()->SpawnActorDeferred<AGaiaItemBase>(SpawnClass,GetActorTransform()))
					{
						SpawnItem->NativePreInitializeItem(NewItemInfo);
						FGaiaContainerInfo FGaiaContainerInfo;
						UGameplayStatics::FinishSpawningActor(SpawnItem,GetActorTransform());
					}
				}
			}
		}
	}
	Destroy();
}

void AGaiaItemSpawner::BeginPlay()
{
	Super::BeginPlay();
	SpawnItem();
}

