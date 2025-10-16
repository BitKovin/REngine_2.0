#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <imgui.h>
#include "BehaviorTreeTypes.h"
#include "BehaviorTree.h"
#include "TreeNode.h"

class BehaviorTreeEditor {
public:
	BehaviorTreeEditor();
	~BehaviorTreeEditor() = default;

	// Lifecycle
	void Init();
	void Update(float deltaTime);
	void Draw();

    // Optional integration helpers
    BehaviorTree& GetTree() { return *tree_; }

private:
	// --- Editor State ---
	struct SimulationState {
		bool playing = false;
		bool paused = true;
		bool stepOnce = false;
		float fixedDeltaTime = 0.1f;
		float timeScale = 1.0f;
		float accumulator = 0.0f;
		bool loop = true; // when false, stop on root completion
	};

	struct FileState {
		std::string currentFileName; // without directories
		std::string lastError;
	};

	struct SelectionState {
		TreeNode* selected = nullptr;
		// Cached editable JSON for selected node's parameters serialized to string
		std::string cachedParamsSerialized;
		bool dirty = false;
	};

	// --- Rendering helpers ---
	void DrawMenuBar();
	void DrawSimulationControls();
	void DrawTreeView();
	void DrawNodeRecursive(TreeNode* node, int depth);
	void DrawPropertiesPanel();
	void DrawBlackboardPanel();

	// Node params caching/apply
	void OnSelectionChanged(TreeNode* newSelection);
	static std::string ExtractParamsFromNodeJsonSerialized(TreeNode* node);
	static bool ApplyParamsSerializedToNode(const std::string& paramsSerialized, TreeNode* node);

	// File operations
	bool LoadTree(const std::string& fileName);
	bool SaveTree(const std::string& fileName);

	// Utilities

	ImU32 StatusColor(NodeStatus status);

    static bool IsActiveNode(TreeNode* node, const std::vector<TreeNode*>& active);


private:
    std::unique_ptr<BehaviorTree> tree_;
	SimulationState sim_{};
	FileState file_{};
	SelectionState selection_{};
};