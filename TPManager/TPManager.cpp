#include "pch.h"
#include "TPManager.h"


BAKKESMOD_PLUGIN(TPManager, "This plugin provides a basic GUI for teleporting players", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void TPManager::onLoad()
{
	_globalCvarManager = cvarManager;
	// TODO listen to which GUI element is being edited at any given time -> 
	// that way you only update 1 entity 
	// instead of all entities

	//cvarManager->log("Plugin loaded!");

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	cvarManager->log("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&TPManager::YourPluginMethod, this, _1, _2));

	////enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&TPManager::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	cvarManager->log("Your hook got called and the ball went POOF");
	//});
	////You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&TPManager::YourPluginMethod, this);
}

void TPManager::onUnload()
{
}

std::vector<positionInfo> TPManager::getPositionInfo()
{
	ServerWrapper gameState = gameWrapper.get()->GetCurrentGameState();

	//list containing all moving objects
	std::vector<positionInfo> allEntities = std::vector<positionInfo>();

	//ball
	allEntities.push_back({
		"Ball",
		gameState.GetBall().GetLocation(),
		gameState.GetBall().GetRotation(),
		gameState.GetBall().GetVelocity(),
		gameState.GetBall().GetAngularVelocity()
	});

	// per car
	ArrayWrapper cars = gameState.GetCars();
	std::vector<CarWrapper> carVector = std::vector<CarWrapper>();
	for (auto car : cars) {
		carVector.push_back(car);
	}

	std::sort(carVector.begin(), carVector.end(), [](CarWrapper& a, CarWrapper& b) -> bool
	{
		return a.GetOwnerName() < b.GetOwnerName();
	});

	for (auto car : carVector) 
	{
		allEntities.push_back({
			car.GetOwnerName(),
			car.GetLocation(),
			car.GetRotation(),
			car.GetVelocity(),
			car.GetAngularVelocity()
		});
	}

	return allEntities;
}

void TPManager::setPositionInfo(positionInfo info)
{
	ServerWrapper gameState = gameWrapper.get()->GetCurrentGameState();

	if (info.name == "Ball") {
		BallWrapper ballW = gameState.GetBall();
		ballW.SetLocation(info.location);
		ballW.SetRotation(info.rotation);
		ballW.SetVelocity(info.velocity);
		ballW.SetAngularVelocity(info.angVelocity, false);
	}
	else
	{
		for (auto car : gameState.GetCars()) {
			if (car.GetOwnerName() != info.name)
				continue;
			
			car.SetLocation(info.location);
			car.SetRotation(info.rotation);
			car.SetVelocity(info.velocity);
			car.SetAngularVelocity(info.velocity, false);
		}
	}
}