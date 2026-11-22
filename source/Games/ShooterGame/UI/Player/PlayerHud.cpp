#include "PlayerHud.hpp"

#include <EngineMain.h>
#include <UI/UiSystem.h>
#include <UI/Menus/UiMenuStack.h>

#include "../../Entities/Player/Player.hpp"

#include <algorithm>

namespace
{
	// Adds `element` to the viewport right below the menu stack, so pause and
	// other pages draw over the HUD.
	void AddBelowMenus(const std::shared_ptr<UiElement>& element)
	{
		UiMenuStack& stack = UiMenus::Stack();
		auto& children = EngineMain::Viewport.children;
		auto it = std::find_if(children.begin(), children.end(),
			[&](const std::shared_ptr<UiElement>& child) { return child.get() == &stack; });

		if (it == children.end())
		{
			EngineMain::Viewport.AddChild(element);
			return;
		}

		element->parent = &EngineMain::Viewport;
		children.insert(it, element);
		EngineMain::Viewport.UpdateChildrenOffsetRecursive();
	}
}

PlayerHud::PlayerHud()
{
}

PlayerHud::~PlayerHud()
{
	if (view)
	{
		EngineMain::Viewport.RemoveChild(view);
		UiSystem::UnregisterRoot(view);
	}

	if (crosshairCanvas)
		EngineMain::Viewport.RemoveChild(crosshairCanvas);

	if (ScreenControls)
		EngineMain::Viewport.RemoveChild(ScreenControls);

	view.reset();
	crosshairCanvas.reset();
	ScreenControls.reset();
}

void PlayerHud::Init(Player* playerRef)
{
	player = playerRef;

	ScreenControls = make_shared<ScreenMobileControls>();
	AddBelowMenus(ScreenControls);

	view = std::make_shared<PlayerHudView>();
	UiSystem::RegisterRoot(view);
	AddBelowMenus(view);

	// The crosshair stays a C++ element (a dot in the centre).
	crosshairCanvas = make_shared<UiCanvas>();
	crosshair = make_shared<UiCrosshair>();
	crosshair->origin = vec2(0.5f, 0.5f);
	crosshairCanvas->AddChild(crosshair);
	AddBelowMenus(crosshairCanvas);
	view->crosshair = crosshair;
}

void PlayerHud::SetVisible(bool visible)
{
	if (view) view->visible = visible;
	if (crosshairCanvas) crosshairCanvas->visible = visible;
	if (ScreenControls) ScreenControls->visible = visible;
}

void PlayerHud::Update()
{
	AdoptReloadedView();

	if (view)
		view->Refresh(player);
}

// UI hot reload swaps a fresh PlayerHudView into the viewport and drops ours.
// Take the fresh one over (back under the menus, with the crosshair); it sets
// everything up again from the player on its first Refresh.
void PlayerHud::AdoptReloadedView()
{
	PlayerHudView* latest = PlayerHudView::Latest();
	if (latest == nullptr || latest == view.get())
		return;

	const auto& children = EngineMain::Viewport.children;
	auto attached = [&](const UiElement* element)
		{
			return std::any_of(children.begin(), children.end(),
				[&](const std::shared_ptr<UiElement>& child) { return child.get() == element; });
		};

	// Ours still on screen: the newer view belongs to another HUD (two players during a level change).
	if (view && attached(view.get()))
		return;
	if (!attached(latest))
		return;

	auto fresh = std::static_pointer_cast<PlayerHudView>(latest->shared_from_this());
	EngineMain::Viewport.RemoveChild(fresh);
	AddBelowMenus(fresh);
	fresh->visible = view ? view->visible : true;
	fresh->crosshair = crosshair;
	view = fresh;
}
