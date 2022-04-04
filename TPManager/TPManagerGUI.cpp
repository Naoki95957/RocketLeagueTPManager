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
	RenderInfo();

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void TPManager::RenderInfo() 
{
	ImGui::TextUnformatted("This is a GUI to look at and control postions and movements in game.");
	//render/modify stuff
	std::vector<positionInfo> info = getPositionInfo();
	if (info.size())
	{
		for (short i = 0; i < info.size(); ++i)
		{
			if (ImGui::TreeNode(info[i].name.c_str()))
			{
				pollingMutex.lock();
				RenderLocation(info[i]);
				RenderRotation(info[i]);
				RenderVelocity(info[i]);
				RenderAngularVelocity(info[i]);
				pollingMutex.unlock();
				ImGui::TreePop();
			}
		}
	}
	else
	{
		ImGui::Text("Must be in a game to see/use.");
	}
}

void TPManager::RenderLocation(positionInfo entity)
{
	if (ImGui::TreeNode("Location"))
	{
		if (ImGui::DragFloat("X", &(entity.location.X)), 5.0 && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_POSITION);
		}
		if (ImGui::DragFloat("Y", &(entity.location.Y)), 5.0 && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_POSITION);
		}
		if (ImGui::DragFloat("Z", &(entity.location.Z)), 5.0 && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_POSITION);
		}

		int selection = 0;
		ImGui::TreePop();
	}
}

void TPManager::RenderRotation(positionInfo entity)
{
	if (ImGui::TreeNode("Rotation"))
	{
		if (ImGui::DragInt("Yaw", &(entity.rotation.Yaw)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_ROTATION);
		}
		if (ImGui::DragInt("Pitch", &(entity.rotation.Pitch)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_ROTATION);
		}
		if (ImGui::DragInt("Roll", &(entity.rotation.Roll)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_ROTATION);
		}
		ImGui::TreePop();
	}
}

void TPManager::RenderVelocity(positionInfo entity)
{
	if (ImGui::TreeNode("Velocity"))
	{
		if (ImGui::DragFloat("X", &(entity.velocity.X)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_VELOCITY);
		}
		if (ImGui::DragFloat("Y", &(entity.velocity.Y)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_VELOCITY);
		}
		if (ImGui::DragFloat("Z", &(entity.velocity.Z)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_VELOCITY);
		}
		ImGui::TreePop();
	}
}

void TPManager::RenderAngularVelocity(positionInfo entity)
{
	if (ImGui::TreeNode("Angular Velocity"))
	{
		if (ImGui::DragFloat("X", &(entity.angVelocity.X)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_ANGULAR_VELOCITY);
		}
		if (ImGui::DragFloat("Y", &(entity.angVelocity.Y)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_ANGULAR_VELOCITY);
		}
		if (ImGui::DragFloat("Z", &(entity.angVelocity.Z)) && ImGui::IsItemActive() && ImGui::IsItemEdited())
		{
			setPositionInfo(entity, updatePositionType::UPDATE_ANGULAR_VELOCITY);
		}
		ImGui::TreePop();
	}
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
