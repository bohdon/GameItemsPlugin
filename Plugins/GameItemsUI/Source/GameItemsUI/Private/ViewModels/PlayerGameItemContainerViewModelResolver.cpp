// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/PlayerGameItemContainerViewModelResolver.h"

#include "GameItemSettings.h"
#include "Blueprint/UserWidget.h"
#include "ViewModels/GameItemContainerViewModel.h"
#include "ViewModels/GameItemsUISubsystem.h"


UPlayerGameItemContainerViewModelResolver::UPlayerGameItemContainerViewModelResolver()
{
	ContainerId = UGameItemSettings::GetDefaultContainerId();
}

UObject* UPlayerGameItemContainerViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* GameItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	APlayerController* OwningPlayer = UserWidget->GetOwningPlayer();
	return GameItemsUISubsystem->GetOrCreateContainerViewModelForActor(OwningPlayer, ContainerId);
}
