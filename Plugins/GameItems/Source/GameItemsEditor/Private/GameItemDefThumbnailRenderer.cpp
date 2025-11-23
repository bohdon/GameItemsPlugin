// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemDefThumbnailRenderer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "GameItemDef.h"
#include "GlobalRenderResources.h"
#include "TextureResource.h"
#include "Engine/Blueprint.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Slate/WidgetRenderer.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Widgets/Images/SImage.h"


bool UGameItemDefThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	const UBlueprint* Blueprint = Cast<UBlueprint>(Object);
	if (Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(UGameItemDef::StaticClass()))
	{
		if (UGameItemDef* ItemDefCDO = Blueprint->GeneratedClass->GetDefaultObject<UGameItemDef>())
		{
			const TOptional<FSlateBrush> Brush = ItemDefCDO->GetEditorIcon();
			return Brush.IsSet();
		}
	}
	return false;
}

void UGameItemDefThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas,
                                         bool bAdditionalViewFamily)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);

	const bool bIsBlueprintValid = IsValid(Blueprint) &&
		IsValid(Blueprint->GeneratedClass) &&
		Blueprint->bHasBeenRegenerated &&
		!Blueprint->bBeingCompiled &&
		!Blueprint->HasAnyFlags(RF_Transient);

	if (!bIsBlueprintValid)
	{
		return;
	}

	FLinearColor BGColor(FColor(45, 42, 39));
	FCanvasTileItem BGTile(FVector2D(X, Y), GWhiteTexture, FVector2D(Width, Height), FVector2D::ZeroVector, FVector2D::UnitVector, BGColor);
	BGTile.Draw(Canvas);

	const UGameItemDef* ItemDefCDO = Blueprint->GeneratedClass->GetDefaultObject<UGameItemDef>();
	const TOptional<FSlateBrush> Brush = ItemDefCDO->GetEditorIcon();
	if (!Brush.IsSet())
	{
		return;
	}

	// a much-less accurate version of SlateBrushThumbnailRenderer

	UTexture2D* Texture = Cast<UTexture2D>(Brush->GetResourceObject());
	UMaterialInterface* Material = Cast<UMaterialInterface>(Brush->GetResourceObject());

	if (Texture)
	{
		FCanvasTileItem TextureTile(FVector2D(X, Y), Texture->GetResource(), FLinearColor::White);
		TextureTile.BlendMode = SE_BLEND_AlphaBlend;
		TextureTile.Draw(Canvas);
	}
	else if (Material)
	{
		const UMaterial* Mat = Material->GetMaterial();
		if (Mat && Mat->IsUIMaterial())
		{
			switch (Brush->DrawAs)
			{
			case ESlateBrushDrawType::Image:
			case ESlateBrushDrawType::Border:
			case ESlateBrushDrawType::Box:
			case ESlateBrushDrawType::NoDrawType:
			case ESlateBrushDrawType::RoundedBox:
				{
					DrawBrush(Width, Height, *Brush, RenderTarget);
				}
				break;
			default:

				check(false);
			}
		}
	}
}

void UGameItemDefThumbnailRenderer::DrawBrush(uint32 Width, uint32 Height, FSlateBrush Brush, FRenderTarget* RenderTarget)
{
	// copied from USlateBrushThumbnailRenderer::CreateThumbnailAsImage

	FWidgetRenderer* WidgetRenderer = new FWidgetRenderer(true, true);

	UTexture2D* CheckerboardTexture = UThumbnailManager::Get().CheckerboardTexture;
	FSlateBrush CheckerboardBrush;
	CheckerboardBrush.SetResourceObject(CheckerboardTexture);
	CheckerboardBrush.ImageSize = FVector2D(CheckerboardTexture->GetSizeX(), CheckerboardTexture->GetSizeY());
	CheckerboardBrush.Tiling = ESlateBrushTileType::Both;

	TSharedRef<SWidget> Thumbnail = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&Brush)
		];

	WidgetRenderer->DrawWidget(RenderTarget, Thumbnail, FVector2D(Width, Height), 0.f);

	if (WidgetRenderer)
	{
		BeginCleanup(WidgetRenderer);
		WidgetRenderer = nullptr;
	}
}
