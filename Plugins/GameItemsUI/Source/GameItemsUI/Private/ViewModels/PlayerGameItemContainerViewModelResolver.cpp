// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/PlayerGameItemContainerViewModelResolver.h"

#include "GameItemSettings.h"
#include "GameItemsUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "ViewModels/GameItemContainerViewModel.h"


UPlayerGameItemContainerViewModelResolver::UPlayerGameItemContainerViewModelResolver()
{
	ContainerId = UGameItemSettings::GetDefaultContainerId();
}

UObject* UPlayerGameItemContainerViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* GameItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	AActor* OwningActor = GetContainerActor(ExpectedType, UserWidget, View);
	return GameItemsUISubsystem->GetOrCreateContainerViewModelForActor(OwningActor, ContainerId);
}

AActor* UPlayerGameItemContainerViewModelResolver::GetContainerActor(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UserWidget->GetOwningPlayer();
}
