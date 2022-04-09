#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "imgui/imgui_searchablecombo.h"
#include "imgui/imgui.h"

#include "version.h"
#include <chrono>
#include <thread>

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

enum class updatePositionType
{	
	UPDATE_ALL,UPDATE_POSITION, UPDATE_ROTATION, UPDATE_VELOCITY, UPDATE_ANGULAR_VELOCITY
};

struct positionInfo
{
	ActorWrapper actor;
	std::string name;
	Vector location;
	Rotator rotation;
	Vector velocity;
	Vector angVelocity;
};

class TPManager: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{

private:
	std::mutex pollingMutex;
	std::mutex finishedPolling;
	bool pollForInfo;
	std::vector<positionInfo> positionalInfoAllEntities;
	std::vector<std::string> itemsToSearch = std::vector<std::string>();
	std::vector<std::string> destToSearch = std::vector<std::string>();
	int totalNumEntities = 0;
	int numBalls = 0;

	const char* SELECTION = "selection_tpmanager";
	const char* DESTINATION = "destination_tpmanager";
	int choiceSelection = 0;
	int choiceDestination = 0;

	float customDestX = 0.0f, customDestY = 0.0f, customDestZ = 0.0f;

	void getContinousInfo();
	std::vector<positionInfo> pollPositionInfo();

	void RenderInfo();
	void RenderLocation(positionInfo entity);
	void RenderRotation(positionInfo entity);
	void RenderVelocity(positionInfo entity);
	void RenderAngularVelocity(positionInfo entity);

public:
	int pollingRateMiliseconds = 250;
	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	std::vector<positionInfo> getPositionInfo();
	void setPositionInfo(positionInfo info, updatePositionType updateField = updatePositionType::UPDATE_ALL);
	void teleportSelectionToEntity(int selection, positionInfo entity, bool above = false);

	std::vector<PriWrapper> getPlayers(const bool includeBots = false, const bool mustBeAlive = false);

	//// Inherited via PluginSettingsWindow
	void RenderSettings() override;
	std::string GetPluginName() override;

	//// Inherited via PluginWindow
	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "TPManager";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
};

