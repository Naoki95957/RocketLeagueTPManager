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

	getContinousInfo();
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
			positionalInfoAllEntities = pollPositionInfo();
		}
		catch (const std::exception&)
		{
			positionalInfoAllEntities = std::vector<positionInfo>();
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

std::vector<positionInfo> TPManager::getPositionInfo() { return positionalInfoAllEntities; }

void TPManager::teleportSelectionToEntity(int selection, positionInfo entity, bool above)
{
	// 0 = All entities
	// 1 = All players
	// 2 = All balls
	// 3+ -> entity list in positionalInfo array
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
					ent.location.Z += 250.0f * (i + 1);
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
		auto ent = positionalInfoAllEntities.at((selection - 3));
		if (ent.actor.memory_address != entity.actor.memory_address)
		{
			ent.location = entity.location;
			if (above)
				ent.location.Z += 250.0f;
			setPositionInfo(ent, updatePositionType::UPDATE_POSITION);
		}
	}
}

std::vector<positionInfo> TPManager::pollPositionInfo()
{
	if (!gameWrapper->IsInGame())
	{
		return std::vector<positionInfo>();
	}

	ServerWrapper gameState = gameWrapper.get()->GetCurrentGameState();

	//list containing all moving objects
	std::vector<positionInfo> allEntities = std::vector<positionInfo>();
	itemsToSearch = std::vector<std::string>();
	itemsToSearch.push_back("All entities");
	itemsToSearch.push_back("All players");
	itemsToSearch.push_back("All balls");

	// per ball
	ArrayWrapper<BallWrapper> balls = gameState.GetGameBalls();
	if (balls.Count() > 0) {
		for (int i = 0; i < balls.Count(); ++i)
		{
			std::string name = "Ball " + std::to_string(i);
			itemsToSearch.push_back(name);
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
			itemsToSearch.push_back(name);
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
	pollingMutex.lock();
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
	pollingMutex.unlock();
}