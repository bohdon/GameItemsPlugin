// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Fragments/GameItemFragment_Usage.h"
#include "Templates/SubclassOf.h"
#include "GameItemFragment_UsageAbility.generated.h"

class UGameplayAbility;


/**
 * Activates an ability when this item is "used".
 * The ability must be granted in advance by equipment or other means.
 */
UCLASS(DisplayName = "Usage Ability")
class GAMEPLAYABILITYITEMS_API UGameItemFragment_UsageAbility : public UGameItemFragment_Usage
{
	GENERATED_BODY()

public:
	/** The abilities to activate by tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer AbilityTags;

	/** The ability to activate by class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", Meta = (AllowAbstract = false))
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	virtual bool Use_Implementation(UGameItem* Item, AActor* Instigator) const override;
};
