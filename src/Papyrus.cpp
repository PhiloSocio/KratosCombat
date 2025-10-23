#include "Papyrus.h"
#include "Settings.h"
#include "MainKratosCombat.h"

namespace Papyrus
{
    void KratosCombatMCM::OnConfigClose(RE::TESQuest*)
    {
        if (!eventsRegistered) eventsRegistered = RegisterEvents();
        WeaponIdentify::WeaponCheck(true);
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
