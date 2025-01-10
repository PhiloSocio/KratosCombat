#pragma once

namespace Papyrus
{
    class KratosCombatMCM
    {
    public:
        static void OnConfigClose(RE::TESQuest*);
        static void OnConfigOpen(RE::TESQuest*);

        static bool Register(RE::BSScript::IVirtualMachine* a_vm);
    };

    void Register();
    static bool eventsRegistered = false;
}
