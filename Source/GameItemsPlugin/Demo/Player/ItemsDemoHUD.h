// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "ItemsDemoHUD.generated.h"

class UUIMainLayout;
class UUserWidget;
class UCommonActivatableWidgetStack;


/**
 * Customized HUD for the demo project.
 */
UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** The widget class to use as the main UI layout. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UUIMainLayout> MainLayoutClass;

	/** Extend the debug actor list to support selecting any actor with gameplay items. */
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;

	/** Return a UI layer from the main layout. */
	UFUNCTION(BlueprintPure, Meta = (GameplayTagFilter = "UI.Layer"))
	UCommonActivatableWidgetStack* GetUILayer(FGameplayTag LayerTag) const;

	virtual void BeginPlay() override;

	void CreateMainLayout();

protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	UUIMainLayout* MainLayout;
};
