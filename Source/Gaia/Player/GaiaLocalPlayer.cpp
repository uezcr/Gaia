#include "GaiaLocalPlayer.h"

#include "Settings/GaiaSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GaiaLocalPlayer)

UGaiaSettingsShared* UGaiaLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		// On PC it's okay to use the sync load because it only checks the disk
		// This could use a platform tag to check for proper save support instead
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
		
		if (bCanLoadBeforeLogin)
		{
			SharedSettings = UGaiaSettingsShared::LoadOrCreateSettings(this);
		}
		else
		{
			// We need to wait for user login to get the real settings so return temp ones
			SharedSettings = UGaiaSettingsShared::CreateTemporarySettings(this);
		}
	}

	return SharedSettings;
}
