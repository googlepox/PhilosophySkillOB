#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse/ParamInfos.h"
#include "obse/GameAPI.h"
#include <obse_common/SafeWrite.h>

IDebugLog gLog("PhilosophySkill.log");

const UInt32 kCalcMagickaReturnRateAddr = 0x00548D60;
float* const kFlt_B37F38 = (float*)0x00B37F38;
float* const kFlt_B37F30 = (float*)0x00B37F30;
double* const kFCostant_100 = (double*)0x00A309F0;

float g_PhilRegenMult = 1.0f;

double __cdecl Hook_Calc_MagickaReturnRate(int a1, int a2, float a3)
{
    if (a2 > 0)
        return 0.0;

    double rate = ((double)a1 * (*kFlt_B37F38) + (*kFlt_B37F30)) * (a3 / (*kFCostant_100));
    return rate * g_PhilRegenMult;
}

void Hook_InstallMagickaRegen()
{
    WriteRelJump(kCalcMagickaReturnRateAddr, (UInt32)&Hook_Calc_MagickaReturnRate);
}

bool Cmd_PhilSetMagickaRegenMult_Execute(COMMAND_ARGS)
{
    *result = 0;

    TESObjectREFR* actor = NULL;
    float mult = 1.0f;

    if (!ExtractArgs(PASS_EXTRACT_ARGS, &actor, &mult))
        return true;

    if (!actor)
        return true;

    g_PhilRegenMult = mult;

    *result = 1;
    return true;
}

const UInt32 kPlayerModExperience_XPAddr = 0x00668CA1;
const UInt32 kPlayerModExperience_XPRejoin = 0x00668CA7;

float g_PhilXPRateMult = 1.0f;

__declspec(naked) void Hook_PlayerModExperience_Multiply()
{
    __asm
    {
        fmul dword ptr[g_PhilXPRateMult]
        fstp dword ptr[esp]
        push ebx
        mov  ecx, edi
        jmp  kPlayerModExperience_XPRejoin
    }
}

void Hook_InstallXPRate()
{
    WriteRelJump(kPlayerModExperience_XPAddr, (UInt32)&Hook_PlayerModExperience_Multiply);
    SafeWrite8(kPlayerModExperience_XPAddr + 5, 0x90);
}

bool Cmd_PhilSetXPRateMult_Execute(COMMAND_ARGS)
{
    *result = 0;

    SInt32 skillID = -1;
    float mult = 1.0f;

    if (!ExtractArgs(PASS_EXTRACT_ARGS, &skillID, &mult))
        return true;

    if (skillID != -1)
    {
        return true;
    }

    g_PhilXPRateMult = mult;
    *result = 1;
    return true;
}

static ParamInfo kParams_PhilSetMagickaRegenMult[2] =
{
    { "actor",       kParamType_ActorBase, 0 },
    { "multiplier",  kParamType_Float,     0 },
};

static ParamInfo kParams_PhilSetXPRateMult[2] =
{
    { "skillID",     kParamType_Integer,   0 },
    { "multiplier",  kParamType_Float,     0 },
};

DEFINE_COMMAND_PLUGIN(PhilSetMagickaRegenMult, "sets Philosophy's magicka regen multiplier for an actor", 0, 2, kParams_PhilSetMagickaRegenMult)
DEFINE_COMMAND_PLUGIN(PhilSetXPRateMult, "sets Philosophy's skill XP rate multiplier", 0, 2, kParams_PhilSetXPRateMult)

extern "C"
{
    bool OBSEPlugin_Query(const OBSEInterface* obse, PluginInfo* info)
    {
        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = "PhilosophySkill";
        info->version = 1;

        if (!obse->isEditor)
        {
            if (obse->obseVersion < OBSE_VERSION_INTEGER)
            {
                _ERROR("OBSE version too old (got %08X need at least %08X)",
                    obse->obseVersion, OBSE_VERSION_INTEGER);
                return false;
            }
        }

        return true;
    }

    bool OBSEPlugin_Load(const OBSEInterface* obse)
    {
        obse->SetOpcodeBase(0x2830);

        obse->RegisterCommand(&kCommandInfo_PhilSetMagickaRegenMult);
        obse->RegisterCommand(&kCommandInfo_PhilSetXPRateMult);

        if (!obse->isEditor)
        {
            Hook_InstallMagickaRegen();
            Hook_InstallXPRate();
        }

        return true;
    }
};