#include "pch.h"
#include "TPManager.h"

BAKKESMOD_PLUGIN(TPManager, "This plugin provides a basic GUI for teleporting players", plugin_version, PLUGINTYPE_THREADED)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void TPManager::onLoad()
{
	_globalCvarManager = cvarManager;
	pollForInfo = true;

	cvarManager->registerCvar(SELECTION, "0");
	cvarManager->registerCvar(DESTINATION, "0");
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

	getContinousInfo();
}

void TPManager::onUnload()
{
	pollForInfo = false;
	finishedPolling.lock();
	finishedPolling.unlock();
	cvarManager->removeCvar(SELECTION);
	cvarManager->removeCvar(DESTINATION);
}

void TPManager::getContinousInfo() 
{
	finishedPolling.lock();
	while (pollForInfo) {
		pollingMutex.lock();
		try
		{
			positionalInfoAllEntities = pollPositionInfo();
			auto selection = cvarManager->getCvar(SELECTION);
			auto destination = cvarManager->getCvar(DESTINATION);
			if (selection.IsNull() || destination.IsNull()) {
				pollingMutex.unlock();
				continue;
			}
			if (positionalInfoAllEntities.size() != totalNumEntities)
			{
				selection.setValue(0);
				destination.setValue(0);
			}
			choiceSelection = selection.getIntValue();
			choiceDestination = destination.getIntValue();
			totalNumEntities = positionalInfoAllEntities.size();
		}
		catch (const std::exception& ex)
		{
			//positionalInfoAllEntities = std::vector<positionInfo>();
		}
		pollingMutex.unlock();
		if (pollingRateMiliseconds < 5)
			pollingRateMiliseconds = 5;
		if (pollingRateMiliseconds > 5000)
			pollingRateMiliseconds = 5000;
		std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMiliseconds));
	}
	finishedPolling.unlock();
}

std::vector<positionInfo> TPManager::getPositionInfo() { return positionalInfoAllEntities; }

void TPManager::teleportSelectionToEntity(int selection, positionInfo entity, bool above)
{
	// 0 = All entities
	// 1 = All players
	// 2 = All balls
	// 3+ -> entity listed in positionalInfo array
	if (selection == 0) {
		for (int i = 0; i < positionalInfoAllEntities.size(); ++i)
		{
			auto ent = positionalInfoAllEntities.at(i);
			if (ent.actor.memory_address != entity.actor.memory_address)
			{
				ent.location = entity.location;
				if (above)
					ent.location.Z += 250.0f * (i + 1);
				setPositionInfo(ent, updatePositionType::UPDATE_POSITION);
			}
		}
	}
	else if (selection == 1)
	{
		for (int i = numBalls; i < positionalInfoAllEntities.size(); ++i)
		{
			auto ent = positionalInfoAllEntities.at(i);
			if (ent.actor.memory_address != entity.actor.memory_address)
			{
				ent.location = entity.location;
				if (above)
					ent.location.Z += 250.0f * (i - numBalls + 1);
				setPositionInfo(ent, updatePositionType::UPDATE_POSITION);
			}
		}
	}
	else if (selection == 2)
	{
		for (int i = 0; i < numBalls; ++i)
		{
			auto ent = positionalInfoAllEntities.at(i);
			if (ent.actor.memory_address != entity.actor.memory_address)
			{
				ent.location = entity.location;
				if (above)
					ent.location.Z += 250.0f * (i + 1);
				setPositionInfo(ent, updatePositionType::UPDATE_POSITION);
			}
		}
	}
	else
	{
		int selected = selection - 3;
		auto ent = positionalInfoAllEntities.at(selected);
		if (ent.actor.memory_address != entity.actor.memory_address)
		{
			ent.location = entity.location;
			if (above)
				ent.location.Z += 250.0f;
			setPositionInfo(ent, updatePositionType::UPDATE_POSITION);
		}
	}
}
/// Shamelessly coppied off rocket-plugin
/// <summary>Gets the players in the current game.</summary>
 /// <param name="includeBots">Bool with if the output should include bots</param>
 /// <param name="mustBeAlive">Bool with if the output should only include alive players</param>
 /// <returns>List of players</returns>
