// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "UIMainLayout.generated.h"

class UCommonActivatableWidgetStack;


/**
 * Main widget added to the player hud which contains the structure for all UI layers.
 */
UCLASS(Abstract)
class GAMEITEMSPLUGIN_API UUIMainLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Register a widget stack as a new ui layer. */
	UFUNCTION(BlueprintCallable)
	void RegisterUILayer(FGameplayTag LayerTag, UCommonActivatableWidgetStack* Stack);

	/** Get a widget stack layer by tag. */
	UFUNCTION(BlueprintPure, Meta = (GameplayTagFilter = "UI.Layer"))
	UCommonActivatableWidgetStack* GetUILayer(FGameplayTag LayerTag) const;

protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetStack>> UILayers;
};
