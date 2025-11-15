// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerGraph.h"

#include "GameItemContainerDef.h"
#include "Rules/GameItemContainerLink.h"


bool FGameItemContainerSpec::IsValid() const
{
	return ContainerDef && ContainerId.IsValid();
}

bool FGameItemContainerLinkSpec::IsValid() const
{
	return ContainerLinkClass && LinkedContainerId.IsValid() && !ContainerQuery.IsEmpty();
}

UGameItemContainerGraph::UGameItemContainerGraph()
{
}
