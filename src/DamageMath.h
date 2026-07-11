#pragma once

#include <algorithm>
#include <cmath>

namespace SafeDT
{
	struct ThresholdResult
	{
		float before{ 0.0F };
		float after{ 0.0F };
		float threshold{ 0.0F };
		float ratio{ 1.0F };
	};

	[[nodiscard]] inline float SanitizeDamage(float a_value) noexcept
	{
		return std::isfinite(a_value) ? std::max(0.0F, a_value) : 0.0F;
	}

	[[nodiscard]] inline float SanitizePercent(float a_value) noexcept
	{
		return std::isfinite(a_value) ? std::clamp(a_value, 0.0F, 1.0F) : 0.0F;
	}

	[[nodiscard]] inline ThresholdResult ApplyThreshold(
		float a_damage,
		float a_armorRating,
		float a_percent,
		float a_maxThreshold) noexcept
	{
		ThresholdResult result{};
		result.before = SanitizeDamage(a_damage);

		const auto armor = SanitizeDamage(a_armorRating);
		const auto percent = SanitizePercent(a_percent);
		const auto maximum = SanitizeDamage(a_maxThreshold);
		result.threshold = armor * percent;
		if (maximum > 0.0F) {
			result.threshold = std::min(result.threshold, maximum);
		}

		result.after = std::max(0.0F, result.before - result.threshold);
		result.ratio = result.before > 0.0F ? result.after / result.before : 1.0F;
		return result;
	}

	[[nodiscard]] inline float ScaleDamage(float a_value, float a_ratio) noexcept
	{
		const auto value = SanitizeDamage(a_value);
		const auto ratio = std::isfinite(a_ratio) ? std::clamp(a_ratio, 0.0F, 1.0F) : 0.0F;
		return value * ratio;
	}
}

