#include "UiHudPreviews.h"

#include <UI/Elements/UiElemLabel.h>
#include <UI/Elements/UiElemPanel.h>
#include <UI/SceneView/UiElemSceneView.h>

namespace
{
    using nlohmann::json;

    UiPreviewScenario Scenario(const char* id, const char* label, json values)
    {
        UiPreviewScenario s;
        s.Id = id;
        s.Label = label;
        s.Values = std::move(values);
        return s;
    }

    UiPreviewParameter Parameter(const char* name, const char* type, json def, const char* help)
    {
        UiPreviewParameter p;
        p.Name = name;
        p.Type = type;
        p.Default = std::move(def);
        p.Help = help;
        return p;
    }

    UiLength Px(float v) { return UiLength{ UiLenKind::Px, v }; }

    // Shared plumbing: both providers rebuild their children from the values.
    class BuiltProvider : public IUiPreviewProvider
    {
    public:
        bool CreatePreview(UiPreviewHost&, const json& values, std::shared_ptr<UiElem>& outRoot, std::string&) override
        {
            auto root = std::make_shared<UiElemPanel>();
            outRoot = root;
            Fill(*root, values);
            return true;
        }
        bool ApplyValues(UiPreviewHost&, UiElem& root, const json& values, std::string&) override
        {
            root.ClearChildren();
            Fill(root, values);
            return true;
        }
        bool ApplyScenario(UiPreviewHost& host, UiElem& root, const std::string& scenarioId, std::string& error) override
        {
            for (const UiPreviewScenario& s : Describe().Scenarios)
                if (s.Id == scenarioId)
                    return ApplyValues(host, root, s.Values, error);
            error = "unknown scenario '" + scenarioId + "'";
            return false;
        }
        void DestroyPreview(UiPreviewHost&, std::shared_ptr<UiElem>& root) override { root.reset(); }

    protected:
        virtual void Fill(UiElem& root, const json& values) = 0;
    };

    // HUD crosshair: four ticks around the centre; `spread` opens them, `hit` shows the hit marker colour.
    class CrosshairPreviewProvider : public BuiltProvider
    {
    public:
        UiPreviewDescriptor Describe() const override
        {
            UiPreviewDescriptor d;
            d.Id = "runtime.engine.hud-crosshair";
            d.Name = "Crosshair (sample HUD)";
            d.Category = "HUD";
            d.Description = "A crosshair of four ticks. Synthetic fixture: spread in pixels and a hit marker; no player is read.";
            d.Kind = "runtime";
            d.ProviderId = "engine.ui.preview.hud-crosshair";
            d.ProviderVersion = 1;
            d.SourceRefs.push_back({ "code", "source/Engine/UI/Hud/UiHudPreviews.cpp" });
            d.HostKind = "hud-layer";
            d.HostLayer = "hud";
            d.RequiredCapabilities = { "markup" };
            UiPreviewParameter spread = Parameter("spread", "float", 6.0, "Distance of the ticks from the centre (px)");
            spread.Min = 0.0; spread.Max = 80.0; spread.Step = 1.0; spread.Unit = "px";
            d.Parameters.push_back(spread);
            d.Parameters.push_back(Parameter("hit", "bool", false, "Hit marker: the ticks turn red"));
            d.Scenarios.push_back(Scenario("idle", "Idle", { { "spread", 6.0 }, { "hit", false } }));
            d.Scenarios.push_back(Scenario("moving", "Moving", { { "spread", 18.0 }, { "hit", false } }));
            d.Scenarios.push_back(Scenario("hip-fire", "Hip-fire spread", { { "spread", 40.0 }, { "hit", false } }));
            d.Scenarios.push_back(Scenario("aiming", "Aiming", { { "spread", 2.0 }, { "hit", false } }));
            d.Scenarios.push_back(Scenario("target-hit", "Hit marker", { { "spread", 6.0 }, { "hit", true } }));
            return d;
        }

