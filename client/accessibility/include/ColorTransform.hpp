#pragma once

#include "ColorBlindnessMode.hpp"
#include <SFML/Graphics/Color.hpp>
#include <array>

namespace accessibility {

/**
 * @brief Classe responsable de la transformation des couleurs selon le mode de daltonisme
 * 
 * Utilise des matrices de transformation basées sur les recherches scientifiques
 * sur la perception des couleurs pour simuler et corriger la vision daltonienne.
 * 
 * Références:
 * - Brettel, H., Viénot, F., & Mollon, J. D. (1997)
 * - Machado, G. M., Oliveira, M. M., & Fernandes, L. A. (2009)
 */
class ColorTransform {
public:
    /**
     * @brief Transforme une couleur SFML selon le mode de daltonisme actif
     * 
     * @param original Couleur originale
     * @param mode Mode de daltonisme à appliquer
     * @return Couleur transformée
     */
    static sf::Color transform(const sf::Color& original, ColorBlindnessMode mode);

    /**
     * @brief Vérifie si deux couleurs sont suffisamment distinctes pour un mode donné
     * 
     * @param color1 Première couleur
     * @param color2 Deuxième couleur
     * @param mode Mode de daltonisme
     * @param threshold Seuil de différence minimum (0-255)
     * @return true si les couleurs sont distinguables
     */
    static bool areColorsDistinguishable(const sf::Color& color1, const sf::Color& color2,
                                          ColorBlindnessMode mode, float threshold = 40.0f);

    /**
     * @brief Augmente le contraste d'une couleur par rapport au fond
     * 
     * @param foreground Couleur de premier plan
     * @param background Couleur de fond
     * @param factor Facteur d'augmentation du contraste (1.0 = normal, 2.0 = double)
     * @return Couleur avec contraste augmenté
     */
    static sf::Color enhanceContrast(const sf::Color& foreground, 
                                      const sf::Color& background,
                                      float factor = 1.5f);

    /**
     * @brief Calcule la luminance perçue d'une couleur (0-255)
     * 
     * Utilise la formule Rec. 709 pour le calcul de luminance perçue.
     * @param color Couleur SFML
     * @return Valeur de luminance entre 0.0 et 255.0
     */
    static float calculateLuminance(const sf::Color& color);

private:
    /**
     * @brief Convertit RGB en espace linéaire pour les transformations
     */
    static std::array<float, 3> rgbToLinear(const sf::Color& color);

    /**
     * @brief Convertit de l'espace linéaire vers RGB
     */
    static sf::Color linearToRgb(const std::array<float, 3>& linear);

    /**
     * @brief Applique une matrice de transformation 3x3 à un vecteur RGB
     */
    static std::array<float, 3> applyMatrix(const std::array<float, 3>& rgb,
                                             const std::array<std::array<float, 3>, 3>& matrix);

    static const std::array<std::array<float, 3>, 3> PROTANOPIA_MATRIX;
    static const std::array<std::array<float, 3>, 3> DEUTERANOPIA_MATRIX;
    static const std::array<std::array<float, 3>, 3> TRITANOPIA_MATRIX;
};

}  // namespace accessibility
