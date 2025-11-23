// Copyright Bohdon Sayre, All Rights Reserved.

#include "GameItemsEditorModule.h"

#include "GameItemDefBlueprint.h"
#include "GameItemDefThumbnailRenderer.h"
#include "Modules/ModuleManager.h"
#include "ThumbnailRendering/ThumbnailManager.h"


DEFINE_LOG_CATEGORY(LogGameItemsEditor);

class FGameItemsEditorModule : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FGameItemsEditorModule, GameItemsEditor)


void FGameItemsEditorModule::StartupModule()
{
	UThumbnailManager::Get().RegisterCustomRenderer(UGameItemDefBlueprint::StaticClass(), UGameItemDefThumbnailRenderer::StaticClass());
}

void FGameItemsEditorModule::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UGameItemDefBlueprint::StaticClass());
	}
}
