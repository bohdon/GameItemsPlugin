// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewModels/PlayerGameItemContainerViewModelResolver.h"

#include "DemoInteractionItemContainerViewModelResolver.generated.h"


/**
 * Resolves a game item container view model from the owning player's current interaction context.
 */
UCLASS()
class GAMEITEMSPLUGIN_API UDemoInteractionItemContainerViewModelResolver : public UPlayerGameItemContainerViewModelResolver
{
	GENERATED_BODY()

public:
	virtual AActor* GetContainerActor(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
};
