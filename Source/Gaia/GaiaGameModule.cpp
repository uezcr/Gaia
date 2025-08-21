#include "Modules/ModuleManager.h"

class FGaiaGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FGaiaGameModule, GaiaGame, "GaiaGame");
