/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * Copyright (C) 2010-2011 Project StarGate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ScriptPCH.h"
#include "violet_hold.h"

//Spells
enum Spells
{
    SPELL_CORROSIVE_SALIVA                     = 54527,
    SPELL_OPTIC_LINK                           = 54396
};

class boss_moragg : public CreatureScript
{
public:
    boss_moragg() : CreatureScript("boss_moragg") { }

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_moraggAI (pCreature);
    }

    struct boss_moraggAI : public BossAI
    {
        boss_moraggAI(Creature *c) : BossAI(c, BOSS_MORAGG)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
        }

        uint32 uiOpticLinkTimer;
        uint32 uiCorrosiveSalivaTimer;

        void Reset()
        {
            uiOpticLinkTimer = 10000;
            uiCorrosiveSalivaTimer = 5000;

            if (instance)
            {
                if (instance->GetData(DATA_WAVE_COUNT) == 6)
                    instance->SetData(DATA_1ST_BOSS_EVENT, NOT_STARTED);
                else if (instance->GetData(DATA_WAVE_COUNT) == 12)
                    instance->SetData(DATA_2ND_BOSS_EVENT, NOT_STARTED);
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            if (instance)
            {
                if (GameObject *pDoor = instance->instance->GetGameObject(instance->GetData64(DATA_MORAGG_CELL)))
                    if (pDoor->GetGoState() == GO_STATE_READY)
                   {
                        EnterEvadeMode();
                        return;
                    }
                if (instance->GetData(DATA_WAVE_COUNT) == 6)
                    instance->SetData(DATA_1ST_BOSS_EVENT, IN_PROGRESS);
                else if (instance->GetData(DATA_WAVE_COUNT) == 12)
                    instance->SetData(DATA_2ND_BOSS_EVENT, IN_PROGRESS);
            }
        }

        void AttackStart(Unit* pWho)
        {
            if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE) || me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                return;

            if (me->Attack(pWho, true))
            {
                me->AddThreat(pWho, 0.0f);
                me->SetInCombatWith(pWho);
                pWho->SetInCombatWith(me);
                DoStartMovement(pWho);
            }
        }

        void MoveInLineOfSight(Unit* /*who*/) {}

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (uiOpticLinkTimer <= diff)
            {
                if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, SPELL_OPTIC_LINK);
                uiOpticLinkTimer = 15000;
            } else uiOpticLinkTimer -= diff;

            if (uiCorrosiveSalivaTimer <= diff)
            {
                DoCast(me->getVictim(), SPELL_CORROSIVE_SALIVA);
                uiCorrosiveSalivaTimer = 10000;
            } else uiCorrosiveSalivaTimer -= diff;

            DoMeleeAttackIfReady();
        }
        void JustDied(Unit* /*killer*/)
        {
			_JustDied();
            if (instance)
            {
                if (instance->GetData(DATA_WAVE_COUNT) == 6)
                {
                    instance->SetData(DATA_1ST_BOSS_EVENT, DONE);
                    instance->SetData(DATA_WAVE_COUNT, 7);
                }
                else if (instance->GetData(DATA_WAVE_COUNT) == 12)
                {
                    instance->SetData(DATA_2ND_BOSS_EVENT, DONE);
                    instance->SetData(DATA_WAVE_COUNT, 13);
                }
            }
        }
    };
};

void AddSC_boss_moragg()
{
    new boss_moragg();
}