#pragma once

class Thresholds final
{
public:
    Thresholds(float tempMin, float tempMax,
               float tempCritLo, float tempCritHi,
               float doWarning, float doCritical)
        : m_temp_min(tempMin)
        , m_temp_max(tempMax)
        , m_temp_critical_lo(tempCritLo)
        , m_temp_critical_hi(tempCritHi)
        , m_do_warning(doWarning)
        , m_do_critical(doCritical)
    {
    }

    ~Thresholds() = default;

    Thresholds(const Thresholds&)            = default;
    Thresholds& operator=(const Thresholds&) = default;

    [[nodiscard]] float tempMin()        const noexcept { return m_temp_min; }
    [[nodiscard]] float tempMax()        const noexcept { return m_temp_max; }
    [[nodiscard]] float tempCriticalLo() const noexcept { return m_temp_critical_lo; }
    [[nodiscard]] float tempCriticalHi() const noexcept { return m_temp_critical_hi; }
    [[nodiscard]] float doWarning()      const noexcept { return m_do_warning; }
    [[nodiscard]] float doCritical()     const noexcept { return m_do_critical; }

    [[nodiscard]] bool isCritical(float temp, float doVal) const noexcept
    {
        return temp < m_temp_critical_lo
            || temp > m_temp_critical_hi
            || doVal < m_do_critical;
    }

    // Note: this overlaps the critical range. Callers must check isCritical() first.
    [[nodiscard]] bool isWarning(float temp, float doVal) const noexcept
    {
        return temp < m_temp_min
            || temp > m_temp_max
            || doVal < m_do_warning;
    }

    [[nodiscard]] bool isNormal(float temp, float doVal) const noexcept
    {
        return temp >= m_temp_min
            && temp <= m_temp_max
            && doVal >= m_do_warning;
    }

private:
    float m_temp_min;
    float m_temp_max;
    float m_temp_critical_lo;
    float m_temp_critical_hi;
    float m_do_warning;
    float m_do_critical;
};
