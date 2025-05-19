#pragma once

#include "json.hpp"

using namespace nlohmann;

class EObject
{
public:

	void Dispose()
	{

		OnDispose();


	}

	virtual void Serialize(json& target)
	{

	}

	virtual void Deserialize(json& source)
	{

	}

protected:

	virtual void OnDispose()
	{

	}

};