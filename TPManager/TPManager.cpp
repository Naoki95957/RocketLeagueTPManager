#include "pch.h"
#include "TPManager.h"

BAKKESMOD_PLUGIN(TPManager, "This plugin provides a basic GUI for teleporting players", plugin_version, PLUGINTYPE_THREADED)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void TPManager::onLoad()
{
	_globalCvarManager = cvarManager;
	pollForInfo = true;

	cvarManager->registerNotifier("get_tpinfo", [this](std::vector<std::string> args) {
		std::vector<positionInfo> info = getPositionInfo();
		cvarManager->log("Entities: " + std::to_string(info.size()));
		for (short i = 0; i < info.size(); ++i)
		{
			cvarManager->log("Ent: " + std::to_string(i));
			cvarManager->log("Name: " + info[i].name);
			cvarManager->log("Loc: x:" + std::to_string(info[i].location.X) + " y:" + std::to_string(info[i].location.Y) + " z:" + std::to_string(info[i].location.Z));
			cvarManager->log("Rot: pitch:" + std::to_string(info[i].rotation.Pitch) + " yaw:" + std::to_string(info[i].rotation.Yaw) + " roll:" + std::to_string(info[i].rotation.Roll));
			cvarManager->log("Vel: x:" + std::to_string(info[i].velocity.X) + " y:" + std::to_string(info[i].velocity.Y) + " z:" + std::to_string(info[i].velocity.Z));
			cvarManager->log("AngVel: x:" + std::to_string(info[i].angVelocity.X) + " y:" + std::to_string(info[i].angVelocity.Y) + " z:" + std::to_string(info[i].angVelocity.Z));
		}
		}, "", 0);

	cvarManager->registerNotifier("set_ball_location", [this](std::vector<std::string> args) {
		std::vector<positionInfo> info = getPositionInfo();
		if (args.size() < 3)
		{
			cvarManager->log("req. 3 args");
			return;
		}
		float x = std::stof(args[1]);
		float y = std::stof(args[2]);
		float z = std::stof(args[3]);

		cvarManager->log("Setting pos of ball - " + std::to_string(x) + " - " + std::to_string(y) + " - " + std::to_string(z));
		gameWrapper.get()->GetCurrentGameState().GetBall().SetLocation(Vector(x, y, z));
		}, "", 0);

	getContinousInfo();
	// TODO maybe make polling a dial

	//cvarManager->log("Plugin loaded!");

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
	pollForInfo = false;
	finishedPolling.lock();
	finishedPolling.unlock();
}

void TPManager::getContinousInfo() 
{
	finishedPolling.lock();
	while (pollForInfo) {
		pollingMutex.lock();
		try
		{
			positionalInfo = pollPositionInfo();
		}
		catch (const std::exception&)
		{
			positionalInfo = std::vector<positionInfo>();
		}
		pollingMutex.unlock();
		if (pollingRateMiliseconds < 50)
			pollingRateMiliseconds = 50;
		if (pollingRateMiliseconds > 5000)
			pollingRateMiliseconds = 5000;
		std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMiliseconds));
	}
	finishedPolling.unlock();
}

std::vector<positionInfo> TPManager::getPositionInfo() { return positionalInfo; }

std::vector<positionInfo> TPManager::pollPositionInfo()
{
	if (!gameWrapper->IsInGame())
	{
		return std::vector<positionInfo>();
	}

	ServerWrapper gameState = gameWrapper.get()->GetCurrentGameState();

	//list containing all moving objects
	std::vector<positionInfo> allEntities = std::vector<positionInfo>();

	// per ball
	ArrayWrapper<BallWrapper> balls = gameState.GetGameBalls();
	if (balls.Count() > 0) {
		for (int i = 0; i < balls.Count(); ++i)
		{
			allEntities.push_back({
			balls.Get(i),
			"Ball " + std::to_string(i),
			balls.Get(i).GetLocation(),
			balls.Get(i).GetRotation(),
			balls.Get(i).GetVelocity(),
			balls.Get(i).GetAngularVelocity()
				});
		}
		numBalls = balls.Count();
	}
	
	// per car
	ArrayWrapper cars = gameState.GetCars();
	std::vector<CarWrapper> carVector = std::vector<CarWrapper>();
	for (auto car : cars) {
		carVector.push_back(car);
	}

	if (!carVector.empty())
	{
		std::sort(carVector.begin(), carVector.end(), [](CarWrapper& a, CarWrapper& b) -> bool
			{
				return a.GetOwnerName() < b.GetOwnerName();
			});


		for (int i = 0; i < carVector.size(); ++i)
		{
			auto car = carVector[i];
			// Since there currently exists a bug grabbing player names...
			std::string name = car.GetOwnerName();
			if (name.size() < 1)
			{
				name = "Player " + std::to_string(i);
			}
			allEntities.push_back({
				car,
				name,
				car.GetLocation(),
				car.GetRotation(),
				car.GetVelocity(),
				car.GetAngularVelocity()
			});
		}
	}
	return allEntities;
}

