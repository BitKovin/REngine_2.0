#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegistry.h"

#include "../Input.h"

class UiButton : public UiElement
{

private:

	bool pendingClick = false;

	Texture* tex = nullptr;

public:

	string ImagePath = "GameData/cat.png";



	std::function<void()>* onClick = nullptr;

	UiButton()
	{ 
	}
	~UiButton()
	{

	}

	void Update()
	{

		UiElement::Update();


		if (hovering) {
			if (Input::GetAction("click")->Pressed())
			{
				if (onClick)
				{
					(*onClick)();
				}
			}
		}
	}

	bool HasPendingClick()
	{
		if (pendingClick)
		{
			pendingClick = false;
			return true;
		}

		return false;
	}

	void Draw()
	{


		if (tex == nullptr)
		{
			tex = AssetRegistry::GetTextureFromFile(ImagePath);
			if(tex->valid == false)
				tex = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
		}

		vec2 pos = position + offset;

		UiRenderer::DrawTexturedRect(pos, size, tex->getID());

		UiElement::Draw();
	}

};