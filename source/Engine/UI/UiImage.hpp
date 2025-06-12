#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegistry.h"

class UiImage : public UiElement
{

private:

public:

	string ImagePath = "GameData/cat.png";

	vec4 Color = vec4(1);

	UiImage()
	{ 
		//tex = AssetRegistry::GetTextureFromFile("GameData/cat.png");
	}
	~UiImage()
	{

	}

	void Draw()
	{

		vec2 pos = finalizedPosition + finalizedOffset;

		if (tex == nullptr)
		{
			tex = AssetRegistry::GetTextureFromFile(ImagePath);
			if (tex->valid == false)
				tex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
		}

		UiRenderer::DrawTexturedRect(pos, finalizedSize, tex->getID(), Color);

		UiElement::Draw();
	}

private:

	Texture* tex = nullptr;

};