void TPManager::setPositionInfo(positionInfo info, updatePositionType updateField)
{
	ServerWrapper gameState = gameWrapper.get()->GetCurrentGameState();
	switch (updateField)
	{
		case updatePositionType::UPDATE_ALL:
			{
				gameWrapper->Execute([=, info = info](GameWrapper*) {
					ActorWrapper(info.actor.memory_address).SetLocation(info.location);
					ActorWrapper(info.actor.memory_address).SetRotation(info.rotation);
					ActorWrapper(info.actor.memory_address).SetVelocity(info.velocity);
					ActorWrapper(info.actor.memory_address).SetAngularVelocity(info.angVelocity, false);
					});
			}
			break;
		case updatePositionType::UPDATE_POSITION:
			{
				gameWrapper->Execute([=, info = info](GameWrapper*) {
					ActorWrapper(info.actor.memory_address).SetLocation(info.location);
					});
			}
			break;
		case updatePositionType::UPDATE_ROTATION:
			{
				gameWrapper->Execute([=, info = info](GameWrapper*) {
					ActorWrapper(info.actor.memory_address).SetRotation(info.rotation);
					});
			}
			break;
		case updatePositionType::UPDATE_VELOCITY:
			{
				gameWrapper->Execute([=, info = info](GameWrapper*) {
					ActorWrapper(info.actor.memory_address).SetVelocity(info.velocity);
					});
			}
			break;
		case updatePositionType::UPDATE_ANGULAR_VELOCITY:
			{
				gameWrapper->Execute([=, info = info](GameWrapper*) {
					ActorWrapper(info.actor.memory_address).SetAngularVelocity(info.angVelocity, false);
					});
			}
			break;
		default:
			break;
	}

	//if (info.name == "Ball") {
	//	BallWrapper ballW = gameState.GetBall();
	//	switch (updateField)
	//	{
	//	case updatePositionType::UPDATE_ALL:
	//	{
	//		gameWrapper->Execute([=, ballW = ballW, info = info](GameWrapper*) {
	//			ActorWrapper(ballW.memory_address).SetLocation(info.location);
	//			ActorWrapper(ballW.memory_address).SetRotation(info.rotation);
	//			ActorWrapper(ballW.memory_address).SetVelocity(info.velocity);
	//			ActorWrapper(ballW.memory_address).SetAngularVelocity(info.angVelocity, false);
	//			});
	//	}
	//		break;
	//	case updatePositionType::UPDATE_POSITION:
	//	{
	//		gameWrapper->Execute([=, ballW = ballW, info = info](GameWrapper*){
	//			ActorWrapper(ballW.memory_address).SetLocation(info.location);
	//			});
	//	}
	//		break;
	//	case updatePositionType::UPDATE_ROTATION:
	//	{
	//		gameWrapper->Execute([=, ballW = ballW, info = info](GameWrapper*) {
	//			ActorWrapper(ballW.memory_address).SetRotation(info.rotation);
	//			});
	//	}
	//		break;
	//	case updatePositionType::UPDATE_VELOCITY:
	//	{
	//		gameWrapper->Execute([=, ballW = ballW, info = info](GameWrapper*) {
	//			ActorWrapper(ballW.memory_address).SetVelocity(info.velocity);
	//			});
	//	}
	//		break;
	//	case updatePositionType::UPDATE_ANGULAR_VELOCITY:
	//	{
	//		gameWrapper->Execute([=, ballW = ballW, info = info](GameWrapper*) {
	//			ActorWrapper(ballW.memory_address).SetAngularVelocity(info.angVelocity, false);
	//			});
	//	}
	//		break;
	//	default:
	//		break;
	//	}
	//}
	//else
	//{	
	//	auto carsInGame = gameState.GetCars();
	//	for (int i = 0; i < carsInGame.Count(); ++i)
	//	{
	//		auto car = carsInGame.Get(i);
	//		if (car.GetOwnerName() != info.name)
	//		{
	//			if (info.name.find("Player ") != std::string::npos)
	//			{
	//				try
	//				{
	//					int playerID = std::stoi(info.name.substr(7, info.name.size()));
	//					if (playerID != i)
	//					{
	//						continue;
	//					}
	//					//success - sheesh
	//				}
	//				catch (const std::exception&)
	//				{
	//					continue;
	//				}
	//			}
	//			else
	//			{
	//				continue;

	//			}
	//		}
	//		
	//		switch (updateField)
	//		{
	//		case updatePositionType::UPDATE_ALL:
	//		{
	//			gameWrapper->Execute([=, car = car, info = info](GameWrapper*) {
	//				ActorWrapper(car.memory_address).SetLocation(info.location);
	//				ActorWrapper(car.memory_address).SetRotation(info.rotation);
	//				ActorWrapper(car.memory_address).SetVelocity(info.velocity);
	//				ActorWrapper(car.memory_address).SetAngularVelocity(info.angVelocity, false);
	//				});
	//		}
	//		break;
	//		case updatePositionType::UPDATE_POSITION:
	//		{
	//			gameWrapper->Execute([=, car = car, info = info](GameWrapper*) {
	//				ActorWrapper(car.memory_address).SetLocation(info.location);
	//				});
	//		}
	//			break;
	//		case updatePositionType::UPDATE_ROTATION:
	//		{
	//			gameWrapper->Execute([=, car = car, info = info](GameWrapper*) {
	//				ActorWrapper(car.memory_address).SetRotation(info.rotation);
	//				});
	//		}
	//			break;
	//		case updatePositionType::UPDATE_VELOCITY:
	//		{
	//			gameWrapper->Execute([=, car = car, info = info](GameWrapper*) {
	//				ActorWrapper(car.memory_address).SetVelocity(info.velocity);
	//				});
	//		}
	//			break;
	//		case updatePositionType::UPDATE_ANGULAR_VELOCITY:
	//		{
	//			gameWrapper->Execute([=, car = car, info = info](GameWrapper*) {
	//				ActorWrapper(car.memory_address).SetAngularVelocity(info.angVelocity, false);
	//			});
	//		}
	//			break;
	//		default:
	//			break;
	//		}
	//	}
	//}
}