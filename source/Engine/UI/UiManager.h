#pragma once
class UiManager
{
public:
	
	static inline float UiScale = 1.0f;

	static inline float GetScaledUiHeight()
	{
		return 1080 / UiScale;
	}

private:

};

