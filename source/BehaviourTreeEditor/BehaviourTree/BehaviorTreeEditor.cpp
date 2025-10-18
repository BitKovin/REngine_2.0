#include "BehaviorTreeEditor.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "BehaviorTree.h"
#include "NodeFactory.h"
#include "Blackboard.h"
#include "CompositeNode.h"
#include "DecoratorNodes.h"
#include <cstdint>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <FileSystem/FileSystem.h>
#include <json.hpp>
#include <iostream>

// --- Utilities ---
static std::string MakeGameDataPath(const std::string& fileName) {
	return std::string("GameData/behaviourTrees/") + fileName;
}
static std::string MakeSaveDataPath(const std::string& fileName) {
	return std::string("GameData/behaviourTrees/") + fileName; // Editor writes to GameData
}

using nlohmann::json;

BehaviorTreeEditor::BehaviorTreeEditor() {
	tree_ = std::make_unique<BehaviorTree>();
	columnWidth = 420.0f;
	propertiesHeight = 200.0f;
}

void BehaviorTreeEditor::Init() {
	FileSystemEngine::Init();
	// Ensure there is a root to work with if empty
	if (!tree_->GetRoot()) {
		// Default root: Infinite Repeat -> Sequence
		auto repeat = NodeFactory::GetInstance().CreateNode("RepeatDecorator");
		auto sequence = NodeFactory::GetInstance().CreateNode("SequenceNode");

		if (repeat && sequence) {
			auto repeatDecorator = std::dynamic_pointer_cast<RepeatDecorator>(repeat);
			if (repeatDecorator) {
				repeatDecorator->SetRepeatCount(-1); // Infinite
			}
			repeat->AddChild(sequence);
			tree_->SetRoot(repeat);
		}
		else if (sequence) {
			// Fallback to just sequence
			tree_->SetRoot(sequence);
		}
	}
}

void BehaviorTreeEditor::Update(float deltaTime)
{
	// Clear drag state if not actually dragging anymore
	if (dragDrop_.isDragging && !ImGui::IsMouseDragging(0)) {
		dragDrop_.isDragging = false;
		dragDrop_.draggedNode = nullptr;
	}

	// Simulation stepping
	if (sim_.playing && !sim_.paused) {
		// Use real-time delta for continuous simulation
		tree_->Update(deltaTime * sim_.timeScale);
	}
	else if (sim_.stepOnce) {
		// Use fixed delta only for single-step
		tree_->Update(sim_.fixedDeltaTime * sim_.timeScale);
		sim_.stepOnce = false;
	}
}

void BehaviorTreeEditor::Draw() {
	if (ImGui::Begin("Behavior Tree Editor")) {
		DrawMenuBar();

		ImGui::Separator();
		DrawSimulationControls();
		ImGui::Separator();

		// Layout: left tree, right properties and blackboard
		ImGui::Columns(2, nullptr, false);
		ImGui::SetColumnWidth(0, columnWidth);
		DrawTreeView();
		columnWidth = ImGui::GetColumnWidth(0);
		ImGui::NextColumn();
		DrawPropertiesPanel();

		// Vertical splitter between properties and blackboard
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
		ImGui::Button("##vsplitter", ImVec2(-1.0f, 3.0f));
		ImGui::PopStyleColor(3);
		if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
		}
		if (ImGui::IsItemActive()) {
			float old_height = propertiesHeight;
			propertiesHeight += ImGui::GetIO().MouseDelta.y;
			float min_h = 50.0f;
			float max_h = old_height + ImGui::GetContentRegionAvail().y - min_h;
			propertiesHeight = std::clamp(propertiesHeight, min_h, max_h);
		}

		DrawBlackboardPanel();
		ImGui::Columns(1);
	}
	ImGui::End();
}

