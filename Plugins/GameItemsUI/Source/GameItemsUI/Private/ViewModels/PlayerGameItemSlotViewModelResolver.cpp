// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/PlayerGameItemSlotViewModelResolver.h"

#include "GameItemSettings.h"
#include "GameItemsUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "ViewModels/GameItemContainerViewModel.h"
#include "ViewModels/GameItemSlotViewModel.h"


UPlayerGameItemSlotViewModelResolver::UPlayerGameItemSlotViewModelResolver()
{
	ContainerId = UGameItemSettings::GetDefaultContainerId();
	Slot = 0;
}

UObject* UPlayerGameItemSlotViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* GameItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();

	AActor* OwningActor = GetContainerActor(ExpectedType, UserWidget, View);
	const UGameItemContainerViewModel* ContainerViewModel = GameItemsUISubsystem->GetOrCreateContainerViewModelForActor(OwningActor, ContainerId);

	TArray<UGameItemSlotViewModel*> SlotViewModels = ContainerViewModel->GetSlotViewModels();
	if (SlotViewModels.IsValidIndex(Slot))
	{
		return SlotViewModels[Slot];
	}
	return nullptr;
}

AActor* UPlayerGameItemSlotViewModelResolver::GetContainerActor(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return UserWidget->GetOwningPlayer();
}
