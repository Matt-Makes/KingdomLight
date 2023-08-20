// Copyright Epic Games, Inc. All Rights Reserved.


#include "CaptureTexture2D.h"

#define LOCTEXT_NAMESPACE "FCaptureTexture2DModule"

void FCaptureTexture2DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCaptureTexture2DModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCaptureTexture2DModule, CaptureTexture2D)