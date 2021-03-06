#include "GameCommander.h"
#include "YoAgent.h"
#include "Util.h"

GameCommander::GameCommander(YoAgent & bot)
	: m_bot(bot)
	//, m_productionManager(bot)
	, m_scoutManager(bot)
	, m_combatCommander(bot)
	, m_initialScoutSet(false)
{

}

void GameCommander::onStart()
{
	//m_productionManager.onStart();
	m_scoutManager.onStart(); //this is empty
	m_combatCommander.onStart(); //sets up dummy groups for Idle, Attack, ScoutDefense //should add ProxyDefense, ScoutDenial, ExpandDenial

	drawDebugInterface();
}

void GameCommander::onFrame()
{
	m_timer.start();

	handleUnitAssignments(); //valid units, combat units, scout units

	//m_productionManager.onFrame();
	m_scoutManager.onFrame();
	m_combatCommander.onFrame(m_combatUnits);

}

void GameCommander::drawDebugInterface()
{
	drawGameInformation(4, 1);
}

void GameCommander::drawGameInformation(int x, int y)
{
	/*std::stringstream ss;
	ss << "Players: " << m_bot.Config().BotName << ":" << m_bot.Config().BotRace << " VS " << m_bot.Config().EnemyRace << ":" << m_bot.Config().EnemyDifficulty << "\n";
	ss << "Strategy: " << m_bot.Config().StrategyName << "\n";
	ss << "Map Name: " << m_bot.Config().MapName << "\n";
	//ss << "Time: " << "\n"; //use Calendar Date+Time here??
	std::cout << ss.str() << std::endl;
	*/
}

// assigns units to various managers
void GameCommander::handleUnitAssignments()
{
	m_validUnits.clear();
	m_combatUnits.clear();

	// filter our units for those which are valid and usable
	setValidUnits();

	// set each type of unit
	setScoutUnits();
	setCombatUnits();
}

bool GameCommander::isAssigned(const sc2::Unit * unit) const
{
	return     (std::find(m_combatUnits.begin(), m_combatUnits.end(), unit) != m_combatUnits.end())
		|| (std::find(m_scoutUnits.begin(), m_scoutUnits.end(), unit) != m_scoutUnits.end());
}

// validates units as usable for distribution to various managers
void GameCommander::setValidUnits()
{
	// make sure the unit is completed and alive and usable
	for (auto & unit : m_bot.m_unitInfoManager.getUnits(Players::Self))
	{
		m_validUnits.push_back(unit);
	}
}

void GameCommander::setScoutUnit(const sc2::Unit * unit)
{
	if (unit == nullptr) return;

	m_initialScoutSet = true;

	m_scoutManager.setWorkerScout(unit);
	assignUnit(unit, m_scoutUnits);
	m_initialScoutSet = true;
}

void GameCommander::setScoutUnits()
{
	// if we haven't set a scout unit, do it
	if (m_scoutUnits.empty() && !m_initialScoutSet && true) //m_bot.Config().ScoutHarassEnemy) //only send a scout if ScoutHarrassEnemy is TRUE
	{
		// if it exists
		if (shouldSendInitialScout()) //if we've built our first race-specific building already (or it is in-progress?)
		{
			// grab the closest worker to the supply provider to send to scout //<-- this gets closest to TownHall, not supply depot
			const sc2::Unit * workerScout = m_bot.m_workerManager.getClosestMineralWorkerTo(m_bot.Observation()->GetStartLocation());

			// if we find a worker (which we should) add it to the scout units
			if (workerScout)
			{
				m_scoutManager.setWorkerScout(workerScout);
				assignUnit(workerScout, m_scoutUnits);
				m_initialScoutSet = true;
			}
			else {}
		}
	}
}

bool GameCommander::shouldSendInitialScout()
{
	return true;

	switch (m_bot.m_playerRace[Players::Self])
	{
	case sc2::Race::Terran:  return m_bot.m_unitInfoManager.getUnitTypeCount(Players::Self, sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, true) > 0;
	case sc2::Race::Protoss: return m_bot.m_unitInfoManager.getUnitTypeCount(Players::Self, sc2::UNIT_TYPEID::PROTOSS_PYLON, true) > 0;
	case sc2::Race::Zerg:    return m_bot.m_unitInfoManager.getUnitTypeCount(Players::Self, sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL, true) > 0;
	default: return false;
	}
}

// sets combat units to be passed to CombatCommander
void GameCommander::setCombatUnits()
{
	for (auto & unit : m_validUnits)
	{
		//BOT_ASSERT(unit, "Have a null unit in our valid units\n");

		if (!isAssigned(unit) && Util::IsCombatUnitType(unit->unit_type, m_bot))
		{
			assignUnit(unit, m_combatUnits);
		}
	}
}

void GameCommander::onUnitCreate(const sc2::Unit * unit)
{

}

void GameCommander::onUnitDestroy(const sc2::Unit * unit)
{
	//_productionManager.onUnitDestroy(unit);
}


void GameCommander::assignUnit(const sc2::Unit * unit, std::vector<const sc2::Unit *> & units)
{
	if (std::find(m_scoutUnits.begin(), m_scoutUnits.end(), unit) != m_scoutUnits.end())
	{
		m_scoutUnits.erase(std::remove(m_scoutUnits.begin(), m_scoutUnits.end(), unit), m_scoutUnits.end());
	}
	else if (std::find(m_combatUnits.begin(), m_combatUnits.end(), unit) != m_combatUnits.end())
	{
		m_combatUnits.erase(std::remove(m_combatUnits.begin(), m_combatUnits.end(), unit), m_combatUnits.end());
	}

	units.push_back(unit);
}
