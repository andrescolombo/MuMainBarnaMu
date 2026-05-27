#pragma once

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

// Lightweight JSON parser for simple key-value translations
namespace i18n {

// Translation domain (separate namespaces for editor, game, metadata)
enum class Domain {
#ifdef _EDITOR
    Editor,     // Only available in debug/editor builds
    Metadata,   // Only available in debug/editor builds
#endif
    Game        // Always available (both debug and release)
};

// Main translator class
class Translator {
public:
    static Translator& GetInstance();

    // Load translations from JSON file
    bool LoadTranslations(Domain domain, const std::wstring& filePath);

    // Set current locale
    void SetLocale(const std::string& locale);

    // Get current locale
    const std::string& GetLocale() const { return m_currentLocale; }

    // Translate a key with fallback
    const char* Translate(Domain domain, const char* key, const char* fallback = nullptr) const;

    // Check if translation exists
    bool HasTranslation(Domain domain, const char* key) const;

    // Format string with arguments
    std::string Format(Domain domain, const char* key, const std::vector<std::string>& args) const;

    // Switch to a different language (reloads all translation files)
    bool SwitchLanguage(const std::string& locale);

    // Get list of available locales by scanning translation directories
    std::vector<std::string> GetAvailableLocales() const;

    // Get display name for a locale from its editor.json file
    std::string GetLanguageDisplayName(const std::string& locale) const;

    // Clear all translations
    void Clear();

private:
    Translator() : m_currentLocale("en") {}
    ~Translator() = default;

    // Prevent copying
    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;

    // Parse simple JSON file
    bool ParseJsonFile(const std::wstring& filePath, std::map<std::string, std::string>& outMap);

    // Replace placeholders in format string
    std::string ReplacePlaceholders(const std::string& format, const std::vector<std::string>& args) const;

    std::string m_currentLocale;

    // Separate translation maps for each domain
#ifdef _EDITOR
    std::map<std::string, std::string> m_editorTranslations;   // Debug/Editor only
    std::map<std::string, std::string> m_metadataTranslations; // Debug/Editor only
#endif
    std::map<std::string, std::string> m_gameTranslations;      // Always available
};

// Convenience functions

// Game translations - always available
inline const char* TranslateGame(const char* key, const char* fallback = nullptr) {
    return Translator::GetInstance().Translate(Domain::Game, key, fallback);
}

namespace GameKeys {
inline constexpr char HelperSessionStatusTitle[] = "helper_session_status.title";
inline constexpr char HelperSessionStatusSessionTime[] = "helper_session_status.session_time";
inline constexpr char HelperSessionStatusCurrentSpot[] = "helper_session_status.current_spot";
inline constexpr char HelperSessionStatusTotalDamage[] = "helper_session_status.total_damage";
inline constexpr char HelperSessionStatusNormalDamage[] = "helper_session_status.normal_damage";
inline constexpr char HelperSessionStatusBonusDamage[] = "helper_session_status.bonus_damage";
inline constexpr char HelperSessionStatusElementalDamage[] = "helper_session_status.elemental_damage";
inline constexpr char HelperSessionStatusAverageDps[] = "helper_session_status.average_dps";
inline constexpr char HelperSessionStatusPeakDps[] = "helper_session_status.peak_dps";
inline constexpr char HelperSessionStatusNormalDps[] = "helper_session_status.normal_dps";
inline constexpr char HelperSessionStatusBonusDps[] = "helper_session_status.bonus_dps";
inline constexpr char HelperSessionStatusElementalDps[] = "helper_session_status.elemental_dps";
inline constexpr char HelperSessionStatusKills[] = "helper_session_status.kills";
inline constexpr char HelperSessionStatusKillsPerMin[] = "helper_session_status.kills_per_min";
inline constexpr char HelperSessionStatusAverageKillTime[] = "helper_session_status.average_kill_time";
inline constexpr char HelperSessionStatusExpGained[] = "helper_session_status.exp_gained";
inline constexpr char HelperSessionStatusExpPerHour[] = "helper_session_status.exp_per_hour";
inline constexpr char HelperSessionStatusZenPicked[] = "helper_session_status.zen_picked";
inline constexpr char HelperSessionStatusZenPerHour[] = "helper_session_status.zen_per_hour";
inline constexpr char HelperSessionStatusJewelsPicked[] = "helper_session_status.jewels_picked";
inline constexpr char HelperSessionStatusPotionsUsed[] = "helper_session_status.potions_used";
inline constexpr char HelperSessionStatusHpPotionsUsed[] = "helper_session_status.hp_potions_used";
inline constexpr char HelperSessionStatusMpPotionsUsed[] = "helper_session_status.mp_potions_used";
inline constexpr char HelperSessionStatusIdleTime[] = "helper_session_status.idle_time";
inline constexpr char HelperSessionStatusStuckTime[] = "helper_session_status.stuck_time";
inline constexpr char HelperSessionStatusNoTargetTime[] = "helper_session_status.no_target_time";
inline constexpr char HelperSessionStatusLastWarning[] = "helper_session_status.last_warning";
inline constexpr char HelperSessionStatusLastDeathReason[] = "helper_session_status.last_death_reason";
inline constexpr char HelperSessionStatusNotAvailable[] = "helper_session_status.not_available";
inline constexpr char HelperSessionStatusUnknown[] = "helper_session_status.unknown";
inline constexpr char HelperSessionStatusNone[] = "helper_session_status.none";
inline constexpr char HelperSessionStatusLowDurability[] = "helper_session_status.low_durability";
inline constexpr char HelperSessionStatusNoArrowsBolts[] = "helper_session_status.no_arrows_bolts";
inline constexpr char HelperSessionStatusNoMana[] = "helper_session_status.no_mana";
inline constexpr char HelperSessionStatusNoPotions[] = "helper_session_status.no_potions";
inline constexpr char HelperSessionStatusInventoryFull[] = "helper_session_status.inventory_full";
inline constexpr char HelperSessionStatusDisconnected[] = "helper_session_status.disconnected";
inline constexpr char HelperSessionStatusKilledByMonster[] = "helper_session_status.killed_by_monster";
inline constexpr char HelperSessionStatusKilledByPK[] = "helper_session_status.killed_by_pk";
inline constexpr char HelperSessionStatusPieces[] = "helper_session_status.pieces";
} // namespace GameKeys

#ifdef _EDITOR
// Editor and Metadata translations - only in debug/editor builds
inline const char* TranslateEditor(const char* key, const char* fallback = nullptr) {
    return Translator::GetInstance().Translate(Domain::Editor, key, fallback);
}

inline const char* TranslateMetadata(const char* key, const char* fallback = nullptr) {
    return Translator::GetInstance().Translate(Domain::Metadata, key, fallback);
}

inline bool HasTranslation(Domain domain, const char* key) {
    return Translator::GetInstance().HasTranslation(domain, key);
}

inline std::string FormatEditor(const char* key, const std::vector<std::string>& args) {
    return Translator::GetInstance().Format(Domain::Editor, key, args);
}
#endif // _EDITOR

} // namespace i18n

// Convenience macros
#define GAME_TEXT(key) i18n::TranslateGame(key, key)

#ifdef _EDITOR
#define EDITOR_TEXT(key) i18n::TranslateEditor(key, key)
#define META_TEXT(key, fallback) i18n::TranslateMetadata(key, fallback)
#endif // _EDITOR
