#include "GaiaPlayerCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Input/GaiaInputConfig.h"
#include "GaiaGamePlayTags.h"
#include "Player/GaiaLocalPlayer.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GaiaPlayerCharacter)

AGaiaPlayerCharacter::AGaiaPlayerCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void AGaiaPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGaiaPlayerCharacter::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);
	const UGaiaLocalPlayer* LP = Cast<UGaiaLocalPlayer>(PC->GetLocalPlayer());
	check(LP);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	Subsystem->ClearAllMappings();
	if (const UGaiaInputConfig* InputConfig = DefaultInputConfig)
	{
		if (UInputMappingContext* IMC = DefaultInputMapping.InputMapping.LoadSynchronous())
		{
			if (DefaultInputMapping.bRegisterWithSettings)
			{
				if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
				{
					Settings->RegisterInputMappingContext(IMC);
				}
							
				FModifyContextOptions Options = {};
				Options.bIgnoreAllPressedKeysUntilRelease = false;
				// Actually add the config to the local player
				Subsystem->AddMappingContext(IMC, DefaultInputMapping.Priority, Options);
			}
		}
		UGaiaInputComponent* GaiaIC = Cast<UGaiaInputComponent>(PlayerInputComponent);
		if (ensureMsgf(GaiaIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to ULyraInputComponent or a subclass of it.")))
		{
			// Add the key mappings that may have been set by the player
			GaiaIC->AddInputMappings(InputConfig, Subsystem);
			GaiaIC->BindNativeAction(InputConfig, GaiaGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
			GaiaIC->BindNativeAction(InputConfig, GaiaGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
		}
	}
}

void AGaiaPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}
		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void AGaiaPlayerCharacter::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X);
	}
	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(Value.Y);
	}
}

void AGaiaPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGaiaPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InitializePlayerInput(PlayerInputComponent);
}

