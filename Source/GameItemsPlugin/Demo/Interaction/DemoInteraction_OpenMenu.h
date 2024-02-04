// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DemoInteraction.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "DemoInteraction_OpenMenu.generated.h"

class UCommonActivatableWidget;
struct FGameplayTag;


/**
 * An interaction to open a menu.
 */
UCLASS(Abstract)
class GAMEITEMSPLUGIN_API UDemoInteraction_OpenMenu : public UDemoInteraction
{
	GENERATED_BODY()

public:
	UDemoInteraction_OpenMenu(const FObjectInitializer& ObjectInitializer);

	/** The menu to display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCommonActivatableWidget> MenuClass;

	/** The UI layer to add the menu to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag MenuLayerTag;

	virtual bool Trigger_Implementation(APawn* Interactor) override;
};