    protected:
        void Fill(UiElem& root, const json& values) override
        {
            const float spread = values.value("spread", 6.f);
            const bool hit = values.value("hit", false);
            const glm::vec4 colour = hit ? glm::vec4(1.f, 0.2f, 0.2f, 1.f) : glm::vec4(1.f);
            root.Style().Width = Px(120.f);
            root.Style().Height = Px(120.f);
            struct Tick { float X, Y, W, H; };
            const float c = 60.f;
            const Tick ticks[4] = { { c - 1.f, c - spread - 10.f, 2.f, 10.f }, { c - 1.f, c + spread, 2.f, 10.f },
                                    { c - spread - 10.f, c - 1.f, 10.f, 2.f }, { c + spread, c - 1.f, 10.f, 2.f } };
            for (const Tick& t : ticks)
            {
                auto tick = std::make_shared<UiElemPanel>();
                tick->Style().Width = Px(t.W);
                tick->Style().Height = Px(t.H);
                tick->Style().X = Px(t.X);
                tick->Style().Y = Px(t.Y);
                tick->Style().BackgroundColor = colour;
                root.AddChild(tick);
            }
        }
    };

    // A world-surface page at its pixel size: what a UiWorldSurface shows, without a surface in the world.
    class SurfacePreviewProvider : public BuiltProvider
    {
    public:
        UiPreviewDescriptor Describe() const override
        {
            UiPreviewDescriptor d;
            d.Id = "runtime.engine.world-surface-panel";
            d.Name = "World surface panel (sample)";
            d.Category = "World UI";
            d.Description = "The page of a world surface at its pixel size. Surface settings are declared by the descriptor; no entity is spawned.";
            d.Kind = "world-surface";
            d.ProviderId = "engine.ui.preview.world-surface-panel";
            d.ProviderVersion = 1;
            d.SourceRefs.push_back({ "code", "source/Engine/UI/WorldSpace/UiWorldSurface.cpp" });
            d.HostKind = "world-surface";
            d.RequiredCapabilities = { "markup", "worldSurface" };
            UiPreviewParameter width = Parameter("width", "int", 512, "Surface width (px)");
            width.Min = 64; width.Max = 2048; width.Step = 16;
            UiPreviewParameter height = Parameter("height", "int", 256, "Surface height (px)");
            height.Min = 64; height.Max = 2048; height.Step = 16;
            d.Parameters.push_back(width);
            d.Parameters.push_back(height);
            d.Parameters.push_back(Parameter("title", "string", "Terminal", "Title text"));
            UiPreviewParameter ppm = Parameter("pixelsPerMeter", "float", 512.0, "Pixels per metre of the plane");
            ppm.Min = 64.0; ppm.Max = 2048.0;
            d.Parameters.push_back(ppm);
            UiPreviewParameter facing = Parameter("facing", "choice", "fixed", "Facing mode of the surface");
            facing.Choices = { "fixed", "billboard", "billboard-y" };
            d.Parameters.push_back(facing);
            UiPreviewParameter interaction = Parameter("interaction", "choice", "aim", "Interaction mode of the surface");
            interaction.Choices = { "none", "aim", "focus", "both" };
            d.Parameters.push_back(interaction);
            d.Scenarios.push_back(Scenario("default", "Default", { { "width", 512 }, { "height", 256 }, { "title", "Terminal" } }));
            d.Scenarios.push_back(Scenario("wide", "Wide sign", { { "width", 1024 }, { "height", 128 }, { "title", "Welcome to the vault" } }));
            return d;
        }

    protected:
        void Fill(UiElem& root, const json& values) override
        {
            root.Style().Width = Px(values.value("width", 512.f));
            root.Style().Height = Px(values.value("height", 256.f));
            root.Style().BackgroundColor = glm::vec4(0.06f, 0.12f, 0.19f, 1.f);
            root.Style().Flow = UiFlow::Down;
            root.Style().PaddingTop = root.Style().PaddingLeft = root.Style().PaddingRight = root.Style().PaddingBottom = Px(16.f);
            auto label = std::make_shared<UiElemLabel>();
            root.AddChild(label);
            label->SetResolvedText(values.value("title", std::string("Terminal")));
        }
    };

