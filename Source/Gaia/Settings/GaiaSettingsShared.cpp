#include "GaiaSettingsShared.h"

#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Culture.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Rendering/SlateRenderer.h"
#include "SubtitleDisplaySubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "Player/GaiaLocalPlayer.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GaiaSettingsShared)

static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

namespace GaiaSettingsSharedCVars
{
	static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone(
		TEXT("gpad.DefaultLeftStickInnerDeadZone"),
		DefaultGamepadLeftStickInnerDeadZone,
		TEXT("Gamepad left stick inner deadzone")
	);

	static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone(
		TEXT("gpad.DefaultRightStickInnerDeadZone"),
		DefaultGamepadRightStickInnerDeadZone,
		TEXT("Gamepad right stick inner deadzone")
	);	
}

UGaiaSettingsShared::UGaiaSettingsShared()
{
	FInternationalization::Get().OnCultureChanged().AddUObject(this, &ThisClass::OnCultureChanged);

	GamepadMoveStickDeadZone = GaiaSettingsSharedCVars::DefaultGamepadLeftStickInnerDeadZone;
	GamepadLookStickDeadZone = GaiaSettingsSharedCVars::DefaultGamepadRightStickInnerDeadZone;
}

int32 UGaiaSettingsShared::GetLatestDataVersion() const
{
	// 0 = before subclassing ULocalPlayerSaveGame
	// 1 = first proper version
	return 1;
}

