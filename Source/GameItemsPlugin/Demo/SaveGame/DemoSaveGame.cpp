// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoSaveGame.h"


UDemoSaveGame::UDemoSaveGame()
{
	SavedDataVersion = static_cast<uint32>(EDemoSaveGameVersion::LatestVersion);
}

void UDemoSaveGame::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	constexpr uint32 LatestVersion = static_cast<uint32>(EDemoSaveGameVersion::LatestVersion);
	if (Ar.IsLoading() && SavedDataVersion != LatestVersion)
	{
		// handle save game upgrades

		SavedDataVersion = LatestVersion;
	}
}
