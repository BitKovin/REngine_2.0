#pragma once
#include <Entity.hpp>
#include <BSP/Quake3Bsp.h>

class TestBsp : public Entity
{
public:
	TestBsp();
	~TestBsp();

private:

	CQuake3BSP* bsp;

};



TestBsp::TestBsp()
{
	bsp = new CQuake3BSP();
	bsp->LoadBSP("GameData/Maps/test.bsp");
	bsp->BuildVBO();
	bsp->GenerateTexture();
	bsp->GenerateLightmap();


	Drawables.push_back(bsp);
}

TestBsp::~TestBsp()
{
}