void BehaviorTreeEditor::DrawMenuBar() {
	if (ImGui::BeginChild("##menubar", ImVec2(0, 28), false, ImGuiWindowFlags_NoScrollbar)) {
		// File name box
		ImGui::SetNextItemWidth(260.0f);
		ImGui::InputText("##file", &file_.currentFileName);
		ImGui::SameLine();
		if (ImGui::Button("LoadTree")) {
			if (!file_.currentFileName.empty()) {
				if (!LoadTree(file_.currentFileName)) {
					// Load failed; keep lastError
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("SaveTree")) {
			if (!file_.currentFileName.empty()) {
				if (!SaveTree(file_.currentFileName)) {
					// Save failed; keep lastError
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset")) {
			tree_->Reset();
		}
		if (!file_.lastError.empty()) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", file_.lastError.c_str());
		}
	}
	ImGui::EndChild();
}

void BehaviorTreeEditor::DrawSimulationControls() {
	if (ImGui::BeginChild("##sim", ImVec2(0, 36), false, ImGuiWindowFlags_NoScrollbar)) {
		if (ImGui::Button(sim_.playing ? (sim_.paused ? "Resume" : "Pause") : "Play")) {
			if (!sim_.playing) {
				// Start
				tree_->Start();
				sim_.playing = true;
				sim_.paused = false;
				sim_.accumulator = 0.0f;
			}
			else {
				// Toggle pause
				sim_.paused = !sim_.paused;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			tree_->Stop();
			sim_.playing = false;
			sim_.paused = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Step")) {
			if (!sim_.playing) {
				// Ensure context is initialized
				tree_->Start();
				sim_.playing = true;
				sim_.paused = true;
			}
			sim_.stepOnce = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(110.0f);
		ImGui::DragFloat("dt", &sim_.fixedDeltaTime, 0.001f, 0.001f, 1.0f, "%.3f");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(110.0f);
		ImGui::DragFloat("speed", &sim_.timeScale, 0.01f, 0.01f, 10.0f, "%.2fx");
		ImGui::SameLine();
	}
	ImGui::EndChild();
}

ImU32 BehaviorTreeEditor::StatusColor(NodeStatus status) {
	switch (status) {
	case NodeStatus::Idle:
		return IM_COL32(255, 255, 255, 255); // Gray
	case NodeStatus::Running:
		return IM_COL32(70, 130, 180, 255);  // Steel blue
	case NodeStatus::Success:
		return IM_COL32(50, 205, 50, 255);   // Lime green
	case NodeStatus::Failure:
		return IM_COL32(220, 20, 60, 255);   // Crimson red
	default:
		return IM_COL32(200, 200, 200, 255);
	}
}

bool BehaviorTreeEditor::IsActiveNode(TreeNode* node, const std::vector<TreeNode*>& active) {
	return std::find(active.begin(), active.end(), node) != active.end();
}


void BehaviorTreeEditor::DrawTreeView() {
	if (ImGui::BeginChild("##tree", ImVec2(0, 0), true)) {
		auto root = tree_->GetRoot();
		if (root) {
			DrawNodeRecursive(root.get(), 0);
		}
		else {
			ImGui::TextDisabled("No root node");
		}
	}
	ImGui::EndChild();
}

void BehaviorTreeEditor::DrawNodeRecursive(TreeNode* node, int depth) {
	if (!node) return;

	bool isRoot = (node == tree_->GetRoot().get());

	const bool leafAtStart = node->GetChildren().empty();
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
	if (selection_.selected == node) flags |= ImGuiTreeNodeFlags_Selected;
	if (leafAtStart) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	if (isRoot) flags |= ImGuiTreeNodeFlags_DefaultOpen;

	// Get execution state
	TreeNode* lastExecuted = tree_->GetLastExecutedNode();
	const auto& activeNodes = tree_->GetActiveNodes();
	bool isActive = IsActiveNode(node, activeNodes);
	bool isLastExecuted = (node == lastExecuted);

	// Determine visual style based on execution state
	ImU32 textColor;
	ImU32 bgColor = 0;

	if (isLastExecuted) {
		textColor = IM_COL32(255, 255, 255, 255);
		bgColor = IM_COL32(255, 165, 0, 255);
	}
	else if (isActive && node->GetStatus() == NodeStatus::Running) {
		textColor = IM_COL32(255, 255, 255, 255);
		bgColor = IM_COL32(65, 105, 225, 255);
	}
	else if (isActive) {
		textColor = IM_COL32(255, 255, 255, 255);
		bgColor = IM_COL32(100, 149, 237, 255);
	}
	else {
		textColor = StatusColor(node->GetStatus());
	}

	// Draw background if needed
	if (bgColor != 0) {
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		ImVec2 contentRegion = ImGui::GetContentRegionAvail();
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();

		ImGui::GetWindowDrawList()->AddRectFilled(
			cursorPos,
			ImVec2(cursorPos.x + contentRegion.x, cursorPos.y + lineHeight),
			bgColor
		);
	}

	// Set item spacing for better drag/drop visualization
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

	ImGui::PushStyleColor(ImGuiCol_Text, textColor);
	bool open = ImGui::TreeNodeEx((void*)node, flags, "%s (%s)", node->GetName().c_str(), node->GetType().c_str());
	ImGui::PopStyleColor();

	// Draw drag drop preview
	if (ImGui::IsItemHovered() && dragDrop_.isDragging && dragDrop_.draggedNode != node && dragDrop_.draggedNode != nullptr) {
		auto pos = CalculateDropPosition(node);
		if (CanPerformDrop(dragDrop_.draggedNode, node, pos)) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 rect_min = ImGui::GetItemRectMin();
			ImVec2 rect_max = ImGui::GetItemRectMax();
			ImU32 color = IM_COL32(255, 255, 0, 255);
			float thickness = 2.0f;
			switch (pos) {
			case DragDropState::DropPosition::Before:
				draw_list->AddLine(ImVec2(rect_min.x, rect_min.y), ImVec2(rect_max.x, rect_min.y), color, thickness);
				break;
			case DragDropState::DropPosition::After:
				draw_list->AddLine(ImVec2(rect_min.x, rect_max.y), ImVec2(rect_max.x, rect_max.y), color, thickness);
				break;
			case DragDropState::DropPosition::On:
				draw_list->AddRect(rect_min, rect_max, color, 0.0f, ImDrawFlags_None, thickness);
				break;
			}
		}
	}

	// Handle drag source (skip for root)
	if (!isRoot) {
		HandleDragSource(node);
	}

	// Add execution indicator
	if (isLastExecuted) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), " <--");
	}

	if (ImGui::IsItemClicked()) {
		OnSelectionChanged(node);
	}

	// Enhanced tooltip with execution info
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();

		if (isLastExecuted) {
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "LAST EXECUTED");
			ImGui::Separator();
		}

		if (isActive) {
			ImGui::TextColored(ImVec4(0, 1, 1, 1), "ACTIVE");
		}

		ImGui::Text("Status: %s",
			node->GetStatus() == NodeStatus::Idle ? "Idle" :
			node->GetStatus() == NodeStatus::Running ? "Running" :
			node->GetStatus() == NodeStatus::Success ? "Success" : "Failure");

		ImGui::Text("Type: %s", node->GetType().c_str());
		ImGui::Text("Name: %s", node->GetName().c_str());
		ImGui::Text("Children: %zu", node->GetChildren().size());

		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Drag: Click and drag to move");
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Drop: Middle - child, Top - before, Bottom - after");

		ImGui::EndTooltip();
	}

	// Context menu for adding/removing children
	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::BeginMenu("Add Child")) {
			for (const auto& typeName : NodeFactory::GetInstance().GetRegisteredNodeTypes()) {
				if (ImGui::MenuItem(typeName.c_str())) {
					auto newNode = NodeFactory::GetInstance().CreateNode(typeName);
					if (newNode) {
						// Check if this is a decorator that already has a child
						if (auto* decorator = dynamic_cast<DecoratorNode*>(node)) {
							if (decorator->GetChildren().empty()) {
								decorator->SetChild(newNode);
							}
							else {
								// Decorator already has a child, add to parent instead
								if (node->GetParent()) {
									node->GetParent()->AddChild(newNode);
								}
								else {
									// If no parent (shouldn't happen for decorator), just add as child anyway
									decorator->SetChild(newNode);
								}
							}
						}
						else {
							// Regular node - just add child
							node->AddChild(newNode);
						}
					}
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndMenu();
		}

		if (node->GetParent() != nullptr && !isRoot) {
			if (ImGui::MenuItem("Remove")) {
				node->GetParent()->RemoveChild(node);
				if (selection_.selected == node) OnSelectionChanged(nullptr);
			}
		}
		ImGui::EndPopup();
	}

	// Handle drop target for ALL nodes (including root as drop target)
	HandleDropTarget(node);

	if (open && !leafAtStart) {
		for (auto& child : node->GetChildren()) {
			DrawNodeRecursive(child.get(), depth + 1);
		}
		ImGui::TreePop();
	}

	ImGui::PopStyleVar(); // ItemSpacing
}

