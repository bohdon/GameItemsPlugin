// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerInterface.h"
#include "Components/ActorComponent.h"
#include "GameItemContainerComponent.generated.h"

class UGameItemContainerDef;


/**
 * Component that provides any number of game item containers.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemContainerComponent : public UActorComponent,
                                                  public IGameItemContainerInterface
{
	GENERATED_BODY()

public:
	UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The default containers to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, TSubclassOf<UGameItemContainerDef>> DefaultContainers;

	/**
	 * Create a new item container.
	 * @return The new container, or null if a container already exists with the same id.
	 */
	UFUNCTION(BlueprintCallable, Meta = (DeterminesOutputType = "ContainerClass"))
	UGameItemContainer* CreateContainer(FGameplayTag ContainerId, TSubclassOf<UGameItemContainerDef> ContainerDef = nullptr);

	virtual void InitializeComponent() override;

	// UActorComponent
	virtual void ReadyForReplication() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UGameItemContainer*> Containers;

	void CreateDefaultContainers();

	void AddContainer(UGameItemContainer* Container);
};
