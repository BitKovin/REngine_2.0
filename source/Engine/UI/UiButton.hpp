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

	vec4 Color = vec4(1);

	std::function<void()>* onClick = nullptr;

	UiButton()
	{ 
		HitCheck = true;
	}
	~UiButton()
	{

	}

	void Update()
	{

		UiElement::Update();

		for (const auto& touch : TouchEvents)
		{
			if (touch.pressed)
			{
				if (onClick)
				{
					(*onClick)();
				}
			}
		}

		/*
		if (hovering) {
			if (Input::GetAction("click")->Pressed())
			{
				if (onClick)
				{
					(*onClick)();
				}
			}
		}
		*/
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

		vec2 pos = finalizedPosition + finalizedOffset;

		UiRenderer::DrawTexturedRect(pos, finalizedSize, tex->getID(), Color);

		UiElement::Draw();
	}

};