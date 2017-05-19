
/* ScriptData
SDName: npc_training_dummy
SD%Complete: 100
SDComment: Custom NPC Training Dummy, like in Wotlk
SDCategory: Custom
EndScriptData */

typedef std::unordered_map<uint64, uint32> AttackerMap;

struct npc_training_dummy : ScriptedAI
{
    npc_training_dummy(Creature *c) : ScriptedAI(c)
    {
        SetCombatMovementAllowed(false);
        m_entry = c->GetEntry();
    }

    uint64 m_entry;
    
    AttackerMap attackers;
    
    void Reset() override
    {
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
        me->SetHealth(me->GetMaxHealth()/5);
        me->SetStunned(true);
    }

    void ReceiveEmote(Player* player, uint32 emote) override
    {
        if (emote == TEXTEMOTE_HUG)
        {
            char w[50];
            //TODO translate "Threat against you:
            snprintf(w, 50, "Menace envers vous : %f", me->getThreatManager().getThreat(player));
            me->Whisper(w, LANG_UNIVERSAL, player);
        }
    }

    void DamageTaken(Unit* done_by, uint32& damage) override
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER)
            attackers[done_by->GetGUID()] = 8000;
        else if (done_by->ToCreature() && done_by->ToCreature()->IsPet()) {
            if (Unit* owner = done_by->ToCreature()->GetOwner())
                attackers[owner->GetGUID()] = 8000;
        }

        if (me->GetHealth() < (me->GetMaxHealth()/10.0f) || me->GetHealth() > (me->GetMaxHealth()/5.0f)) // allow players using finishers
            me->SetHealth(me->GetMaxHealth()/5);
    }

    void UpdateAI(const uint32 diff) override
    {
        for (auto itr = attackers.begin(); itr != attackers.end();)
        {
            if (itr->second <= diff)
            {
                if (Player* attacker = ObjectAccessor::GetPlayer(*me, itr->first))
                {
                    attacker->CombatStop(false);
                    attacker->AttackStop(); //little bug here: this will interrupt spells... not nice, but no so much of a problem in a capital
                    attacker->CombatStopWithPets(true);
                    attacker->ClearInCombat();
                }

                itr = attackers.erase(itr);

                if (attackers.empty())
                {
                    EnterEvadeMode();
                    me->SetHealth(me->GetMaxHealth()/5);
                }
            }
            else
            {
                itr->second -= diff;
                ++itr;
            }
        }
    }
};

CreatureAI* GetAI_npc_training_dummy(Creature* pCreature)
{
    return new npc_training_dummy(pCreature);
}

void AddSC_training_dummy()
{
    OLDScript* newscript;
    
    newscript = new OLDScript;
    newscript->Name = "npc_training_dummy";
    newscript->GetAI = &GetAI_npc_training_dummy;
    sScriptMgr->RegisterOLDScript(newscript);
}
