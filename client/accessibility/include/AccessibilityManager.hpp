#pragma once

#include "ColorBlindnessMode.hpp"
#include "ColorTransform.hpp"

#include <SFML/Graphics/Color.hpp>

#include <string>

namespace accessibility {

/**
 * @brief Gestionnaire principal du système d'accessibilité
 *
 * Singleton qui gère les paramètres d'accessibilité globaux et fournit
 * une interface unifiée pour toutes les fonctionnalités d'accessibilité.
 *
 * @example
 * ```cpp
 * auto& mgr = AccessibilityManager::instance();
 * mgr.setColorBlindMode(ColorBlindnessMode::Protanopia);
 * sf::Color adjusted = mgr.transformColor(sf::Color::Red);
 * ```
 */
class AccessibilityManager {
public:
    /**
     * @brief Obtient l'instance unique du gestionnaire (Singleton)
     */
    static AccessibilityManager& instance();

    /**
     * @brief Définit le mode de daltonisme actif
     *
     * @param mode Nouveau mode à activer
     */
    void setColorBlindMode(ColorBlindnessMode mode);

    /**
     * @brief Obtient le mode de daltonisme actuel
     *
     * @return Mode actif
     */
    ColorBlindnessMode getColorBlindMode() const { return current_mode_; }

    /**
     * @brief Active/désactive l'utilisation de formes distinctes pour les projectiles
     *
     * @param enabled true pour activer, false pour désactiver
     */
    void setProjectileShapesEnabled(bool enabled) { projectile_shapes_enabled_ = enabled; }

    /**
     * @brief Vérifie si les formes distinctes sont activées
     */
    bool isProjectileShapesEnabled() const { return projectile_shapes_enabled_; }

    /**
     * @brief Transforme une couleur selon le mode actif
     *
     * @param original Couleur originale
     * @return Couleur transformée
     */
    sf::Color transformColor(const sf::Color& original) const;

    /**
     * @brief Obtient une couleur optimisée pour les projectiles alliés
     *
     * @param original Couleur de base
     * @return Couleur ajustée pour une meilleure visibilité
     */
    sf::Color getPlayerProjectileColor(const sf::Color& original) const;

    /**
     * @brief Obtient une couleur optimisée pour les projectiles ennemis
     *
     * @param original Couleur de base
     * @return Couleur ajustée pour une meilleure visibilité
     */
    sf::Color getEnemyProjectileColor(const sf::Color& original) const;

    /**
     * @brief Obtient une couleur de bordure contrastée
     *
     * @param fillColor Couleur de remplissage
     * @return Couleur de bordure qui contraste bien
     */
    sf::Color getBorderColor(const sf::Color& fillColor) const;

    /**
     * @brief Charge les paramètres d'accessibilité depuis un fichier
     *
     * @param filepath Chemin du fichier de configuration
     * @return true si le chargement a réussi
     */
    bool loadSettings(const std::string& filepath = "settings.ini");

    /**
     * @brief Sauvegarde les paramètres d'accessibilité
     *
     * @param filepath Chemin du fichier de configuration
     * @return true si la sauvegarde a réussi
     */
    bool saveSettings(const std::string& filepath = "settings.ini") const;

    /**
     * @brief Réinitialise tous les paramètres d'accessibilité par défaut
     */
    void resetToDefaults();

    AccessibilityManager(const AccessibilityManager&) = delete;
    AccessibilityManager& operator=(const AccessibilityManager&) = delete;

private:
    AccessibilityManager();
    ~AccessibilityManager() = default;

    ColorBlindnessMode current_mode_ = ColorBlindnessMode::Normal;
    bool projectile_shapes_enabled_ = true;

    // Cache pour optimiser les transformations répétées
    mutable sf::Color cached_player_projectile_color_;
    mutable sf::Color cached_enemy_projectile_color_;
    mutable bool cache_valid_ = false;

    /**
     * @brief Invalide le cache des couleurs
     */
    void invalidateCache();
};

}  // namespace accessibility