std::vector<PriWrapper> TPManager::getPlayers(const bool includeBots, const bool mustBeAlive)
{
	std::vector<PriWrapper> players;
	ServerWrapper game = gameWrapper.get()->GetCurrentGameState();
	if (game.IsNull()) {
		return players;
	}

	if (mustBeAlive) {
		for (CarWrapper car : game.GetCars()) {
			if (car.IsNull() || car.GetPRI().IsNull() || (!includeBots && car.GetPRI().GetbBot())) {
				continue;
			}

			players.push_back(car.GetPRI());
		}
	}
	else {
		for (PriWrapper PRI : game.GetPRIs()) {
			if (PRI.IsNull() || (!includeBots && PRI.GetbBot())) {
				continue;
			}
			players.push_back(PRI);
		}
	}

	return players;
}

std::vector<positionInfo> TPManager::pollPositionInfo()
{
	if (!gameWrapper->IsInGame())
	{
		return std::vector<positionInfo>();
	}

	ServerWrapper gameState = gameWrapper.get()->GetCurrentGameState();
	if (gameState.IsNull())
	{
		return std::vector<positionInfo>();
	}

	// This is how I'm forcing special options used in the GUI

	//list containing all moving objects
	//+ a few special options
	std::vector<positionInfo> allEntities = std::vector<positionInfo>();
	itemsToSearch = std::vector<std::string>();
	itemsToSearch.push_back("All entities");
	itemsToSearch.push_back("All players");
	itemsToSearch.push_back("All balls");

	// List that contains all destinations
	destToSearch = std::vector<std::string>();
	destToSearch.push_back("Custom location");

	// per ball
	ArrayWrapper<BallWrapper> balls = gameState.GetGameBalls();
	if (balls.IsNull()) 
	{
		return std::vector<positionInfo>();
	}
	if (balls.Count() > 0) {
		for (int i = 0; i < balls.Count(); ++i)
		{
			std::string name = "Ball " + std::to_string(i);
			itemsToSearch.push_back(name);
			destToSearch.push_back(name);
			allEntities.push_back({
			balls.Get(i),
			name,
			balls.Get(i).GetLocation(),
			balls.Get(i).GetRotation(),
			balls.Get(i).GetVelocity(),
			balls.Get(i).GetAngularVelocity()
				});
		}
		numBalls = balls.Count();
	}
	
	// per car
	if (gameState.GetCars().IsNull() || gameState.GetPRIs().IsNull())
	{
		return std::vector<positionInfo>();
	}
	std::vector<PriWrapper> cars = getPlayers(true);
	if (!cars.empty())
	{
		std::sort(cars.begin(), cars.end(), [](PriWrapper& a, PriWrapper& b) -> bool
			{
				return a.GetPlayerName().ToString() < b.GetPlayerName().ToString();
			});

		for (int i = 0; i < cars.size(); ++i)
		{
			auto car = cars[i];
			// Since there currently exists a bug grabbing player names...
			std::string name = car.GetPlayerName().ToString();
			if (name.size() < 1)
			{
				name = "Player " + std::to_string(i);
			}
			itemsToSearch.push_back(name);
			destToSearch.push_back(name);
			allEntities.push_back({
				car.GetCar(),
				name,
				car.GetCar().GetLocation(),
				car.GetCar().GetRotation(),
				car.GetCar().GetVelocity(),
				car.GetCar().GetAngularVelocity()
			});
		}
	}
	return allEntities;
}

void TPManager::setPositionInfo(positionInfo info, updatePositionType updateField)
{
	pollingMutex.lock();
	try
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
	}
	catch (const std::exception&)
	{
		cvarManager->log("Failed to update position");
	}
	pollingMutex.unlock();
}