    // A scene view with one model, a key light and an ambient light.
    class SceneViewPreviewProvider : public BuiltProvider
    {
    public:
        UiPreviewDescriptor Describe() const override
        {
            UiPreviewDescriptor d;
            d.Id = "runtime.engine.scene-view";
            d.Name = "Scene view (sample)";
            d.Category = "World UI";
            d.Description = "A SceneView with one model, a key light and an ambient light. The picture is rendered into a target of the content size.";
            d.Kind = "runtime";
            d.ProviderId = "engine.ui.preview.scene-view";
            d.ProviderVersion = 1;
            d.SourceRefs.push_back({ "code", "source/Engine/UI/SceneView/UiElemSceneView.cpp" });
            d.HostKind = "viewport";
            d.RequiredCapabilities = { "markup", "sceneView" };
            UiPreviewParameter width = Parameter("width", "int", 320, "View width (px)");
            width.Min = 64; width.Max = 1024; width.Step = 16;
            UiPreviewParameter height = Parameter("height", "int", 320, "View height (px)");
            height.Min = 64; height.Max = 1024; height.Step = 16;
            d.Parameters.push_back(width);
            d.Parameters.push_back(height);
            d.Parameters.push_back(Parameter("model", "string", std::string("GameData/models/engine/widgetPlane.glb"), "GameData-relative path of the .glb"));
            UiPreviewParameter distance = Parameter("distance", "float", 2.5, "Camera distance from the origin (m)");
            distance.Min = 0.2; distance.Max = 20.0;
            d.Parameters.push_back(distance);
            UiPreviewParameter fov = Parameter("fov", "float", 40.0, "Vertical field of view (degrees)");
            fov.Min = 10.0; fov.Max = 120.0;
            d.Parameters.push_back(fov);
            UiPreviewParameter yaw = Parameter("yaw", "float", 0.0, "Model rotation about the vertical axis (degrees)");
            yaw.Min = -180.0; yaw.Max = 180.0;
            d.Parameters.push_back(yaw);
            d.Scenarios.push_back(Scenario("default", "Default", { { "width", 320 }, { "height", 320 }, { "yaw", 0.0 } }));
            d.Scenarios.push_back(Scenario("turned", "Turned 60 degrees", { { "width", 320 }, { "height", 320 }, { "yaw", 60.0 } }));
            return d;
        }

    protected:
        void Fill(UiElem& root, const json& values) override
        {
            root.Style().Width = Px(values.value("width", 320.f));
            root.Style().Height = Px(values.value("height", 320.f));
            auto view = std::make_shared<UiElemSceneView>();
            view->Style().Width = Px(values.value("width", 320.f));
            view->Style().Height = Px(values.value("height", 320.f));
            root.AddChild(view);
            view->ApplyAttribute("background", "#204060");
            view->SetCamera(glm::vec3(0.f, 0.f, values.value("distance", 2.5f)), glm::vec3(0.f), values.value("fov", 40.f));
            UiSceneModel* model = view->AddModel(values.value("model", std::string("GameData/models/engine/widgetPlane.glb")));
            model->SetTransform(glm::vec3(0.f), glm::vec3(0.f, values.value("yaw", 0.f), 0.f), glm::vec3(1.f));
            auto key = std::make_shared<UiSceneLight>();
            view->AddChild(key);
            auto ambient = std::make_shared<UiSceneLight>();
            ambient->LightType = UiSceneLight::Type::Ambient;
            ambient->Color = glm::vec3(0.5f);
            view->AddChild(ambient);
        }
    };
}

void UiHudPreviews::RegisterProviders()
{
    UiPreviewRegistry::RegisterProvider(std::make_shared<CrosshairPreviewProvider>());
    UiPreviewRegistry::RegisterProvider(std::make_shared<SurfacePreviewProvider>());
    UiPreviewRegistry::RegisterProvider(std::make_shared<SceneViewPreviewProvider>());
}
