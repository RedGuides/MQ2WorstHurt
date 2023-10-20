#include <mq/Plugin.h>

PreSetup("MQ2WorstHurt");
PLUGIN_VERSION(0.2);

bool dataWorstHurt(const char* szIndex, MQTypeVar& Ret)
{
	// szIndex format:
	// <group|xtarget|both>,<n>,<radius>,<include pets>

	// Default argument values
	bool includeGroup = true;
	bool includeXTarget = true;
	bool includePets = true;
	int n = 1;
	float radius = 999999;

	char szArg[MAX_STRING] = { 0 };

	// By default, return Me
	Ret = mq::datatypes::pSpawnType->MakeTypeVar(pLocalPlayer);

	if (GetArg(szArg, szIndex, 1, false, false, true) && szArg[0] != '\0')
	{
		if (!_stricmp(szArg, "group"))
		{
			includeXTarget = false;
		}
		else if (!_stricmp(szArg, "xtarget"))
		{
			includeGroup = false;
		}
		// Default settings are both, but bail out if it's not specified
		else if (_stricmp(szArg, "both"))
			return true;
	}

	if (GetArg(szArg, szIndex, 2, false, false, true) && szArg[0] != '\0')
	{
		n = GetIntFromString(szArg, 0);
		if (n < 0)
			return true;
	}

	if (GetArg(szArg, szIndex, 3, false, false, true) && szArg[0] != '\0')
	{
		radius = GetFloatFromString(szArg, -1.0f);
		if (radius < 0.0f)
			return true;
	}

	if (GetArg(szArg, szIndex, 4, false, false, true) && szArg[0] != '\0')
	{
		double result;
		if (!Calculate(szArg, result))
			return true;
		includePets = result != 0.0f;
	}

	// list of spawns, sorted by pctHPs
	std::multimap<float, PlayerClient*> spawns;

	// Helper function to add a spawn and its pet (if enabled) to the list
	auto AddSpawn = [&](PlayerClient* pSpawn) -> void {
		// Ignore spawns if they are null, outside our radius, or they're not a PC/Pet
		if (!pSpawn || GetDistance(pLocalPlayer, pSpawn) > radius)
			return;

		// Don't add things twice, this could happen if a group member is on xtarget
		for each (auto kvp in spawns)
			if (kvp.second == pSpawn)
				return;

		if (!(GetSpawnType(pSpawn) == PC || GetSpawnType(pSpawn) == PET || GetSpawnType(pSpawn) == MERCENARY))
			return;

		auto pctHPs = pSpawn->HPMax > 0 ? 100.0f * (float)pSpawn->HPCurrent / (float)pSpawn->HPMax : 0.0f;
		spawns.insert(std::pair<float, PlayerClient*>(pctHPs, pSpawn));
		if (includePets && pSpawn->PetID)
		{
			pSpawn = GetSpawnByID(pSpawn->PetID);
			if (pSpawn)
			{
				pctHPs = pSpawn->HPMax > 0 ? 100.0f * (float)pSpawn->HPCurrent / (float)pSpawn->HPMax : 0.0f;
				spawns.insert(std::pair<float, PlayerClient*>(pctHPs, pSpawn));
			}
		}
	};

	// Always include self
	AddSpawn(pLocalPlayer);

	// Add group members if set to, and we're in a group
	if (includeGroup && pLocalPC && pLocalPC->pGroupInfo)
	{
		for (CGroupMember* pMember : *pLocalPC->pGroupInfo)
		{
			if (pMember && pMember->pSpawn)
			{
				AddSpawn(pMember->pSpawn);
			}
		}
	}

	// Add XTargets if set to
	if (includeXTarget && pLocalPC && pLocalPC->pXTargetMgr)
	{
		for (auto& xts : pLocalPC->pXTargetMgr->XTargetSlots)
		{
			if (xts.xTargetType == XTARGET_SPECIFIC_PC)
			{
				AddSpawn(GetSpawnByID(xts.SpawnID));
			}
		}
	}

	// return nth spawn in the list. Multimap sorts in ascending order, so by iterating through it n times we get the nth lowest HP spawn
	for (auto& spawn : spawns)
	{
		if (--n == 0)
		{
			Ret = mq::datatypes::pSpawnType->MakeTypeVar(spawn.second);
			return true;
		}
	}

	// We could get here if we reach there are less than n items in the list
	return true;
}

// Called once, when the plugin is to initialize
PLUGIN_API void InitializePlugin()
{
	AddMQ2Data("WorstHurt", dataWorstHurt);
}

// Called once, when the plugin is to shutdown
PLUGIN_API void ShutdownPlugin()
{
	RemoveMQ2Data("WorstHurt");
}
