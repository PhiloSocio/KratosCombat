#include "Papyrus.h"
#include "Settings.h"
#include "events.h"

#ifdef KRATOS_COMBAT_3
    #include "RelicManager.h"
#else
    #include "MainKratosCombat.h"
#endif

namespace Papyrus
{
    void KratosCombatMCM::OnConfigClose(RE::TESQuest*)
    {
        if (!eventsRegistered) eventsRegistered = RegisterEvents();
        
#ifdef KRATOS_COMBAT_3
        RelicManager::GetSingleton()->OnConfigClose();
#else
        WeaponIdentify::WeaponCheck(true);
#endif

        Config::CheckConfig(true);
    }
/*
    void KratosCombatMCM::OnConfigOpen(RE::TESQuest*)
    {
    //    WeaponIdentify::WeaponCheck();
    //    Config::CheckConfig();
    }
*/
    bool KratosCombatMCM::Register(RE::BSScript::IVirtualMachine* a_vm)
    {
        if (a_vm) {
            a_vm->RegisterFunction("OnConfigClose", "KratosCombatMCM", OnConfigClose);
        //    a_vm->RegisterFunction("OnConfigOpen", "KratosCombatMCM", OnConfigOpen);
            spdlog::info("Registered KratosCombatMCM");
            return true;
        } else return false;
    }

    void Register()
    {
        auto papyrus = SKSE::GetPapyrusInterface();
        papyrus->Register(KratosCombatMCM::Register);
    }
}
