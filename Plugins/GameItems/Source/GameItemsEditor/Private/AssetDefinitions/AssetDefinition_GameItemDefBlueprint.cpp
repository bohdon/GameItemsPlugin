// Copyright Bohdon Sayre, All Rights Reserved.


#include "AssetDefinitions/AssetDefinition_GameItemDefBlueprint.h"

#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "GameItemDef.h"
#include "GameItemDefBlueprintFactory.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "ToolMenus.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/MessageDialog.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText UAssetDefinition_GameItemDefBlueprint::GetAssetDisplayName() const
{
	return LOCTEXT("AssetTypeActions_GameItemDefBlueprint", "Game Item Blueprint");
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameItemDefBlueprint::GetAssetCategories() const
{
	static const auto Categories = {EAssetCategoryPaths::Gameplay};
	return Categories;
}

UFactory* UAssetDefinition_GameItemDefBlueprint::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameItemDefBlueprintFactory* Factory = NewObject<UGameItemDefBlueprintFactory>();
	Factory->ParentClass = TSubclassOf<UGameItemDef>(*InBlueprint->GeneratedClass);
	return Factory;
}


// Menu Extensions
//--------------------------------------------------------------------

namespace MenuExtension_GameItemDefBlueprint
{
	void ExecuteNewChildBlueprint(const FToolMenuContext& InContext, const FAssetData InItemDefAsset)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		if (const UGameItemDefBlueprint* ParentItemDef = Cast<UGameItemDefBlueprint>(InItemDefAsset.GetAsset()))
		{
			if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentItemDef->GeneratedClass))
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidClassToMakeBlueprintFrom", "Invalid class with which to make a Blueprint."));
				return;
			}

			FString Name;
			FString PackageName;
			IAssetTools::Get().CreateUniqueAssetName(ParentItemDef->GetOutermost()->GetName(), TEXT("_Child"), PackageName, Name);
			const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

			UGameItemDefBlueprintFactory* AnimBlueprintFactory = NewObject<UGameItemDefBlueprintFactory>();
			AnimBlueprintFactory->ParentClass = TSubclassOf<UGameItemDef>(*ParentItemDef->GeneratedClass);

			ContentBrowserModule.Get().CreateNewAsset(Name, PackagePath, ParentItemDef->GetClass(), AnimBlueprintFactory);
		}
	}

	static FDelayedAutoRegisterHelper DelayedAutoRegister(EDelayedRegisterRunPhase::EndOfEngineInit, [] {
			UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
				{
					FToolMenuOwnerScoped OwnerScoped(UE_MODULE_NAME);
					UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UGameItemDefBlueprint::StaticClass());

					FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
					Section.AddDynamicEntry(NAME_None, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
						{
							if (const UContentBrowserAssetContextMenuContext* CBContext = UContentBrowserAssetContextMenuContext::FindContextWithAssets(InSection))
							{
								if (const FAssetData* SelectedItemDefPtr = CBContext->GetSingleSelectedAssetOfType(UGameItemDefBlueprint::StaticClass(), EIncludeSubclasses::No))
								{
									FToolUIAction UIAction;
									UIAction.ExecuteAction = FToolMenuExecuteAction::CreateStatic(&ExecuteNewChildBlueprint, *SelectedItemDefPtr);
									InSection.AddMenuEntry(
										"GameItemDefBlueprint_NewChildItemDef",
										LOCTEXT("GameItemDef_NewChildItemDef", "Create Child Item Def"),
										LOCTEXT("GameItemDef_NewChildItemDef_Tooltip", "Create a new Item Def as a child of this definition"),
										FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Blueprint"),
										UIAction);
								}
							}
						}));
				}));
		});
}


#undef LOCTEXT_NAMESPACE
