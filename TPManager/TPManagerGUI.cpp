#include "pch.h"
#include "TPManager.h"

// Plugin Settings Window code here
std::string TPManager::GetPluginName() {
	return "TPManager";
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> TPManager
void TPManager::RenderSettings() {
	//ImGui::TextUnformatted("TPManager plugin settings");
	ImGui::PushItemWidth(350.0f);
	RenderInfo();
}

// Do ImGui rendering here
void TPManager::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	ImGui::PushItemWidth(350.0f);
	RenderInfo();
	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void TPManager::RenderInfo() 
{
	ImGui::TextUnformatted("This is a GUI to look at and control postions and movements in game. Simply cuz I was tired of typing \"Player location x y z\" over and over");
	//render/modify stuff
	std::vector<positionInfo> info = getPositionInfo();
	char input[1024];
	if (info.size())
	{
		if (ImGui::TreeNode("Ball/s"))
		{
			ImGui::Separator();
			ImGui::Indent(20);
			for (int i = 0; i < numBalls; ++i)
			{
				if (ImGui::TreeNode(info[i].name.c_str()))
				{
					pollingMutex.lock();
					RenderLocation(info[i]);
					RenderVelocity(info[i]);
					RenderAngularVelocity(info[i]);
					RenderRotation(info[i]);
					ImGui::NewLine();
					ImGui::SearchableCombo("will TP", &currentTPItem, itemsToSearch, "No entities", "type to search");
					ImGui::SameLine();
					if (ImGui::Button("to here"))
					{
						teleportSelectionToEntity(currentTPItem, info[i]);
					}
					ImGui::SameLine();
					if (ImGui::Button("above here"))
					{
						teleportSelectionToEntity(currentTPItem, info[i], true);
					}
					ImGui::NewLine();
					pollingMutex.unlock();
					ImGui::TreePop();
				}
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNode("Player/s")) 
		{
			ImGui::Separator();
			ImGui::Indent(20);
			for (int i = numBalls; i < info.size(); ++i)
			{
				if (ImGui::TreeNode(info[i].name.c_str()))
				{
					pollingMutex.lock();
					RenderLocation(info[i]);
					RenderVelocity(info[i]);
					RenderAngularVelocity(info[i]);
					RenderRotation(info[i]);
					ImGui::NewLine();
					ImGui::SearchableCombo("will TP", &currentTPItem, itemsToSearch, "No entities", "type to search");
					ImGui::SameLine();
					if (ImGui::Button("to here"))
					{
						teleportSelectionToEntity(currentTPItem, info[i]);
					}
					ImGui::SameLine();
					if (ImGui::Button("above here"))
					{
						teleportSelectionToEntity(currentTPItem, info[i], true);
					}
					pollingMutex.unlock();
					ImGui::TreePop();
				}
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::Text("Must be in a game to see/use.");
	}

	ImGui::Separator();
	ImGui::NewLine();
	ImGui::PushItemWidth(250.0f);
	ImGui::SliderInt("Polling time (ms)", &pollingRateMiliseconds, 50, 5000);
}

void TPManager::RenderLocation(positionInfo entity)
{
	ImGui::TextUnformatted("Location");
	ImGui::PushItemWidth(150.0f);
	ImGui::BeginGroup();
	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::PushID(("Location_x" + std::to_string((uintptr_t) &entity)).c_str());
	if (ImGui::DragFloat("", &(entity.location.X)), 5.0 && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_POSITION);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::PushID(("Location_y" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragFloat("", &(entity.location.Y)), 5.0 && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_POSITION);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::PushID(("Location_z" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragFloat("", &(entity.location.Z)), 5.0 && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_POSITION);
	}
	ImGui::PopID();
	ImGui::EndGroup();
}

void TPManager::RenderRotation(positionInfo entity)
{
	ImGui::TextUnformatted("Rotation");
	ImGui::PushItemWidth(150.0f);
	ImGui::BeginGroup();
	ImGui::Text("Yaw");
	ImGui::SameLine();
	ImGui::PushID(("Rotation_yaw" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragInt("", &(entity.rotation.Yaw)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_ROTATION);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Pitch");
	ImGui::SameLine();
	ImGui::PushID(("Rotation_pitch" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragInt("", &(entity.rotation.Pitch)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_ROTATION);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Roll");
	ImGui::SameLine();
	ImGui::PushID(("Rotation_roll" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragInt("", &(entity.rotation.Roll)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_ROTATION);
	}
	ImGui::PopID();
	ImGui::EndGroup();
}

void TPManager::RenderVelocity(positionInfo entity)
{
	ImGui::TextUnformatted("Velocity"); 
	ImGui::PushItemWidth(150.0f);
	ImGui::BeginGroup();
	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::PushID(("Velocity_x" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragFloat("", &(entity.velocity.X)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_VELOCITY);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::PushID(("Velocity_y" + std::to_string((uintptr_t)&entity)).c_str());
	ImGui::SameLine();
	if (ImGui::DragFloat("", &(entity.velocity.Y)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_VELOCITY);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::PushID(("Velocity_z" + std::to_string((uintptr_t)&entity)).c_str());
	ImGui::SameLine();
	if (ImGui::DragFloat("", &(entity.velocity.Z)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_VELOCITY);
	}
	ImGui::PopID();
	ImGui::EndGroup();
}

void TPManager::RenderAngularVelocity(positionInfo entity)
{
	ImGui::TextUnformatted("Angular Velocity"); 
	ImGui::PushItemWidth(150.0f);
	ImGui::BeginGroup();
	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::PushID(("angVelocity_x" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragFloat("", &(entity.angVelocity.X)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_ANGULAR_VELOCITY);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::PushID(("angVelocity_y" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragFloat("", &(entity.angVelocity.Y)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_ANGULAR_VELOCITY);
	}
	ImGui::PopID();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::PushID(("angVelocity_z" + std::to_string((uintptr_t)&entity)).c_str());
	if (ImGui::DragFloat("", &(entity.angVelocity.Z)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
	{
		setPositionInfo(entity, updatePositionType::UPDATE_ANGULAR_VELOCITY);
	}
	ImGui::PopID();
	ImGui::EndGroup();
}

// Name of the menu that is used to toggle the window.
std::string TPManager::GetMenuName()
{
	return "tpmanager";
}

// Title to give the menu
std::string TPManager::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void TPManager::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool TPManager::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool TPManager::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void TPManager::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void TPManager::OnClose()
{
	isWindowOpen_ = false;
}
