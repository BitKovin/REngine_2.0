#include "UiTestTerminal.h"

#include "UiTestRunner.h"

#include <Camera.h>
#include <EngineMain.h>
#include <Entity.h>
#include <Input.h>
#include <LoadingScreen/LoadingScreenSystem.h>
#include <RegisterLevelObject.h>
#include <UI/Hud/UiElemWorldPanel.h>
#include <UI/Hud/UiHost.h>
#include <UI/Hud/UiTooltips.h>
#include <UI/SceneView/UiElemSceneView.h>
#include <UI/UiSystem.h>
#include <UI/WorldSpace/UiWorldSurface.h>
#include <algorithm>

#include <cstdlib>
#include <sstream>

class UiTestTerminalEntity;
static UiTestTerminalEntity* g_terminal = nullptr;

class UiTestTerminalEntity : public Entity
{
public:
    UiWorldSurface* Surface = nullptr;
    int Hits = 0, Hits2 = 0;

    void LoadAssets() override
    {
        Surface = new UiWorldSurface(this);
        Surface->TwoSided = true;
        Surface->PixelsPerMeter = 512.f;
        Surface->Position = Position;
        Surface->Rotation = Rotation;
        Surface->SetContent("GameData/ui/kit/tests/p5_terminal.xml");
        Surface->On("Hit", [this] { ++Hits; });
        Surface->On("Hit2", [this] { ++Hits2; });
        Drawables.push_back(Surface);
        g_terminal = this;
    }

    void Update() override
    {
        if (Surface)
        {
            Surface->Position = Position;
            Surface->Rotation = Rotation;
        }
    }

    void Destroy() override
    {
        if (g_terminal == this)
            g_terminal = nullptr;
        Entity::Destroy();
    }
};

REGISTER_ENTITY(UiTestTerminalEntity, "ui_test_terminal")

namespace
{
    bool Compare(double actual, const std::string& op, double want, const std::string& what, std::string& error)
    {
        bool pass = false;
        if (op == "==") pass = actual == want;
        else if (op == "!=") pass = actual != want;
        else if (op == ">") pass = actual > want;
        else if (op == ">=") pass = actual >= want;
        else if (op == "<") pass = actual < want;
        else if (op == "<=") pass = actual <= want;
        if (!pass)
            error = "expect_terminal " + what + ": " + std::to_string(actual) + " " + op + " " + std::to_string(want) + " is false";
        return pass;
    }

    bool SpawnTerminal(const std::string& args, std::string& error)
    {
        const float distance = args.empty() ? 1.5f : static_cast<float>(std::atof(args.c_str()));
        Entity* e = Entity::Spawn("ui_test_terminal");
        if (!e) { error = "ui_test_terminal is not registered"; return false; }
        e->Position = Camera::position + Camera::Forward() * distance;
        e->Rotation = Camera::rotation;   // faces the camera, like a billboard
        return true;
    }

