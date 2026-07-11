#include "DamageMath.h"

#include <Windows.h>

namespace
{
	struct Settings
	{
		bool playerOnly{ true };
		bool enableNormalArmor{ true };
		bool enablePowerArmor{ true };
		bool logHits{ false };
		float normalPhysicalPercent{ 0.02F };
		float normalEnergyPercent{ 0.02F };
		float powerArmorPhysicalPercent{ 0.02F };
		float powerArmorEnergyPercent{ 0.02F };
		float maximumThreshold{ 100.0F };
	};

	Settings g_settings{};
	constexpr auto kIniPath = ".\\Data\\F4SE\\Plugins\\SafeDamageThreshold.ini";

	[[nodiscard]] float ReadFloat(const char* a_section, const char* a_key, float a_default)
	{
		char fallback[32]{};
		char value[64]{};
		std::snprintf(fallback, sizeof(fallback), "%.6f", a_default);
		GetPrivateProfileStringA(a_section, a_key, fallback, value, sizeof(value), kIniPath);
		char* end = nullptr;
		const auto parsed = std::strtof(value, std::addressof(end));
		return end != value && std::isfinite(parsed) ? parsed : a_default;
	}

	void LoadSettings()
	{
		g_settings.playerOnly = GetPrivateProfileIntA("General", "bPlayerOnly", 1, kIniPath) != 0;
		g_settings.enableNormalArmor = GetPrivateProfileIntA("General", "bEnableNormalArmorDT", 1, kIniPath) != 0;
		g_settings.enablePowerArmor = GetPrivateProfileIntA("General", "bEnablePowerArmorDT", 1, kIniPath) != 0;
		g_settings.logHits = GetPrivateProfileIntA("Debug", "bLogHits", 0, kIniPath) != 0;
		g_settings.normalPhysicalPercent = SafeDT::SanitizePercent(ReadFloat("Threshold", "fNormalPhysicalPercent", 0.02F));
		g_settings.normalEnergyPercent = SafeDT::SanitizePercent(ReadFloat("Threshold", "fNormalEnergyPercent", 0.02F));
		g_settings.powerArmorPhysicalPercent = SafeDT::SanitizePercent(ReadFloat("Threshold", "fPowerArmorPhysicalPercent", 0.02F));
		g_settings.powerArmorEnergyPercent = SafeDT::SanitizePercent(ReadFloat("Threshold", "fPowerArmorEnergyPercent", 0.02F));
		g_settings.maximumThreshold = SafeDT::SanitizeDamage(ReadFloat("Threshold", "fMaximumThreshold", 100.0F));
	}

	using DoHitMe_t = void (*)(RE::Actor*, RE::HitData&);
	DoHitMe_t g_originalDoHitMe{ nullptr };

	void ClampHitData(RE::HitData& a_hitData)
	{
		a_hitData.healthDamage = SafeDT::SanitizeDamage(a_hitData.healthDamage);
		a_hitData.totalDamage = SafeDT::SanitizeDamage(a_hitData.totalDamage);
		a_hitData.physicalDamage = SafeDT::SanitizeDamage(a_hitData.physicalDamage);
		a_hitData.targetedLimbDamage = SafeDT::SanitizeDamage(a_hitData.targetedLimbDamage);
		a_hitData.resistedPhysicalDamage = SafeDT::SanitizeDamage(a_hitData.resistedPhysicalDamage);
		a_hitData.resistedTypedDamage = SafeDT::SanitizeDamage(a_hitData.resistedTypedDamage);

		if (a_hitData.damageTypes) {
			for (auto& damageType : *a_hitData.damageTypes) {
				damageType.second.f = SafeDT::SanitizeDamage(damageType.second.f);
			}
		}
	}

