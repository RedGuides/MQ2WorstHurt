// Uncomment this if using Builder, as it will have correct structs
//#define USING_BUILDER

#include "../MQ2Plugin.h"

PLUGIN_VERSION(0.2);
PreSetup("MQ2WorstHurt");

BOOL dataWorstHurt(PCHAR szIndex, MQ2TYPEVAR& Ret);

BOOL dataWorstHurt(PCHAR szIndex, MQ2TYPEVAR& Ret)
{
	// szIndex format:
	// <group|xtarget|both>,<n>,<radius>,<include pets>

	// Default argument values
	auto includeGroup = true;
	auto includeXTarget = true;
	auto n = 1;
	auto radius = 999999.0f;
	auto includePets = true;

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
		if (!pSpawn || GetDistance((PSPAWNINFO)pCharSpawn, pSpawn) > radius)
			return;

		// Don't add things twice, this could happen if a group member is on xtarget
		for each (auto kvp in spawns)
			if (kvp.second == pSpawn)
				return;

#ifdef USING_BUILDER
		if (!(GetSpawnType(pSpawn) == PC || GetSpawnType(pSpawn) == PET))
			return;

		auto pctHPs = pSpawn->HPMax > 0 ? 100.0f * (float)pSpawn->HPCurrent / (float)pSpawn->HPMax : 0.0f;
		spawns.insert(std::pair<float, PSPAWNINFO>(pctHPs, pSpawn));
		if (includePets && pSpawn->PetID)
		{
			pSpawn = (PSPAWNINFO)GetSpawnByID(pSpawn->PetID);
			if (pSpawn)
			{
				auto pctHPs = pSpawn->HPMax > 0 ? 100.0f * (float)pSpawn->HPCurrent / (float)pSpawn->HPMax : 0.0f;
				spawns.insert(std::pair<float, PSPAWNINFO>(pctHPs, pSpawn));
			}
		}
#else
		MQ2VARPTR varPtr = { 0 };
		MQ2TYPEVAR ret = { 0 };

		varPtr.Ptr = pSpawn;

		// Sanity check to make sure this is actually a PC/Pet
		if (!pSpawnType->GetMember(varPtr, "Type", "", ret) || (_stricmp("Pet", (char*)ret.Ptr) && _stricmp("PC", (char*)ret.Ptr)))
			return;

		if (pSpawnType->GetMember(varPtr, "PctHPs", "", ret))
			spawns.insert(std::pair<float, PSPAWNINFO>((float)ret.Int64, pSpawn));

		if (includePets)
		{
			// GetMember(Pet) will set Ptr to PetSpawn if there's no pet
			pSpawnType->GetMember(varPtr, "Pet", "", ret);
			if (ret.Ptr != &PetSpawn)
			{
				varPtr.Ptr = ret.Ptr;

				// Sanity check to make sure this is actually a pet
				if (!pSpawnType->GetMember(varPtr, "Type", "", ret) || _stricmp("Pet", (char*)ret.Ptr))
					return;

				if (pSpawnType->GetMember(varPtr, "PctHPs", "", ret))
					spawns.insert(std::pair<float, PSPAWNINFO>((float)ret.Int64, (PSPAWNINFO)varPtr.Ptr));
			}
		}
#endif
	};

	// Always include self
	AddSpawn((PSPAWNINFO)pCharSpawn);

	// Add group members if set to, and we're in a group
	if (includeGroup && pChar && pChar->pGroupInfo) // pGroupInfo is at 0x2844 in my EQData.h from test, and in the live exe, so this works
		for (auto nMember = 0; nMember < 6; nMember++)
			if (pChar->pGroupInfo->pMember[nMember])
				AddSpawn(pChar->pGroupInfo->pMember[nMember]->pSpawn);

	// Add XTargets if set to
	if (includeXTarget)
	{
		for (auto nXTarget = 0; nXTarget < 13; nXTarget++)
		{
			// pXTargetMgr is at 0x2830 in my EQData.h from test, and in the live exe, so this works
			auto xts = pChar->pXTargetMgr->XTargetSlots[nXTarget];
			if (xts.xTargetType == XTARGET_SPECIFIC_PC)
				AddSpawn((PSPAWNINFO)GetSpawnByID(xts.SpawnID));
		}
	}

	// return nth spawn in the list. Multimap sorts in ascending order, so be iterating through it n times we get the nth lowest HP spawn
	for (auto it = spawns.begin(); it != spawns.end(); ++it)
	{
		if (--n == 0)
		{
			Ret.Ptr = (*it).second;
			Ret.Type = pSpawnType;
			return true;
		}
	}

	// We could get here if we reach there are less than n items in the list
	return true;
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	AddMQ2Data("WorstHurt", dataWorstHurt);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	RemoveMQ2Data("WorstHurt");
}