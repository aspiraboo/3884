////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#include "otpch.h"

#include "outfit.h"
#include "tools.h"

#include "player.h"
#include "condition.h"

#include "game.h"
extern Game g_game;

bool Outfits::parseOutfitNode(pugi::xml_node& p)
{
	if(std::string(p.name()).compare("outfit"))
		return false;

	pugi::xml_attribute attr;
	int32_t idOutfit;
	if((attr = p.attribute("id")))
	{
		idOutfit = attr.as_int();
	} else {
		std::clog << "[Error - Outfits::parseOutfitNode] Missing outfit id, skipping" << std::endl;
		return false;

	}

	Outfit newOutfit;
	newOutfit.outfitId = idOutfit;

	if((attr = p.attribute("default")))
		newOutfit.isDefault = booleanString(attr.as_string());

	if(!(attr = p.attribute("name")))
	{
		std::stringstream ss;
		ss << "Outfit #" << newOutfit.outfitId;
		ss >> newOutfit.name;
	}
	else
		newOutfit.name = attr.as_string();

	bool override = false;
	if((attr = p.attribute("override")))
		if(booleanString(attr.as_string()))
			override = true;

	if((attr = p.attribute("access")))
		newOutfit.accessLevel = attr.as_int();

	if((attr = p.attribute("quest")))
	{
		newOutfit.storageId = attr.as_string();
		newOutfit.storageValue = "1";
	}
	else
	{
		if((attr = p.attribute("storageId")))
			newOutfit.storageId = attr.as_string();

		if((attr = p.attribute("storageValue")))
			newOutfit.storageValue = attr.as_string();
	}

	if((attr = p.attribute("premium")))
		newOutfit.isPremium = booleanString(attr.as_string());
	
	for(auto listNode : p.children())
	{		
		if(std::string(listNode.name()).compare("list"))
			continue;

		Outfit outfit = newOutfit;
		if(!(attr = listNode.attribute("looktype")) && !(attr = listNode.attribute("lookType")))
		{
			std::clog << "[Error - Outfits::parseOutfitNode] Missing looktype for an outfit with id " << outfit.outfitId << std::endl;
			continue;
		}

		outfit.lookType = attr.as_int();
		if(!(attr = listNode.attribute("gender")) && !(attr = listNode.attribute("type")) && !(attr = listNode.attribute("sex")))
		{
			std::clog << "[Error - Outfits::parseOutfitNode] Missing gender(s) for an outfit with id " << outfit.outfitId
				<< " and looktype " << outfit.lookType << std::endl;
			continue;
		}

		IntegerVec intVector;
		if(!parseIntegerVec(attr.as_string(), intVector))
		{
			std::clog << "[Error - Outfits::parseOutfitNode] Invalid gender(s) for an outfit with id " << outfit.outfitId
				<< " and looktype " << outfit.lookType << std::endl;
			continue;
		}

		if((attr = listNode.attribute("addons")))
			outfit.addons = attr.as_int();

		if((attr = listNode.attribute("name")))
			outfit.name = attr.as_string();

		if((attr = listNode.attribute("premium")))
			outfit.isPremium = booleanString(attr.as_string());

		if((attr = listNode.attribute("requirement")))
		{
			std::string tmpStrValue = asLowerCaseString(attr.as_string());
			if(tmpStrValue == "none")
				outfit.requirement = REQUIREMENT_NONE;
			else if(tmpStrValue == "first")
				outfit.requirement = REQUIREMENT_FIRST;
			else if(tmpStrValue == "second")
				outfit.requirement = REQUIREMENT_SECOND;
			else if(tmpStrValue == "any")
				outfit.requirement = REQUIREMENT_ANY;
			else if(tmpStrValue != "both")
				std::clog << "[Warning - Outfits::loadFromXml] Unknown requirement tag value, using default (both)" << std::endl;
		}

		if((attr = listNode.attribute("manaShield")))
			outfit.manaShield = booleanString(attr.as_string());

		if((attr = listNode.attribute("invisible")))
			outfit.invisible = booleanString(attr.as_string());

		if((attr = listNode.attribute("healthGain")))
		{
			outfit.healthGain = attr.as_int();
			outfit.regeneration = true;
		}

		if((attr = listNode.attribute("healthTicks")))
		{
			outfit.healthTicks = attr.as_int();
			outfit.regeneration = true;
		}

		if((attr = listNode.attribute("manaGain")))
		{
			outfit.manaGain = attr.as_int();
			outfit.regeneration = true;
		}

		if((attr = listNode.attribute("manaTicks")))
		{
			outfit.manaTicks = attr.as_int();
			outfit.regeneration = true;
		}

		if((attr = listNode.attribute("speed")))
			outfit.speed = attr.as_int();
		
		for(auto configNode : listNode.children())
		{
			if(!std::string(configNode.name()).compare("reflect"))
			{
				if((attr = configNode.attribute("percentAll")))
				{
					for(uint32_t i = COMBAT_FIRST; i <= COMBAT_LAST; i++)
						outfit.reflect[REFLECT_PERCENT][(CombatType_t)i] += attr.as_int();
				}

				if((attr = configNode.attribute("percentElements")))
				{
					outfit.reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += attr.as_int();
				}

				if((attr = configNode.attribute("percentMagic")))
				{
					outfit.reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_HOLYDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_PERCENT][COMBAT_DEATHDAMAGE] += attr.as_int();
				}

				if((attr = configNode.attribute("percentEnergy")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentFire")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentPoison")) || (attr = configNode.attribute("percentEarth")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentIce")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentHoly")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_HOLYDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentDeath")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_DEATHDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentLifeDrain")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_LIFEDRAIN] += attr.as_int();

				if((attr = configNode.attribute("percentManaDrain")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_MANADRAIN] += attr.as_int();

				if((attr = configNode.attribute("percentDrown")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_DROWNDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentPhysical")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_PHYSICALDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentHealing")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_HEALING] += attr.as_int();

				if((attr = configNode.attribute("percentUndefined")))
					outfit.reflect[REFLECT_PERCENT][COMBAT_UNDEFINEDDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceAll")))
				{
					for(uint32_t i = COMBAT_FIRST; i <= COMBAT_LAST; i++)
						outfit.reflect[REFLECT_CHANCE][(CombatType_t)i] += attr.as_int();
				}

				if((attr = configNode.attribute("chanceElements")))
				{
					outfit.reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += attr.as_int();
				}

				if((attr = configNode.attribute("chanceMagic")))
				{
					outfit.reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_HOLYDAMAGE] += attr.as_int();
					outfit.reflect[REFLECT_CHANCE][COMBAT_DEATHDAMAGE] += attr.as_int();
				}

				if((attr = configNode.attribute("chanceEnergy")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceFire")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chancePoison")) || (attr = configNode.attribute("chanceEarth")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceIce")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceHoly")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_HOLYDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceDeath")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_DEATHDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceLifeDrain")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_LIFEDRAIN] += attr.as_int();

				if((attr = configNode.attribute("chanceManaDrain")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_MANADRAIN] += attr.as_int();

				if((attr = configNode.attribute("chanceDrown")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_DROWNDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chancePhysical")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_PHYSICALDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("chanceHealing")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_HEALING] += attr.as_int();

				if((attr = configNode.attribute("chanceUndefined")))
					outfit.reflect[REFLECT_CHANCE][COMBAT_UNDEFINEDDAMAGE] += attr.as_int();
			}			
			else if(!std::string(configNode.name()).compare("absorb"))
			{
				if((attr = configNode.attribute("percentAll")))
				{
					for(int32_t i = COMBAT_FIRST; i <= COMBAT_LAST; i++)
						outfit.absorb[(CombatType_t)i] += attr.as_int();
				}

				if((attr = configNode.attribute("percentElements")))
				{
					outfit.absorb[COMBAT_ENERGYDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_FIREDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_EARTHDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_ICEDAMAGE] += attr.as_int();
				}

				if((attr = configNode.attribute("percentMagic")))
				{
					outfit.absorb[COMBAT_ENERGYDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_FIREDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_EARTHDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_ICEDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_HOLYDAMAGE] += attr.as_int();
					outfit.absorb[COMBAT_DEATHDAMAGE] += attr.as_int();
				}

				if((attr = configNode.attribute("percentEnergy")))
					outfit.absorb[COMBAT_ENERGYDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentFire")))
					outfit.absorb[COMBAT_FIREDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentPoison")) || (attr = configNode.attribute("percentEarth")))
					outfit.absorb[COMBAT_EARTHDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentIce")))
					outfit.absorb[COMBAT_ICEDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentHoly")))
					outfit.absorb[COMBAT_HOLYDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentDeath")))
					outfit.absorb[COMBAT_DEATHDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentLifeDrain")))
					outfit.absorb[COMBAT_LIFEDRAIN] += attr.as_int();

				if((attr = configNode.attribute("percentManaDrain")))
					outfit.absorb[COMBAT_MANADRAIN] += attr.as_int();

				if((attr = configNode.attribute("percentDrown")))
					outfit.absorb[COMBAT_DROWNDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentPhysical")))
					outfit.absorb[COMBAT_PHYSICALDAMAGE] += attr.as_int();

				if((attr = configNode.attribute("percentHealing")))
					outfit.absorb[COMBAT_HEALING] += attr.as_int();

				if((attr = configNode.attribute("percentUndefined")))
					outfit.absorb[COMBAT_UNDEFINEDDAMAGE] += attr.as_int();
			}			
			else if(!std::string(configNode.name()).compare("skills"))
			{
				if((attr = configNode.attribute("fist")))
					outfit.skills[SKILL_FIST] += attr.as_int();

				if((attr = configNode.attribute("club")))
					outfit.skills[SKILL_CLUB] += attr.as_int();

				if((attr = configNode.attribute("axe")))
					outfit.skills[SKILL_AXE] += attr.as_int();

				if((attr = configNode.attribute("sword")))
					outfit.skills[SKILL_SWORD] += attr.as_int();

				if((attr = configNode.attribute("distance")) || (attr = configNode.attribute("dist")))
					outfit.skills[SKILL_DIST] += attr.as_int();

				if((attr = configNode.attribute("shielding")) || (attr = configNode.attribute("shield")))
					outfit.skills[SKILL_SHIELD] = attr.as_int();

				if((attr = configNode.attribute("fishing")) || (attr = configNode.attribute("fish")))
					outfit.skills[SKILL_FISH] = attr.as_int();

				if((attr = configNode.attribute("melee")))
				{
					outfit.skills[SKILL_FIST] += attr.as_int();
					outfit.skills[SKILL_CLUB] += attr.as_int();
					outfit.skills[SKILL_SWORD] += attr.as_int();
					outfit.skills[SKILL_AXE] += attr.as_int();
				}

				if((attr = configNode.attribute("weapon")) || (attr = configNode.attribute("weapons")))
				{
					outfit.skills[SKILL_CLUB] += attr.as_int();
					outfit.skills[SKILL_SWORD] += attr.as_int();
					outfit.skills[SKILL_AXE] += attr.as_int();
					outfit.skills[SKILL_DIST] += attr.as_int();
				}

				if((attr = configNode.attribute("fistPercent")))
					outfit.skillsPercent[SKILL_FIST] += attr.as_int();

				if((attr = configNode.attribute("clubPercent")))
					outfit.skillsPercent[SKILL_CLUB] += attr.as_int();

				if((attr = configNode.attribute("swordPercent")))
					outfit.skillsPercent[SKILL_SWORD] += attr.as_int();

				if((attr = configNode.attribute("axePercent")))
					outfit.skillsPercent[SKILL_AXE] += attr.as_int();

				if((attr = configNode.attribute("distancePercent")) || (attr = configNode.attribute("distPercent")))
					outfit.skillsPercent[SKILL_DIST] += attr.as_int();

				if((attr = configNode.attribute("shieldingPercent")) || (attr = configNode.attribute("shieldPercent")))
					outfit.skillsPercent[SKILL_SHIELD] = attr.as_int();

				if((attr = configNode.attribute("fishingPercent")) || (attr = configNode.attribute("fishPercent")))
					outfit.skillsPercent[SKILL_FISH] = attr.as_int();

				if((attr = configNode.attribute("meleePercent")))
				{
					outfit.skillsPercent[SKILL_FIST] += attr.as_int();
					outfit.skillsPercent[SKILL_CLUB] += attr.as_int();
					outfit.skillsPercent[SKILL_SWORD] += attr.as_int();
					outfit.skillsPercent[SKILL_AXE] += attr.as_int();
				}

				if((attr = configNode.attribute("weaponPercent")) || (attr = configNode.attribute("weaponsPercent")))
				{
					outfit.skillsPercent[SKILL_CLUB] += attr.as_int();
					outfit.skillsPercent[SKILL_SWORD] += attr.as_int();
					outfit.skillsPercent[SKILL_AXE] += attr.as_int();
					outfit.skillsPercent[SKILL_DIST] += attr.as_int();
				}
			}			
			else if(!std::string(configNode.name()).compare("stats"))
			{
				if((attr = configNode.attribute("maxHealth")))
					outfit.stats[STAT_MAXHEALTH] = attr.as_int();

				if((attr = configNode.attribute("maxMana")))
					outfit.stats[STAT_MAXMANA] = attr.as_int();

				if((attr = configNode.attribute("soul")))
					outfit.stats[STAT_SOUL] = attr.as_int();

				if((attr = configNode.attribute("level")))
					outfit.stats[STAT_LEVEL] = attr.as_int();

				if((attr = configNode.attribute("magLevel")) ||
					(attr = configNode.attribute("magicLevel")))
					outfit.stats[STAT_MAGICLEVEL] = attr.as_int();

				if((attr = configNode.attribute("maxHealthPercent")))
					outfit.statsPercent[STAT_MAXHEALTH] = attr.as_int();

				if((attr = configNode.attribute("maxManaPercent")))
					outfit.statsPercent[STAT_MAXMANA] = attr.as_int();

				if((attr = configNode.attribute("soulPercent")))
					outfit.statsPercent[STAT_SOUL] = attr.as_int();

				if((attr = configNode.attribute("levelPercent")))
					outfit.statsPercent[STAT_LEVEL] = attr.as_int();

				if((attr = configNode.attribute("magLevelPercent")) ||
					(attr = configNode.attribute("magicLevelPercent")))
					outfit.statsPercent[STAT_MAGICLEVEL] = attr.as_int();
			}			
			else if(!std::string(configNode.name()).compare("suppress"))
			{
				if((attr = configNode.attribute("poison")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_POISON;

				if((attr = configNode.attribute("fire")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_FIRE;

				if((attr = configNode.attribute("energy")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_ENERGY;

				if((attr = configNode.attribute("physical")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_PHYSICAL;

				if((attr = configNode.attribute("haste")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_HASTE;

				if((attr = configNode.attribute("paralyze")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_PARALYZE;

				if((attr = configNode.attribute("outfit")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_OUTFIT;

				if((attr = configNode.attribute("invisible")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_INVISIBLE;

				if((attr = configNode.attribute("light")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_LIGHT;

				if((attr = configNode.attribute("manaShield")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_MANASHIELD;

				if((attr = configNode.attribute("infight")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_INFIGHT;

				if((attr = configNode.attribute("drunk")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_DRUNK;

				if((attr = configNode.attribute("exhaust")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_EXHAUST;

				if((attr = configNode.attribute("regeneration")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_REGENERATION;

				if((attr = configNode.attribute("soul")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_SOUL;

				if((attr = configNode.attribute("drown")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_DROWN;

				if((attr = configNode.attribute("muted")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_MUTED;

				if((attr = configNode.attribute("attributes")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_ATTRIBUTES;

				if((attr = configNode.attribute("freezing")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_FREEZING;

				if((attr = configNode.attribute("dazzled")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_DAZZLED;

				if((attr = configNode.attribute("cursed")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_CURSED;

				if((attr = configNode.attribute("pacified")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_PACIFIED;

				if((attr = configNode.attribute("gamemaster")))
					if(booleanString(attr.as_string()))
						outfit.conditionSuppressions |= CONDITION_GAMEMASTER;
			}
		}

		bool add = false;
		OutfitMap::iterator fit;
		for(IntegerVec::iterator it = intVector.begin(); it != intVector.end(); ++it)
		{
			fit = outfitsMap[(*it)].find(outfit.outfitId);
			if(fit != outfitsMap[(*it)].end())
			{
				if(override)
				{
					fit->second = outfit;
					if(!add)
						add = true;
				}
				else
					std::clog << "[Warning - Outfits::parseOutfitNode] Duplicated outfit for gender " << (*it) << " with lookType " << outfit.outfitId << std::endl;
			}
			else
			{
				outfitsMap[(*it)][outfit.outfitId] = outfit;
				if(!add)
					add = true;
			}
		}

		if(add)
			allOutfits.push_back(outfit);
	}

	return true;
}

bool Outfits::loadFromXml()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(getFilePath(FILE_TYPE_XML, "outfits.xml").c_str());	
	if(!result)
	{
		std::clog << "[Warning - Outfits::loadFromXml] Cannot load outfits file, using defaults." << std::endl;		
		return false;
	}

	if(!std::string(doc.name()).compare("outfits"))
	{
		std::clog << "[Error - Outfits::loadFromXml] Malformed outfits file." << std::endl;		
		return false;
	}

	for(auto p : doc.child("outfits").children())
	{
		parseOutfitNode(p);		
	}

	return true;
}

uint32_t Outfits::getOutfitId(uint32_t lookType)
{
	for(OutfitList::iterator it = allOutfits.begin(); it != allOutfits.end(); ++it)
	{
		if(it->lookType == lookType)
			return it->outfitId;
	}

	return 0;
}

bool Outfits::getOutfit(uint32_t lookType, Outfit& outfit)
{
	for(OutfitList::iterator it = allOutfits.begin(); it != allOutfits.end(); ++it)
	{
		if(it->lookType != lookType)
			continue;

		outfit = *it;
		return true;
	}

	return false;
}

bool Outfits::getOutfit(uint32_t outfitId, uint16_t sex, Outfit& outfit)
{
	OutfitMap map = outfitsMap[sex];
	OutfitMap::iterator it = map.find(outfitId);
	if(it == map.end())
		return false;

	outfit = it->second;
	return true;
}

bool Outfits::addAttributes(uint32_t playerId, uint32_t outfitId, uint16_t sex, uint16_t addons)
{
	Player* player = g_game.getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	OutfitMap map = outfitsMap[sex];
	OutfitMap::iterator it = map.find(outfitId);
	if(it == map.end())
		return false;

	Outfit outfit = it->second;
	if(outfit.requirement != (AddonRequirement_t)addons && (outfit.requirement != REQUIREMENT_ANY || !addons))
		return false;

	if(outfit.invisible)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_OUTFIT, CONDITION_INVISIBLE, -1, 0);
		player->addCondition(condition);
	}

	if(outfit.manaShield)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_OUTFIT, CONDITION_MANASHIELD, -1, 0);
		player->addCondition(condition);
	}

	if(outfit.speed)
		g_game.changeSpeed(player, outfit.speed);

	if(outfit.conditionSuppressions)
	{
		player->setConditionSuppressions(outfit.conditionSuppressions, false);
		player->sendIcons();
	}

	if(outfit.regeneration)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_OUTFIT, CONDITION_REGENERATION, -1, 0);
		if(outfit.healthGain)
			condition->setParam(CONDITIONPARAM_HEALTHGAIN, outfit.healthGain);

		if(outfit.healthTicks)
			condition->setParam(CONDITIONPARAM_HEALTHTICKS, outfit.healthTicks);

		if(outfit.manaGain)
			condition->setParam(CONDITIONPARAM_MANAGAIN, outfit.manaGain);

		if(outfit.manaTicks)
			condition->setParam(CONDITIONPARAM_MANATICKS, outfit.manaTicks);

		player->addCondition(condition);
	}

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(outfit.skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, outfit.skills[i]);
		}

		if(outfit.skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, (int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((outfit.skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(outfit.stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, outfit.stats[s]);
		}

		if(outfit.statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, (int32_t)(player->getDefaultStats((stats_t)s) * ((outfit.statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();

	return true;
}

bool Outfits::removeAttributes(uint32_t playerId, uint32_t outfitId, uint16_t sex)
{
	Player* player = g_game.getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	OutfitMap map = outfitsMap[sex];
	OutfitMap::iterator it = map.find(outfitId);
	if(it == map.end())
		return false;

	Outfit outfit = it->second;
	if(outfit.invisible)
		player->removeCondition(CONDITION_INVISIBLE, CONDITIONID_OUTFIT);

	if(outfit.manaShield)
		player->removeCondition(CONDITION_MANASHIELD, CONDITIONID_OUTFIT);

	if(outfit.speed)
		g_game.changeSpeed(player, -outfit.speed);

	if(outfit.conditionSuppressions)
	{
		player->setConditionSuppressions(outfit.conditionSuppressions, true);
		player->sendIcons();
	}

	if(outfit.regeneration)
		player->removeCondition(CONDITION_REGENERATION, CONDITIONID_OUTFIT);

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(outfit.skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -outfit.skills[i]);
		}

		if(outfit.skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -(int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((outfit.skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(outfit.stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -outfit.stats[s]);
		}

		if(outfit.statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -(int32_t)(player->getDefaultStats((stats_t)s) * ((outfit.statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();

	return true;
}

int16_t Outfits::getOutfitAbsorb(uint32_t lookType, uint16_t sex, CombatType_t combat)
{
	OutfitMap map = outfitsMap[sex];
	if(!map.size())
		return 0;

	for(OutfitMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		if(it->second.lookType == lookType)
			return it->second.absorb[combat];
	}

	return 0;
}

int16_t Outfits::getOutfitReflect(uint32_t lookType, uint16_t sex, CombatType_t combat)
{
	OutfitMap map = outfitsMap[sex];
	if(!map.size())
		return 0;

	for(OutfitMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		if(it->second.lookType != lookType)
			continue;

		if(it->second.reflect[REFLECT_PERCENT][combat] && random_range(1, 100) < it->second.reflect[REFLECT_CHANCE][combat])
			return it->second.reflect[REFLECT_PERCENT][combat];
	}

	return 0;
}