	void ApplyDamageThreshold(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		ClampHitData(a_hitData);
		if (!a_target) {
			return;
		}

		if (g_settings.playerOnly && a_target != RE::PlayerCharacter::GetSingleton()) {
			return;
		}

		const auto inPowerArmor = RE::PowerArmor::ActorInPowerArmor(*a_target);
		if ((inPowerArmor && !g_settings.enablePowerArmor) || (!inPowerArmor && !g_settings.enableNormalArmor)) {
			return;
		}

		const auto actorValues = RE::ActorValue::GetSingleton();
		if (!actorValues || !actorValues->damageResistance || !actorValues->energyResistance) {
			return;
		}

		const auto physicalArmor = SafeDT::SanitizeDamage(a_target->GetActorValue(*actorValues->damageResistance));
		const auto energyArmor = SafeDT::SanitizeDamage(a_target->GetActorValue(*actorValues->energyResistance));
		const auto physicalPercent = inPowerArmor ? g_settings.powerArmorPhysicalPercent : g_settings.normalPhysicalPercent;
		const auto energyPercent = inPowerArmor ? g_settings.powerArmorEnergyPercent : g_settings.normalEnergyPercent;

		const auto physical = SafeDT::ApplyThreshold(
			a_hitData.physicalDamage,
			physicalArmor,
			physicalPercent,
			g_settings.maximumThreshold);

		float typedBefore = 0.0F;
		float energyBefore = 0.0F;
		if (a_hitData.damageTypes) {
			for (const auto& damageType : *a_hitData.damageTypes) {
				const auto damage = SafeDT::SanitizeDamage(damageType.second.f);
				typedBefore += damage;
				if (damageType.first == actorValues->energyResistance) {
					energyBefore += damage;
				}
			}
		}

		const auto energy = SafeDT::ApplyThreshold(
			energyBefore,
			energyArmor,
			energyPercent,
			g_settings.maximumThreshold);

		a_hitData.physicalDamage = physical.after;
		float typedAfter = typedBefore;
		if (a_hitData.damageTypes && energyBefore > 0.0F) {
			for (auto& damageType : *a_hitData.damageTypes) {
				if (damageType.first == actorValues->energyResistance) {
					damageType.second.f = SafeDT::ScaleDamage(damageType.second.f, energy.ratio);
				}
			}
			typedAfter = std::max(0.0F, typedBefore - (energy.before - energy.after));
		}

		const auto rawBefore = physical.before + typedBefore;
		const auto rawAfter = physical.after + typedAfter;
		const auto overallRatio = rawBefore > 0.0F ? std::clamp(rawAfter / rawBefore, 0.0F, 1.0F) : 1.0F;
		const auto typedRatio = typedBefore > 0.0F ? std::clamp(typedAfter / typedBefore, 0.0F, 1.0F) : 1.0F;

		// Keep every engine damage field consistent. This is also the PA durability fix:
		// no field passed to the original DoHitMe routine can ever be negative.
		a_hitData.healthDamage = SafeDT::ScaleDamage(a_hitData.healthDamage, overallRatio);
		a_hitData.totalDamage = SafeDT::ScaleDamage(a_hitData.totalDamage, overallRatio);
		a_hitData.targetedLimbDamage = SafeDT::ScaleDamage(a_hitData.targetedLimbDamage, overallRatio);
		a_hitData.resistedPhysicalDamage = SafeDT::ScaleDamage(a_hitData.resistedPhysicalDamage, physical.ratio);
		a_hitData.resistedTypedDamage = SafeDT::ScaleDamage(a_hitData.resistedTypedDamage, typedRatio);
		ClampHitData(a_hitData);

		if (g_settings.logHits) {
			REX::INFO(
				"target={:08X} PA={} DR={:.1f} ER={:.1f} physical={:.2f}->{:.2f} energy={:.2f}->{:.2f} ratio={:.3f}",
				a_target->GetFormID(),
				inPowerArmor,
				physicalArmor,
				energyArmor,
				physical.before,
				physical.after,
				energy.before,
				energy.after,
				overallRatio);
		}
	}

	void HookedDoHitMe(RE::Actor* a_target, RE::HitData& a_hitData)
	{
		ApplyDamageThreshold(a_target, a_hitData);
		g_originalDoHitMe(a_target, a_hitData);
	}

	bool InstallHook()
	{
		const auto target = RE::ID::Actor::DoHitMe.address();
		if (MH_Initialize() != MH_OK) {
			REX::ERROR("MinHook initialization failed");
			return false;
		}
		if (MH_CreateHook(
				reinterpret_cast<void*>(target),
				reinterpret_cast<void*>(HookedDoHitMe),
				reinterpret_cast<void**>(std::addressof(g_originalDoHitMe))) != MH_OK) {
			REX::ERROR("Could not create Actor::DoHitMe hook");
			return false;
		}
		if (MH_EnableHook(reinterpret_cast<void*>(target)) != MH_OK) {
			REX::ERROR("Could not enable Actor::DoHitMe hook");
			return false;
		}
		return true;
	}
}

F4SE_PLUGIN_PRELOAD(const F4SE::PreLoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, { .log = true, .trampoline = false, .hook = false });
	LoadSettings();
	if (!InstallHook()) {
		return false;
	}
	REX::INFO("Safe Damage Threshold preload hook installed for Fallout 4 1.11.221");
	return true;
}

