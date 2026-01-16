#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace accessibility {

/**
 * @brief Type de forme pour les projectiles
 */
enum class ProjectileShape { Circle, Diamond, Triangle, Square, Cross, Star };

/**
 * @brief Classe responsable du rendu des formes distinctes pour les projectiles
 *
 * Cette classe rend les projectiles joueur et ennemis visuellement distincts
 * non seulement par la couleur, mais aussi par la forme géométrique.
 * Cela améliore considérablement l'accessibilité pour les joueurs daltoniens.
 */
class ProjectileShapeRenderer {
public:
    ProjectileShapeRenderer() = default;

    /**
     * @brief Dessine un projectile avec une forme spécifique
     *
     * @param window Fenêtre de rendu
     * @param x Position X du projectile
     * @param y Position Y du projectile
     * @param size Taille du projectile
     * @param shape Forme à utiliser
     * @param fillColor Couleur de remplissage
     * @param outlineThickness Épaisseur de la bordure
     * @param outlineColor Couleur de la bordure
     */
    void drawProjectile(sf::RenderWindow& window, float x, float y, float size,
                        ProjectileShape shape, const sf::Color& fillColor,
                        float outlineThickness = 2.0f,
                        const sf::Color& outlineColor = sf::Color::White);

    /**
     * @brief Dessine un projectile joueur (cercle avec bordure)
     *
     * @param window Fenêtre de rendu
     * @param x Position X
     * @param y Position Y
     * @param size Taille
     * @param color Couleur
     */
    void drawPlayerProjectile(sf::RenderWindow& window, float x, float y, float size,
                              const sf::Color& color);

    /**
     * @brief Dessine un projectile ennemi (diamant avec bordure)
     *
     * @param window Fenêtre de rendu
     * @param x Position X
     * @param y Position Y
     * @param size Taille
     * @param color Couleur
     */
    void drawEnemyProjectile(sf::RenderWindow& window, float x, float y, float size,
                             const sf::Color& color);

    /**
     * @brief Dessine un motif additionnel sur un projectile pour plus de clarté
     *
     * @param window Fenêtre de rendu
     * @param x Position X
     * @param y Position Y
     * @param size Taille
     * @param isPlayerProjectile true pour projectile joueur, false pour ennemi
     */
    void drawProjectilePattern(sf::RenderWindow& window, float x, float y, float size,
                               bool isPlayerProjectile);

private:
    /**
     * @brief Crée une forme en diamant/losange
     */
    sf::ConvexShape createDiamond(float x, float y, float size);

    /**
     * @brief Crée une forme triangulaire
     */
    sf::ConvexShape createTriangle(float x, float y, float size);

    /**
     * @brief Crée une forme carrée
     */
    sf::RectangleShape createSquare(float x, float y, float size);

    /**
     * @brief Crée une forme en croix
     */
    void drawCross(sf::RenderWindow& window, float x, float y, float size, const sf::Color& color,
                   float thickness);

    /**
     * @brief Crée une forme en étoile à 5 branches
     */
    sf::ConvexShape createStar(float x, float y, float size);
};

}  // namespace accessibility
