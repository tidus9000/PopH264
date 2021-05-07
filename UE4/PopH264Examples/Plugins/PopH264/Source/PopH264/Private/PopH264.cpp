// Copyright Epic Games, Inc. All Rights Reserved.

#include "PopH264.h"
#include <popH264/popH264-1.3.38/include/popH264/PopH264.h>
#include "ImportPopH264dll.h"
#define LOCTEXT_NAMESPACE "FPopH264Module"

void FPopH264Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	ImportPopH264dll pop;
	int32_t Version = pop.PopH264_GetVersion();

	UE_LOG(LogTemp, Warning, TEXT("PopH264dll loaded. Version %i"), Version);
}

void FPopH264Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPopH264Module, PopH264)