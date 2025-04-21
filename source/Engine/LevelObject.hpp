#pragma once

#include "EObject.hpp"

#include "IDrawMesh.h"

#include <vector>

using namespace std;

class LevelObject : public EObject
{
public:

	bool Static = false;

	LevelObject(){}
	~LevelObject(){}

	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void UpdatePhysics() {}

	virtual void AsyncUpdate(){}

	virtual void Start() {}

	virtual void UpdateDebugUI() {};

	virtual void FinalLevelRemove() {};

	virtual void Finalize()
	{

	}

	virtual void LoadAssetsIfNeeded(){}

	virtual vector<IDrawMesh*> GetDrawMeshes() { return vector<IDrawMesh*>(); }


private:

};