    // mouse_center [fx fy]: the (simulated) mouse at a fraction of the screen (default the middle); with a free cursor the aim ray goes through the mouse
    bool MouseCenter(const std::string& args, std::string&)
    {
        float fx = 0.5f, fy = 0.5f;
        std::istringstream in(args);
        in >> fx >> fy;
        TouchEvent t;
        t.id = 1;
        t.position = glm::vec2(EngineMain::MainInstance->ScreenSize) * glm::vec2(fx, fy);
        Input::SimulatedMouse = t;
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // loading_test <layout> <progress>: a markup loading screen; the first call sets the page, each call draws one loading frame
    bool LoadingTest(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string path;
        float progress = 0.f;
        in >> path >> progress;
        static std::string s_path;
        if (s_path != path)
        {
            s_path = path;
            LoadingScreenSystem::SetLoadingScreen(path);
        }
        LoadingScreenSystem::Progress = progress;
        LoadingScreenSystem::Update(progress);
        UiTestRunner::WaitFrames(2);
        return true;
    }

    // worldpanel_at <id> <forward> <right> <up>: a WorldPanel follows the point that far from the camera (metres, along the camera's axes)
    bool WorldPanelAt(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string id;
        float f = 0, r = 0, u = 0;
        in >> id >> f >> r >> u;
        auto* panel = dynamic_cast<UiElemWorldPanel*>(UiSystem::FindById(id));
        if (!panel) { error = "no WorldPanel '" + id + "'"; return false; }
        panel->SetWorldPosition(Camera::position + Camera::Forward() * f + Camera::Right() * r + Camera::Up() * u);
        return true;
    }

    // expect_worldpanel <id> centre_dist|edge_dist|clamped|behind <op> <value>
    bool ExpectWorldPanel(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string id, field, op, value;
        in >> id >> field >> op >> value;
        auto* panel = dynamic_cast<UiElemWorldPanel*>(UiSystem::FindById(id));
        if (!panel) { error = "no WorldPanel '" + id + "'"; return false; }
        const glm::vec2 view = EngineMain::Viewport.GetSize();
        const glm::vec2 centre = glm::vec2(panel->AbsoluteTransform * glm::vec3(panel->BorderRect.W * 0.5f, panel->BorderRect.H * 0.5f, 1.f));
        double actual = 0;
        if (field == "centre_dist") actual = glm::length(centre - view * 0.5f);
        else if (field == "edge_dist") actual = std::min(std::min(centre.x, view.x - centre.x), std::min(centre.y, view.y - centre.y));
        else if (field == "clamped") actual = panel->IsClamped() ? 1 : 0;
        else if (field == "behind") actual = panel->IsBehind() ? 1 : 0;
        else if (field == "hidden") actual = panel->Computed().Visibility != UiVisibility::Visible ? 1 : 0;
        else { error = "unknown field '" + field + "'"; return false; }
        return Compare(actual, op, std::atof(value.c_str()), field, error);
    }

    bool WorldPanelClamp(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string id, value;
        in >> id >> value;
        UiElem* panel = UiSystem::FindById(id);
        if (!panel) { error = "no element '" + id + "'"; return false; }
        panel->SetAttribute("clamp-to-edge", value == "0" ? "false" : "true");
        return true;
    }

    bool HudAdd(const std::string& args, std::string& error) { if (!UiHost::AddHud(args)) { error = "cannot load " + args; return false; } return true; }
    bool OverlayAdd(const std::string& args, std::string& error) { if (!UiHost::AddOverlay(args)) { error = "cannot load " + args; return false; } return true; }

    // expect_scene <id> renders|total_renders|yaw|dragging|target_w|target_h <op> <value>   (renders: since the last check of that field)
    bool ExpectScene(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string id, field, op, value;
        in >> id >> field >> op >> value;
        auto* view = dynamic_cast<UiElemSceneView*>(UiSystem::FindById(id));
        if (!view) { error = "no SceneView '" + id + "'"; return false; }
        double actual = 0;
        if (field == "renders")
        {
            static int s_last = 0;
            const int now = view->RenderCount();
            actual = now - s_last;
            s_last = now;
        }
        else if (field == "total_renders") actual = UiSceneViews::TotalRenders();
        else if (field == "yaw") actual = view->OrbitYaw();
        else if (field == "dragging") actual = view->Dragging() ? 1 : 0;
        else if (field == "target_w") actual = view->TargetSize().x;
        else if (field == "target_h") actual = view->TargetSize().y;
        else { error = "unknown field '" + field + "'"; return false; }
        return Compare(actual, op, std::atof(value.c_str()), field, error);
    }

    bool ExpectTooltip(const std::string& args, std::string& error)
    {
        const bool want = args != "0";
        if (UiTooltips::IsShowing() != want) { error = std::string("expect_tooltip: a tooltip is ") + (UiTooltips::IsShowing() ? "showing" : "not showing"); return false; }
        return true;
    }

    bool NeedTerminal(std::string& error)
    {
        if (g_terminal && g_terminal->Surface) return true;
        error = "no terminal (spawn_terminal first)";
        return false;
    }

    bool TerminalHover(const std::string& args, std::string& error)
    {
        if (!NeedTerminal(error)) return false;
        std::istringstream in(args);
        float x = 0, y = 0;
        in >> x >> y;
        g_terminal->Surface->Hover(x, y);
        return true;
    }

    // terminal_ray <kx> [ky]: a world ray from the camera, kx of the camera's right and ky of its up vector off the forward direction
    bool TerminalRay(const std::string& args, std::string& error)
    {
        if (!NeedTerminal(error)) return false;
        float kx = 0.f, ky = 0.f;
        std::istringstream in(args);
        in >> kx >> ky;
        g_terminal->Surface->HoverRay(Camera::position, glm::normalize(Camera::Forward() + Camera::Right() * kx + Camera::Up() * ky));
        return true;
    }

    bool TerminalUnhover(const std::string&, std::string& error) { if (!NeedTerminal(error)) return false; g_terminal->Surface->Unhover(); return true; }
    bool TerminalClick(const std::string&, std::string& error) { if (!NeedTerminal(error)) return false; g_terminal->Surface->Click(); return true; }

    bool TerminalFocus(const std::string& args, std::string& error)
    {
        if (!NeedTerminal(error)) return false;
        if (args == "enter") g_terminal->Surface->EnterFocus();
        else if (args == "exit") g_terminal->Surface->ExitFocus();
        else { error = "usage: terminal_focus enter|exit"; return false; }
        return true;
    }

    bool ExpectTerminal(const std::string& args, std::string& error)
    {
        std::istringstream in(args);
        std::string field, op, value;
        in >> field >> op >> value;
        if (field.empty() || op.empty() || value.empty()) { error = "usage: expect_terminal <field> <op> <value>"; return false; }
        if (field == "renders")   // surface re-renders since the last check of this field
        {
            static int s_last = 0;
            const int now = UiWorldSurface::RenderCount();
            const int delta = now - s_last;
            s_last = now;
            return Compare(delta, op, std::atof(value.c_str()), field, error);
        }
        if (!NeedTerminal(error)) return false;
        UiWorldSurface& s = *g_terminal->Surface;
        double actual = 0;
        if (field == "hits") actual = g_terminal->Hits;
        else if (field == "hits2") actual = g_terminal->Hits2;
        else if (field == "aimed") actual = UiWorldSurface::Aimed() == &s ? 1 : 0;
        else if (field == "interactive") actual = UiWorldSurface::IsAimingAtInteractive() ? 1 : 0;
        else if (field == "pointer_x") actual = s.PointerPixel().x;
        else if (field == "pointer_y") actual = s.PointerPixel().y;
        else if (field == "focused") actual = s.IsFocused() ? 1 : 0;
        else if (field == "out_of_range") actual = s.Content() && (s.Content()->States & kStateOutOfRange) ? 1 : 0;
        else { error = "unknown field '" + field + "'"; return false; }
        return Compare(actual, op, std::atof(value.c_str()), field, error);
    }
}

void UiTestTerminal::RegisterCommands()
{
    UiTestRunner::RegisterCommand("spawn_terminal", SpawnTerminal);
    UiTestRunner::RegisterCommand("loading_test", LoadingTest);
    UiTestRunner::RegisterCommand("worldpanel_at", WorldPanelAt);
    UiTestRunner::RegisterCommand("expect_worldpanel", ExpectWorldPanel);
    UiTestRunner::RegisterCommand("expect_scene", ExpectScene);
    UiTestRunner::RegisterCommand("expect_tooltip", ExpectTooltip);
    UiTestRunner::RegisterCommand("worldpanel_clamp", WorldPanelClamp);
    UiTestRunner::RegisterCommand("hud_add", HudAdd);
    UiTestRunner::RegisterCommand("overlay_add", OverlayAdd);
    UiTestRunner::RegisterCommand("mouse_center", MouseCenter);
    UiTestRunner::RegisterCommand("terminal_hover", TerminalHover);
    UiTestRunner::RegisterCommand("terminal_ray", TerminalRay);
    UiTestRunner::RegisterCommand("terminal_unhover", TerminalUnhover);
    UiTestRunner::RegisterCommand("terminal_click", TerminalClick);
    UiTestRunner::RegisterCommand("terminal_focus", TerminalFocus);
    UiTestRunner::RegisterCommand("expect_terminal", ExpectTerminal);
}
