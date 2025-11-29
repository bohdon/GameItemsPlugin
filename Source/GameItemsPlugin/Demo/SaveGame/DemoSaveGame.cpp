// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoSaveGame.h"


int32 UDemoSaveGame::GetLatestDataVersion() const
{
	return static_cast<uint32>(EDemoSaveGameVersion::LatestVersion);
}

void UDemoSaveGame::HandlePostLoad()
{
	if (SavedDataVersion != GetLatestDataVersion())
	{
		// handle save game upgrades
	}

	Super::HandlePostLoad();
}
