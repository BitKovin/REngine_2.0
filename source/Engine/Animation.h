#pragma once
#include "SkeletalMesh.hpp"

class Animation : public SkeletalMesh
{
public:

	Animation(Entity* owner) : SkeletalMesh(owner)
	{
		skipMeshLoad = true;
	}

	bool IsInFrustrum(Frustum frustrum)	{return false;};

	virtual bool isVisible(){return false;}

	bool IsCameraVisible(){return false;}

	bool IsShadowVisible(){return false;}

	void DrawForward(mat4x4 view, mat4x4 projection){}

	void DrawDepth(mat4x4 view, mat4x4 projection) {}

	void DrawShadow(mat4x4 view, mat4x4 projection){}

private:

};