void BehaviorTreeEditor::HandleDragSource(TreeNode* node) {
	// Don't allow dragging the root node
	if (node == tree_->GetRoot().get()) {
		return;
	}

	// Start drag when item is dragged
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		dragDrop_.draggedNode = node;
		dragDrop_.isDragging = true;

		// Set drag payload
		ImGui::SetDragDropPayload("BT_NODE", &node, sizeof(TreeNode*));

		// Preview of what's being dragged
		ImGui::Text("Moving: %s", node->GetName().c_str());

		// Show children count if any
		if (!node->GetChildren().empty()) {
			ImGui::Text("+ %zu children", node->GetChildren().size());
		}

		ImGui::EndDragDropSource();
	}
}

BehaviorTreeEditor::DragDropState::DropPosition BehaviorTreeEditor::CalculateDropPosition(TreeNode* target) {
	ImVec2 rectMin = ImGui::GetItemRectMin();
	ImVec2 rectMax = ImGui::GetItemRectMax();
	float height = rectMax.y - rectMin.y;
	float mouseY = ImGui::GetMousePos().y - rectMin.y;
	float relative = mouseY / height;
	if (relative < 0.3f) return DragDropState::DropPosition::Before;
	if (relative > 0.7f) return DragDropState::DropPosition::After;
	return DragDropState::DropPosition::On;
}