UGaiaSettingsShared* UGaiaSettingsShared::CreateTemporarySettings(const UGaiaLocalPlayer* LocalPlayer)
{
	// This is not loaded from disk but should be set up to save
	UGaiaSettingsShared* SharedSettings = Cast<UGaiaSettingsShared>(CreateNewSaveGameForLocalPlayer(UGaiaSettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

UGaiaSettingsShared* UGaiaSettingsShared::LoadOrCreateSettings(const UGaiaLocalPlayer* LocalPlayer)
{
	// This will stall the main thread while it loads
	UGaiaSettingsShared* SharedSettings = Cast<UGaiaSettingsShared>(LoadOrCreateSaveGameForLocalPlayer(UGaiaSettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

bool UGaiaSettingsShared::AsyncLoadOrCreateSettings(const UGaiaLocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate)
{
	FOnLocalPlayerSaveGameLoadedNative Lambda = FOnLocalPlayerSaveGameLoadedNative::CreateLambda([Delegate]
		(ULocalPlayerSaveGame* LoadedSave)
		{
			UGaiaSettingsShared* LoadedSettings = CastChecked<UGaiaSettingsShared>(LoadedSave);
			
			LoadedSettings->ApplySettings();

			Delegate.ExecuteIfBound(LoadedSettings);
		});

	return ULocalPlayerSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer(UGaiaSettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME, Lambda);
}

void UGaiaSettingsShared::SaveSettings()
{
	// Schedule an async save because it's okay if it fails
	AsyncSaveGameToSlotForLocalPlayer();

	// TODO_BH: Move this to the serialize function instead with a bumped version number
	if (UEnhancedInputLocalPlayerSubsystem* System = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPlayer))
	{
		if (UEnhancedInputUserSettings* InputSettings = System->GetUserSettings())
		{
			InputSettings->AsyncSaveSettings();
		}
	}
}

void UGaiaSettingsShared::ApplySettings()
{
	ApplySubtitleOptions();
	ApplyBackgroundAudioSettings();
	ApplyCultureSettings();

	if (UEnhancedInputLocalPlayerSubsystem* System = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPlayer))
	{
		if (UEnhancedInputUserSettings* InputSettings = System->GetUserSettings())
		{
			InputSettings->ApplySettings();
		}
	}
}

void UGaiaSettingsShared::SetColorBlindStrength(int32 InColorBlindStrength)
{
	InColorBlindStrength = FMath::Clamp(InColorBlindStrength, 0, 10);
	if (ColorBlindStrength != InColorBlindStrength)
	{
		ColorBlindStrength = InColorBlindStrength;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency)(int32)ColorBlindMode, (int32)ColorBlindStrength, true, false);
	}
}

int32 UGaiaSettingsShared::GetColorBlindStrength() const
{
	return ColorBlindStrength;
}

void UGaiaSettingsShared::SetColorBlindMode(EColorBlindMode InMode)
{
	if (ColorBlindMode != InMode)
	{
		ColorBlindMode = InMode;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
			(EColorVisionDeficiency)(int32)ColorBlindMode, (int32)ColorBlindStrength, true, false);
	}
}

EColorBlindMode UGaiaSettingsShared::GetColorBlindMode() const
{
	return ColorBlindMode;
}

void UGaiaSettingsShared::ApplySubtitleOptions()
{
	if (USubtitleDisplaySubsystem* SubtitleSystem = USubtitleDisplaySubsystem::Get(OwningPlayer))
	{
		FSubtitleFormat SubtitleFormat;
		SubtitleFormat.SubtitleTextSize = SubtitleTextSize;
		SubtitleFormat.SubtitleTextColor = SubtitleTextColor;
		SubtitleFormat.SubtitleTextBorder = SubtitleTextBorder;
		SubtitleFormat.SubtitleBackgroundOpacity = SubtitleBackgroundOpacity;

		SubtitleSystem->SetSubtitleDisplayOptions(SubtitleFormat);
	}
}

//////////////////////////////////////////////////////////////////////

void UGaiaSettingsShared::SetAllowAudioInBackgroundSetting(EGameAllowBackgroundAudioSetting NewValue)
{
	if (ChangeValueAndDirty(AllowAudioInBackground, NewValue))
	{
		ApplyBackgroundAudioSettings();
	}
}

void UGaiaSettingsShared::ApplyBackgroundAudioSettings()
{
	if (OwningPlayer && OwningPlayer->IsPrimaryPlayer())
	{
		FApp::SetUnfocusedVolumeMultiplier((AllowAudioInBackground != EGameAllowBackgroundAudioSetting::Off) ? 1.0f : 0.0f);
	}
}

//////////////////////////////////////////////////////////////////////

void UGaiaSettingsShared::ApplyCultureSettings()
{
	if (bResetToDefaultCulture)
	{
		const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
		check(SystemDefaultCulture.IsValid());

		const FString CultureToApply = SystemDefaultCulture->GetName();
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Clear this string
			GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		bResetToDefaultCulture = false;
	}
	else if (!PendingCulture.IsEmpty())
	{
		// SetCurrentCulture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a copy of it to work with
		const FString CultureToApply = PendingCulture;
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Note: This is intentionally saved to the users config
			// We need to localize text before the player logs in and very early in the loading screen
			GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *CultureToApply, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		ClearPendingCulture();
	}
}

void UGaiaSettingsShared::ResetCultureToCurrentSettings()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

const FString& UGaiaSettingsShared::GetPendingCulture() const
{
	return PendingCulture;
}

void UGaiaSettingsShared::SetPendingCulture(const FString& NewCulture)
{
	PendingCulture = NewCulture;
	bResetToDefaultCulture = false;
	bIsDirty = true;
}

void UGaiaSettingsShared::OnCultureChanged()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void UGaiaSettingsShared::ClearPendingCulture()
{
	PendingCulture.Reset();
}

bool UGaiaSettingsShared::IsUsingDefaultCulture() const
{
	FString Culture;
	GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);

	return Culture.IsEmpty();
}

void UGaiaSettingsShared::ResetToDefaultCulture()
{
	ClearPendingCulture();
	bResetToDefaultCulture = true;
	bIsDirty = true;
}

//////////////////////////////////////////////////////////////////////

void UGaiaSettingsShared::ApplyInputSensitivity()
{
	
}

