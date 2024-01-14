#pragma once
#include "SimpleIni.h"
#include "API/TrueDirectionalMovementAPI.h"

class Config
{
public:
	
	//Axe throw
	static inline float ThrowSpeed					= 3600.f;		//	cm/s
	static inline float ThrowRotationSpeed			= 1440.f;		//	deg/s
	static inline std::string ThrowEvent			= "ThrowWeapon";
	static inline std::string ThrowEventV			= "ThrowWeaponV";
	static inline std::string ThrowEndEvent			= "ThrowAttackEnd";
	
	//Axe call
	static inline float	ArrivalTime					= 0.53f;		//	s
	static inline float	ArrivalRotationSpeed		= 18.f;			//	deg/s
	static inline float	ArrivalRoadCurveMagnitude	= 12.f;			//	deg
	static inline float	CatchingTreshold			= 69.f;			//	cm
	static inline std::string CallEvent				= "CallWeapon";
	static inline std::string CallAttackEvent		= "LeviCallAttack";
	static inline std::string ChargeLeviEvent		= "LeviCharge";
	static inline bool	DontDamageWhileArrive		= false;

	//Limit values
	static inline float MaxArrivalSpeed				= 16000.f;		//	cm/s
	static inline float MinArrivalSpeed				= 1800.f;		//	cm/s
	
	static inline float MaxAxeStuckAngle			= 10.f;			//	cm/s
	static inline float MinAxeStuckAngle			= -80.f;		//	cm/s
	
	static inline float ProjCastDelay				= 1.1f;			//	s
	
	//Draupnir spear
	static inline uint32_t DraupnirSpearCount		= 9;			 //	count of spear explodable projectiles (max 9)
	static inline std::string DraupnirsCallEvent	= "DraupnirsCall";
	static inline std::string SpearThrowEvent		= "ThrowSpear";
	
	//Weapon register values
	static inline std::string LeviathanModESP		= "KratosCombatbyAnA.esp";
	static inline std::string DraupnirModESP		= "Draupnir Spear Mechanic.esp";
	
	static inline std::string LeviathanAxeKeyword	= "LeviathanAxeKWD";
	static inline std::string BladeOfChaosKeyword	= "BladeOfChaosKWD";
	static inline std::string DraupnirSpearKeyword	= "DraupnirSpearKWD";

	static inline RE::SpellItem* SpellBHstate	= nullptr;
	
	static void CheckConfig();
	static void CheckProjectiles();

private:
	static void ReadStringSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting);
	static void ReadFloatSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, float& a_setting);
	static void ReadIntSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, uint32_t& a_setting);
//	static void ReadBoolSetting(CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, bool& a_setting);
};
/**/
class APIs
{
public:
	static inline TDM_API::IVTDM1* tdm = nullptr;
	static inline TDM_API::APIResult result;

	static void Request();
};