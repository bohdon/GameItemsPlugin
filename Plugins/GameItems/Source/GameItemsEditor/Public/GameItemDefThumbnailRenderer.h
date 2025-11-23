// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "GameItemDefThumbnailRenderer.generated.h"

struct FSlateBrush;


UCLASS()
class GAMEITEMSEDITOR_API UGameItemDefThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height,
	                  FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;

	/** Draw a slate brush to a render target. */
	virtual void DrawBrush(uint32 Width, uint32 Height, FSlateBrush Brush, FRenderTarget* RenderTarget);
};
