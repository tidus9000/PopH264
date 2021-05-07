// Copyright Epic Games, Inc. All Rights Reserved.

#include "PopH264.h"
#include <popH264/popH264-1.3.38/include/popH264/PopH264.h>
#define LOCTEXT_NAMESPACE "FPopH264Module"

void FPopH264Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FPopH264Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPopH264Module, PopH264)