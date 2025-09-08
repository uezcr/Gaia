#include "GaiaInteractionComponent.h"

#include "GaiaInteractableInterface.h"
#include "Character/GaiaCharacter.h"
#include "Gameplay/GaiaCollisionChannels.h"
#include "Kismet/GameplayStatics.h"

UGaiaInteractionComponent::UGaiaInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGaiaInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupInteraction();
}

bool UGaiaInteractionComponent::TryInteract()
{
	FGaiaInteractionContext Context;
	Context.Instigator = GetOwner();
	return false;
}

void UGaiaInteractionComponent::SetupInteraction()
{
	AGaiaCharacter* Character = Cast<AGaiaCharacter>(GetOwner());
	if (Character == nullptr) return;
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC == nullptr) return;
	if (PC->IsLocalController())
	{
		GetWorld()->GetTimerManager().SetTimer(InteractionTimer,this,&UGaiaInteractionComponent::Trace,InteractionInterval,true);
	}
}

void UGaiaInteractionComponent::Trace()
{
	AGaiaCharacter* Character = Cast<AGaiaCharacter>(GetOwner());
	if (Character == nullptr) return;
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC == nullptr) return;
	FVector CameraLoc;
	FRotator CameraRot;
	PC->GetPlayerViewPoint(CameraLoc,CameraRot);
	FVector CameraDir = CameraRot.Vector().GetSafeNormal();
	FVector FocalLoc = CameraLoc + (CameraDir * TraceDistance);
	AActor* HitActor = GetDirectHitActor(CameraLoc,FocalLoc);
	if (CurrentSelectedActor != HitActor)
	{
		if (HitActor != nullptr)
		{
			if (CurrentSelectedActor != nullptr)
			{
				IGaiaInteractableInterface::Execute_EndSelected(CurrentSelectedActor);
				CurrentSelectedActor = HitActor;
				IGaiaInteractableInterface::Execute_Selected(CurrentSelectedActor);
			}
			else
			{
				CurrentSelectedActor = HitActor;
				IGaiaInteractableInterface::Execute_Selected(CurrentSelectedActor);
			}
		}
		else
		{
			if (CurrentSelectedActor != nullptr)
			{
				IGaiaInteractableInterface::Execute_EndSelected(CurrentSelectedActor);
				CurrentSelectedActor = nullptr;
			}
			else
			{
				CurrentSelectedActor = nullptr;
			}
		}
	}
}

AActor* UGaiaInteractionComponent::GetDirectHitActor(const FVector& InTraceStart, const FVector& InTraceEnd) const
{
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult,InTraceStart,InTraceEnd, ECC_Visibility))
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (HitActor->Implements<UGaiaInteractableInterface>())
			{
				return HitActor;
			}
		}
	}
	return nullptr;
}

