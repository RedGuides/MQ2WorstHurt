#include "../MQ2Plugin.h"

PreSetup("MQ2WorstHurt");
PLUGIN_VERSION(0.2);

BOOL dataWorstHurt(PCHAR szIndex, MQ2TYPEVAR& Ret)
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
	Ret.Type = pSpawnType;
	Ret.Ptr = pCharSpawn;

	if (GetArg(szArg, szIndex, 1, FALSE, FALSE, TRUE) && strlen(szArg) > 0)
	{
		if (!_stricmp(szArg, "group"))
		{
			includeGroup = true;
			includeXTarget = false;
		}
		else if (!_stricmp(szArg, "xtarget"))
		{
			includeGroup = false;
			includeXTarget = true;
		}
		// Default settings are both, but bail out if it's not specified
		else if (_stricmp(szArg, "both"))
			return true;
	}

	if (GetArg(szArg, szIndex, 2, FALSE, FALSE, TRUE) && strlen(szArg) > 0)
	{
		char* pFound;
		n = strtoul(szArg, &pFound, 10);
		if (!pFound || n < 1)
			return true;
	}

	if (GetArg(szArg, szIndex, 3, FALSE, FALSE, TRUE) && strlen(szArg) > 0)
	{
		char* pFound;
		radius = strtof(szArg, &pFound);
		if (!pFound)
			return true;
	}

	if (GetArg(szArg, szIndex, 4, FALSE, FALSE, TRUE) && strlen(szArg) > 0)
	{
		double result;
		if (!Calculate(szArg, result))
			return true;
		includePets = result != 0.0f;
	}

	PCHARINFO pChar = GetCharInfo();

	// list of spawns, sorted by pctHPs
	std::multimap<float, PSPAWNINFO> spawns;

	// Helper function to add a spawn and its pet (if enabled) to the list
	auto AddSpawn = [&](PSPAWNINFO pSpawn) -> void {
		// Ignore spawns if they are null, outside our radius, or they're not a PC/Pet
		if (!pSpawn || GetDistance(reinterpret_cast<PSPAWNINFO>pCharSpawn, pSpawn) > radius)
			return;

		// Don't add things twice, this could happen if a group member is on xtarget
		for each (auto kvp in spawns)
			if (kvp.second == pSpawn)
				return;

		if (!(GetSpawnType(pSpawn) == PC || GetSpawnType(pSpawn) == PET))
			return;

		auto pctHPs = pSpawn->HPMax > 0 ? 100.0f * (float)pSpawn->HPCurrent / (float)pSpawn->HPMax : 0.0f;
		spawns.insert(std::pair<float, PSPAWNINFO>(pctHPs, pSpawn));
		if (includePets && pSpawn->PetID)
		{
			pSpawn = reinterpret_cast<PSPAWNINFO>(GetSpawnByID(pSpawn->PetID));
			if (pSpawn)
			{
				pctHPs = pSpawn->HPMax > 0 ? 100.0f * (float)pSpawn->HPCurrent / (float)pSpawn->HPMax : 0.0f;
				spawns.insert(std::pair<float, PSPAWNINFO>(pctHPs, pSpawn));
			}
		}
	};

	// Always include self
	AddSpawn(reinterpret_cast<PSPAWNINFO>pCharSpawn);

	// Add group members if set to, and we're in a group
	if (includeGroup && pChar && pChar->pGroupInfo)
	{
		for (auto& nMember : pChar->pGroupInfo->pMember)
		{
			AddSpawn(nMember->pSpawn);
		}
	}

	// Add XTargets if set to
	if (includeXTarget && pChar && pChar->pXTargetMgr)
	{
		// TODO :: Will need to switch to range based for loop when core is updated to support this. ArrayClass_R0 template does not have a begin function.
		for (int nXTarget = 0; nXTarget < pChar->pXTargetMgr->XTargetSlots.Count; ++nXTarget)
		{
			auto xts = pChar->pXTargetMgr->XTargetSlots[nXTarget];
			if (xts.xTargetType == XTARGET_SPECIFIC_PC)
				AddSpawn(reinterpret_cast<PSPAWNINFO>(GetSpawnByID(xts.SpawnID)));
		}
	}

	// return nth spawn in the list. Multimap sorts in ascending order, so by iterating through it n times we get the nth lowest HP spawn
	for (auto& spawn : spawns)
	{
		if (--n == 0)
		{
			Ret.Ptr = spawn.second;
			Ret.Type = pSpawnType;
			return true;
		}
	}

	// We could get here if we reach there are less than n items in the list
	return true;
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin()
{
	AddMQ2Data("WorstHurt", dataWorstHurt);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin()
{
	RemoveMQ2Data("WorstHurt");
}