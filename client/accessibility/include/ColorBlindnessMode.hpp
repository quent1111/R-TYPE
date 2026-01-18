#pragma once

#include <string>

namespace accessibility {

/**
 * @brief Modes de daltonisme supportés par le système d'accessibilité
 *
 * Chaque mode applique une transformation de couleur spécifique pour améliorer
 * la jouabilité pour les personnes atteintes de différents types de daltonisme.
 */
enum class ColorBlindnessMode { Normal, Protanopia, Deuteranopia, Tritanopia, HighContrast };

/**
 * @brief Convertit un mode de daltonisme en chaîne de caractères
 */
inline const char* colorBlindnessModeToString(ColorBlindnessMode mode) {
    switch (mode) {
        case ColorBlindnessMode::Normal:
            return "Normal";
        case ColorBlindnessMode::Protanopia:
            return "Protanopia";
        case ColorBlindnessMode::Deuteranopia:
            return "Deuteranopia";
        case ColorBlindnessMode::Tritanopia:
            return "Tritanopia";
        case ColorBlindnessMode::HighContrast:
            return "HighContrast";
        default:
            return "Unknown";
    }
}

/**
 * @brief Convertit une chaîne en mode de daltonisme
 */
inline ColorBlindnessMode stringToColorBlindnessMode(const std::string& str) {
    if (str == "Protanopia")
        return ColorBlindnessMode::Protanopia;
    if (str == "Deuteranopia")
        return ColorBlindnessMode::Deuteranopia;
    if (str == "Tritanopia")
        return ColorBlindnessMode::Tritanopia;
    if (str == "HighContrast")
        return ColorBlindnessMode::HighContrast;
    return ColorBlindnessMode::Normal;
}

/**
 * @brief Convertit depuis l'enum global ColorBlindMode (client)
 */
inline ColorBlindnessMode fromClientColorBlindMode(int client_mode) {
    switch (client_mode) {
        case 0:
            return ColorBlindnessMode::Normal;
        case 1:
            return ColorBlindnessMode::Protanopia;
        case 2:
            return ColorBlindnessMode::Deuteranopia;
        case 3:
            return ColorBlindnessMode::Tritanopia;
        case 4:
            return ColorBlindnessMode::HighContrast;
        default:
            return ColorBlindnessMode::Normal;
    }
}

}  // namespace accessibility