void BehaviorTreeEditor::HandleDropTarget(TreeNode* target) {
	// Use the tree node itself as the drop target
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BT_NODE")) {
			IM_ASSERT(payload->DataSize == sizeof(TreeNode*));
			TreeNode* draggedNode = *(TreeNode**)payload->Data;

			if (draggedNode && draggedNode != target) {
				auto pos = CalculateDropPosition(target);
				if (CanPerformDrop(draggedNode, target, pos)) {
					PerformDrop(draggedNode, target, pos);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
}

bool BehaviorTreeEditor::CanPerformDrop(TreeNode* dragged, TreeNode* target, DragDropState::DropPosition pos) {
	if (!dragged || !target) return false;
	if (dragged == target) return false;
	if (dragged == tree_->GetRoot().get()) return false;

	TreeNode* newParent = (pos == DragDropState::DropPosition::On) ? target : target->GetParent();
	if (!newParent) return false;

	// Check for circular dependency
	TreeNode* parentCheck = newParent;
	while (parentCheck) {
		if (parentCheck == dragged) return false;
		parentCheck = parentCheck->GetParent();
	}

	// Check if newParent can accept more children
	if (auto decorator = dynamic_cast<DecoratorNode*>(newParent)) {
		size_t maxChildren = 1;
		size_t currentChildren = newParent->GetChildren().size();
		bool isExistingChild = (dragged->GetParent() == newParent);
		if (currentChildren >= maxChildren && !isExistingChild) {
			return false;
		}
	}

	// Allow for composites without limit
	return true;
}

void BehaviorTreeEditor::PerformDrop(TreeNode* dragged, TreeNode* target, DragDropState::DropPosition pos) {
	if (!dragged || !target) return;
	if (!CanPerformDrop(dragged, target, pos)) return;

	TreeNode* oldParent = dragged->GetParent();
	if (!oldParent) return;

	std::shared_ptr<TreeNode> draggedShared = oldParent->ExtractChild(dragged);
	if (!draggedShared) return;

	TreeNode* newParent = (pos == DragDropState::DropPosition::On) ? target : target->GetParent();

	if (auto decorator = dynamic_cast<DecoratorNode*>(newParent)) {
		decorator->SetChild(draggedShared);
	}
	else {
		if (pos == DragDropState::DropPosition::On) {
			newParent->AddChild(draggedShared);
		}
		else {
			auto& children = newParent->GetChildren();
			auto it = std::find_if(children.begin(), children.end(), [&](const std::shared_ptr<TreeNode>& c) { return c.get() == target; });
			if (it == children.end()) return;
			size_t index = std::distance(children.begin(), it);
			if (pos == DragDropState::DropPosition::After) ++index;
			children.insert(children.begin() + index, draggedShared);
			draggedShared->SetParent(newParent);
		}
	}

	// Clear drag state
	dragDrop_.draggedNode = nullptr;
	dragDrop_.isDragging = false;

	// Update selection
	if (selection_.selected == dragged) {
		OnSelectionChanged(draggedShared.get());
	}
}

void BehaviorTreeEditor::OnSelectionChanged(TreeNode* newSelection) {
	selection_.selected = newSelection;
	selection_.dirty = false;
	selection_.cachedParamsSerialized.clear();
	if (!newSelection) return;

	selection_.cachedParamsSerialized = ExtractParamsFromNodeJsonSerialized(newSelection);
}

std::string BehaviorTreeEditor::ExtractParamsFromNodeJsonSerialized(TreeNode* node) {
	json nodeJson = node->ToJson();
	json params;
	for (auto it = nodeJson.begin(); it != nodeJson.end(); ++it) {
		const std::string key = it.key();
		if (key == "name" || key == "type" || key == "children") continue;
		params[key] = it.value();
	}
	return params.dump(2);
}

bool BehaviorTreeEditor::ApplyParamsSerializedToNode(const std::string& paramsSerialized, TreeNode* node) {
	json params = json::parse(paramsSerialized, nullptr, false);
	if (params.is_discarded()) return false;
	json nodeJson = node->ToJson();
	for (auto it = params.begin(); it != params.end(); ++it) {
		nodeJson[it.key()] = it.value();
	}
	node->FromJson(nodeJson);
	return true;
}

// Forward declarations of local editors
static bool EditJsonObject(json& j);
static bool EditJsonValue(const char* label, json& v);

void BehaviorTreeEditor::DrawPropertiesPanel() {
	if (ImGui::BeginChild("##props", ImVec2(0, propertiesHeight), true)) {
		if (!selection_.selected) {
			ImGui::TextDisabled("Select a node to edit properties");
		}
		else {
			ImGui::Text("%s (%s)", selection_.selected->GetName().c_str(), selection_.selected->GetType().c_str());
			ImGui::InputText("display name", &selection_.selected->name_);
			ImGui::Separator();

			json params = json::parse(selection_.cachedParamsSerialized, nullptr, false);
			if (params.is_discarded()) { params = json::object(); }
			bool changed = EditJsonObject(params);
			if (changed) {
				// Auto-apply changes immediately to avoid UI reverting on focus change
				selection_.cachedParamsSerialized = params.dump(2);
				ApplyParamsSerializedToNode(selection_.cachedParamsSerialized, selection_.selected);
				selection_.dirty = false;
			}
		}
	}
	ImGui::EndChild();
}
// Local editors implementation
static bool EditJsonObject(json& j) {
	bool edited = false;
	for (auto it = j.begin(); it != j.end(); ++it) {
		std::string label = it.key();
		edited = EditJsonValue(label.c_str(), it.value()) || edited;
	}
	return edited;
}

static bool EditBTVariable(const char* label, json& jVar) {
	bool changed = false;
	// Expected shape from BTVariable::ToJson(): { sourceType:int, value:any | blackboardKey:string }
	int sourceType = (int)jVar.value("sourceType", 0);
	if (ImGui::BeginTable(label, 2, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("%s", label);
		ImGui::TableSetColumnIndex(1);
		if (ImGui::Combo("##src", &sourceType, "Constant\0Blackboard\0")) {
			jVar["sourceType"] = sourceType;
			if (sourceType == 0) {
				jVar.erase("blackboardKey");
				if (!jVar.contains("value")) jVar["value"] = 0; // default
			}
			else {
				// jVar.erase("value");  // Removed to preserve original value and type
				if (!jVar.contains("blackboardKey")) jVar["blackboardKey"] = std::string("");
			}
			changed = true;
		}

		if (sourceType == 0) {
			auto& v = jVar["value"];
			// edit scalar types generically
			if (v.is_number_integer()) {
				int val = v.get<int>();
				if (ImGui::DragInt("##ival", &val)) { v = val; changed = true; }
			}
			else if (v.is_number_float()) {
				float val = v.get<float>();
				if (ImGui::DragFloat("##fval", &val, 0.01f)) { v = val; changed = true; }
			}
			else if (v.is_boolean()) {
				bool val = v.get<bool>();
				if (ImGui::Checkbox("##bval", &val)) { v = val; changed = true; }
			}
			else {
				std::string val = v.is_string() ? v.get<std::string>() : std::string("");
				if (ImGui::InputText("##sval", &val)) { v = val; changed = true; }
			}
		}
		else {
			std::string key = jVar.value("blackboardKey", std::string(""));
			if (ImGui::InputText("##bbkey", &key)) { jVar["blackboardKey"] = key; changed = true; }
		}
		ImGui::EndTable();
	}
	return changed;
}

static bool EditJsonValue(const char* label, json& v) {
	bool changed = false;
	if (v.is_object()) {
		// Heuristic: If object looks like BTVariable (has sourceType)
		if (v.contains("sourceType")) {
			changed = EditBTVariable(label, v);
		}
		else {
			if (ImGui::TreeNode(label)) {
				for (auto it = v.begin(); it != v.end(); ++it) {
					std::string childLabel = std::string(label) + "." + it.key();
					changed = EditJsonValue(childLabel.c_str(), it.value()) || changed;
				}
				ImGui::TreePop();
			}
		}
	}
	else if (v.is_array()) {
		if (ImGui::TreeNode(label)) {
			for (size_t i = 0; i < v.size(); ++i) {
				std::string elemLabel = std::string(label) + "[" + std::to_string(i) + "]";
				changed = EditJsonValue(elemLabel.c_str(), v[i]) || changed;
			}
			ImGui::TreePop();
		}
	}
	else if (v.is_boolean()) {
		bool b = v.get<bool>();
		if (ImGui::Checkbox(label, &b)) { v = b; changed = true; }
	}
	else if (v.is_number_integer()) {
		int i = v.get<int>();
		if (ImGui::DragInt(label, &i)) { v = i; changed = true; }
	}
	else if (v.is_number_float()) {
		float f = v.get<float>();
		if (ImGui::DragFloat(label, &f, 0.01f)) { v = f; changed = true; }
	}
	else {
		std::string s = v.is_string() ? v.get<std::string>() : std::string("");
		if (ImGui::InputText(label, &s)) { v = s; changed = true; }
	}
	return changed;
}

void BehaviorTreeEditor::DrawBlackboardPanel() {
	if (ImGui::BeginChild("##bb", ImVec2(0, 0), true)) {
		ImGui::Text("Blackboard");
		ImGui::Separator();

		// Render all keys; provide add/remove and type-specific editors
		// We'll mirror to/from json for editing simplicity
		json j = tree_->GetBlackboard().ToJson();
		bool changed = false;

		// List entries
		for (auto it = j.begin(); it != j.end(); ++it) {
			std::string key = it.key();
			nlohmann::json& val = it.value();
			ImGui::PushID(key.c_str());
			if (ImGui::SmallButton("X")) {
				j.erase(key);
				ImGui::PopID();
				changed = true;
				break; // iterator invalidated
			}
			ImGui::SameLine();
			ImGui::Text("%s", key.c_str());
			ImGui::SameLine();
			std::string lbl = std::string("##") + key;
			changed = EditJsonValue(lbl.c_str(), val) || changed;
			ImGui::PopID();
		}

		// Add new variable
		static std::string newKey;
		static int newType = 0; // 0=int,1=float,2=bool,3=string
		ImGui::Separator();
		ImGui::InputText("New Key", &newKey);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		ImGui::Combo("Type", &newType, "int\0float\0bool\0string\0");
		ImGui::SameLine();
		if (ImGui::Button("Add")) {
			if (!newKey.empty() && !j.contains(newKey)) {
				if (newType == 0) j[newKey] = 0;
				else if (newType == 1) j[newKey] = 0.0f;
				else if (newType == 2) j[newKey] = false;
				else j[newKey] = std::string("");
				newKey.clear();
				changed = true;
			}
		}

		if (changed) {
			// Apply back to blackboard
			tree_->GetBlackboard().FromJson(j);
		}
	}
	ImGui::EndChild();
}

bool BehaviorTreeEditor::LoadTree(const std::string& fileName) {
	file_.lastError.clear();
	try {
		std::string content = FileSystemEngine::ReadFile(MakeGameDataPath(fileName));
		if (content.empty()) {
			file_.lastError = "File empty or not found";
			return false;
		}
		json j = json::parse(content, nullptr, false);
		if (j.is_discarded()) {
			file_.lastError = "JSON parse error";
			return false;
		}
		if (!tree_->FromJson(j)) {
			file_.lastError = "Tree FromJson failed";
			return false;
		}
		// Clear selection cache as structure changed
		OnSelectionChanged(nullptr);
		return true;
	}
	catch (const std::runtime_error& e) {
		file_.lastError = e.what();
		return false;
	}
}

bool BehaviorTreeEditor::SaveTree(const std::string& fileName) {
	file_.lastError.clear();
	json j = tree_->ToJson();
	std::string content = j.dump(4);
	if (!FileSystemEngine::WriteFile(MakeSaveDataPath(fileName), content)) {
		file_.lastError = "Write failed";
		return false;
	}
	return true;
}