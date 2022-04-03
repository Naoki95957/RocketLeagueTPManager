#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

const unsigned short UPDATE_ALL = 0;
const unsigned short UPDATE_POSITION = 1;
const unsigned short UPDATE_ROTATION = 2;
const unsigned short UPDATE_VELOCITY = 3;
const unsigned short UPDATE_ANGULAR_VELOCITY = 4;

struct positionInfo
{
	std::string name;
	Vector location;
	Rotator rotation;
	Vector velocity;
	Vector angVelocity;
};

class TPManager: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*//*, public BakkesMod::Plugin::PluginWindow*/
{
	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	std::vector<positionInfo> getPositionInfo();
	void setPositionInfo(positionInfo info, unsigned short updateField = UPDATE_ALL);

	//// Inherited via PluginSettingsWindow
	//void RenderSettings() override;
	//std::string GetPluginName() override;
	//void SetImGuiContext(uintptr_t ctx) override;

	//// Inherited via PluginWindow

	//bool isWindowOpen_ = false;
	//bool isMinimized_ = false;
	//std::string menuTitle_ = "TPManager";

	//virtual void Render() override;
	//virtual std::string GetMenuName() override;
	//virtual std::string GetMenuTitle() override;
	//virtual void SetImGuiContext(uintptr_t ctx) override;
	//virtual bool ShouldBlockInput() override;
	//virtual bool IsActiveOverlay() override;
	//virtual void OnOpen() override;
	//virtual void OnClose() override;
};

