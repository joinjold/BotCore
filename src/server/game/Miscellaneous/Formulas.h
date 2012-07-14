/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TRINITY_FORMULAS_H
#define TRINITY_FORMULAS_H

#include "World.h"

namespace Trinity
{
    namespace Honor
    {
        inline float hk_honor_at_level_f(uint8 level, uint32 count = 1)
        {
            return count * level * 1.55f;
        }
        
        inline uint32 hk_honor_at_level(uint8 level, uint32 count = 1)
        {
            return ceil(hk_honor_at_level_f(level, count));
        }
    }
    namespace XP
    {
        enum XPColorChar { RED, ORANGE, YELLOW, GREEN, GRAY };

        inline uint8 GetGrayLevel(uint8 pl_level)
        {
            if (pl_level <= 5)
                return 0;
            else if (pl_level <= 39)
                return pl_level - 5 - pl_level/10;
            else if (pl_level <= 59)
                return pl_level - 1 - pl_level/5;
            else
                return pl_level - 9;
        }

        inline XPColorChar GetColorCode(uint8 pl_level, uint8 mob_level)
        {
            if (mob_level >= pl_level + 5)
                return RED;
            else if (mob_level >= pl_level + 3)
                return ORANGE;
            else if (mob_level >= pl_level - 2)
                return YELLOW;
            else if (mob_level > GetGrayLevel(pl_level))
                return GREEN;
            else
                return GRAY;
        }

        inline uint8 GetZeroDifference(uint8 pl_level)
        {
            if (pl_level < 8)  return 5;
            if (pl_level < 10) return 6;
            if (pl_level < 12) return 7;
            if (pl_level < 16) return 8;
            if (pl_level < 20) return 9;
            if (pl_level < 30) return 11;
            if (pl_level < 40) return 12;
            if (pl_level < 45) return 13;
            if (pl_level < 50) return 14;
            if (pl_level < 55) return 15;
            if (pl_level < 60) return 16;
            return 17;
        }

        inline uint32 BaseGain(uint8 pl_level, uint8 mob_level, ContentLevels content)
        {
            uint32 nBaseExp;
            switch (content)
            {
                case CONTENT_1_60:  nBaseExp = 45;  break;
                case CONTENT_61_70: nBaseExp = 235; break;
                case CONTENT_71_80: nBaseExp = 580; break;
                default:
                    sLog.outError("BaseGain: Unsupported content level %u",content);
                    nBaseExp = 45;  break;
            }

            if (mob_level >= pl_level)
            {
                uint8 nLevelDiff = mob_level - pl_level;
                if (nLevelDiff > 4)
                    nLevelDiff = 4;
                return ((pl_level*5 + nBaseExp) * (20 + nLevelDiff)/10 + 1)/2;
            }
            else
            {
                uint8 gray_level = GetGrayLevel(pl_level);
                if (mob_level > gray_level)
                {
                    uint8 ZD = GetZeroDifference(pl_level);
                    return (pl_level*5 + nBaseExp) * (ZD + mob_level - pl_level)/ZD;
                }
                return 0;
            }
        }

        inline uint32 Gain(Player *pl, Unit *u)
        {
            if (u->GetTypeId() == TYPEID_UNIT && (
                ((Creature*)u)->isTotem() || ((Creature*)u)->isPet() ||
                (((Creature*)u)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_XP_AT_KILL)))
                return 0;

            uint32 xp_gain = BaseGain(pl->getLevel(), u->getLevel(), GetContentLevelsForMapAndZone(u->GetMapId(),u->GetZoneId()));
            if (xp_gain == 0)
                return 0;

            //elites in instances have a 2.75x xp bonus instead of the regular 2x world bonus
            if (u->GetTypeId() == TYPEID_UNIT && ((Creature*)u)->isElite())
            {
				int normalXP, customXP;
				if(u->GetMap() && u->GetMap()->IsDungeon()) {
					normalXP=uint32(xp_gain*2.75);
					customXP=uint32(xp_gain*sWorld.getRate(RATE_XP_KILL_ELITE)*1.375);
					sLog.outString("Forumlas.h Gain() default Instance EliteXP=%i new Instance EliteXP=%i",normalXP,customXP);
					xp_gain *= sWorld.getRate(RATE_XP_KILL_ELITE)*1.375;  //default of 2 in config will yield 2.75 for instances
				} else {
					normalXP=uint32(xp_gain*2.75);
					customXP=uint32(xp_gain*sWorld.getRate(RATE_XP_KILL_ELITE));
					sLog.outString("Forumlas.h Gain() default EliteXP=%i new EliteXP=%i",normalXP,customXP);
					xp_gain *= sWorld.getRate(RATE_XP_KILL_ELITE);
				}
            }

            return uint32(xp_gain*sWorld.getRate(RATE_XP_KILL));
        }

        inline float xp_in_group_rate(uint32 count, bool isRaid)
        {
            if (isRaid)
            {
                // FIX ME: must apply decrease modifiers dependent from raid size
                return 1.0f;
            }
            else
            {
                switch (count)
                {
                    case 0:
                    case 1:
                    case 2:
                        return 1.0f;
                    case 3:
                        return 1.166f;
                    case 4:
                        return 1.3f;
                    case 5:
                    default:
                        return 1.4f;
                }
            }
        }
    }
}
#endif

