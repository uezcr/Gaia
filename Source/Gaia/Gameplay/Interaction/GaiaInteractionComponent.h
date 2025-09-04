#pragma once

#include "Components/ActorComponent.h"
#include "GaiaInteractionComponent.generated.h"

#define UE_API GAIAGAME_API

UCLASS(ClassGroup=(Gaia), meta=(BlueprintSpawnableComponent))
class UGaiaInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UE_API UGaiaInteractionComponent();

protected:
	UE_API virtual void BeginPlay() override;

public:

private:
	UE_API void SetupInteraction();
	UFUNCTION(BlueprintCallable, Category = "GaiaInteractionComponent")
	UE_API void Trace();
	UE_API AActor* GetDirectHitActor(const FVector& InTraceStart, const FVector& InTraceEnd) const;

protected:
	const float InteractionInterval = 0.025f;
	FTimerHandle InteractionTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GaiaInteractionComponent")
	float TraceDistance = 500.f;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaInteractionComponent")
	AActor* CurrentSelectedActor = nullptr;
};
#undef UE_API
