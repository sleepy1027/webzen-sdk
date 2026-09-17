#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

// Deliberately empty beyond IModuleInterface: all behavior lives on
// UWebzenSDKSubsystem (WebzenSDKSubsystem.h), which is what Blueprints and
// game C++ actually call. This class only exists because Unreal requires
// every plugin to have a module entry point.
class FWebzenSDKModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
