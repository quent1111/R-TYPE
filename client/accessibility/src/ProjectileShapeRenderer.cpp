#include "../include/ProjectileShapeRenderer.hpp"
#include <cmath>

namespace accessibility {

void ProjectileShapeRenderer::drawProjectile(sf::RenderWindow& window,
                                             float x, float y,
                                             float size,
                                             ProjectileShape shape,
                                             const sf::Color& fillColor,
                                             float outlineThickness,
                                             const sf::Color& outlineColor) {
    switch (shape) {
        case ProjectileShape::Circle: {
            sf::CircleShape circle(size);
            circle.setOrigin(size, size);
            circle.setPosition(x, y);
            circle.setFillColor(fillColor);
            circle.setOutlineThickness(outlineThickness);
            circle.setOutlineColor(outlineColor);
            window.draw(circle);
            break;
        }
        case ProjectileShape::Diamond: {
            auto diamond = createDiamond(x, y, size);
            diamond.setFillColor(fillColor);
            diamond.setOutlineThickness(outlineThickness);
            diamond.setOutlineColor(outlineColor);
            window.draw(diamond);
            break;
        }
        case ProjectileShape::Triangle: {
            auto triangle = createTriangle(x, y, size);
            triangle.setFillColor(fillColor);
            triangle.setOutlineThickness(outlineThickness);
            triangle.setOutlineColor(outlineColor);
            window.draw(triangle);
            break;
        }
        case ProjectileShape::Square: {
            auto square = createSquare(x, y, size);
            square.setFillColor(fillColor);
            square.setOutlineThickness(outlineThickness);
            square.setOutlineColor(outlineColor);
            window.draw(square);
            break;
        }
        case ProjectileShape::Cross: {
            drawCross(window, x, y, size, fillColor, outlineThickness);
            break;
        }
        case ProjectileShape::Star: {
            auto star = createStar(x, y, size);
            star.setFillColor(fillColor);
            star.setOutlineThickness(outlineThickness);
            star.setOutlineColor(outlineColor);
            window.draw(star);
            break;
        }
    }
}

void ProjectileShapeRenderer::drawPlayerProjectile(sf::RenderWindow& window,
                                                   float x, float y,
                                                   float size,
                                                   const sf::Color& color) {
    drawProjectile(window, x, y, size, ProjectileShape::Circle, color, 2.0f, sf::Color::White);

    sf::CircleShape center(size * 0.3f);
    center.setOrigin(size * 0.3f, size * 0.3f);
    center.setPosition(x, y);
    center.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(center);
}

void ProjectileShapeRenderer::drawEnemyProjectile(sf::RenderWindow& window,
                                                  float x, float y,
                                                  float size,
                                                  const sf::Color& color) {
    drawProjectile(window, x, y, size, ProjectileShape::Diamond, color, 2.0f, sf::Color::White);

    drawCross(window, x, y, size * 0.6f, sf::Color(255, 255, 255, 150), 2.0f);
}

void ProjectileShapeRenderer::drawProjectilePattern(sf::RenderWindow& window,
                                                    float x, float y,
                                                    float size,
                                                    bool isPlayerProjectile) {
    if (isPlayerProjectile) {
        sf::CircleShape innerCircle(size * 0.4f);
        innerCircle.setOrigin(size * 0.4f, size * 0.4f);
        innerCircle.setPosition(x, y);
        innerCircle.setFillColor(sf::Color::Transparent);
        innerCircle.setOutlineThickness(1.5f);
        innerCircle.setOutlineColor(sf::Color(255, 255, 255, 180));
        window.draw(innerCircle);
    } else {
        drawCross(window, x, y, size * 0.7f, sf::Color(255, 255, 255, 180), 2.0f);
    }
}

sf::ConvexShape ProjectileShapeRenderer::createDiamond(float x, float y, float size) {
    sf::ConvexShape diamond(4);

    diamond.setPoint(0, sf::Vector2f(0, -size));
    diamond.setPoint(1, sf::Vector2f(size, 0));
    diamond.setPoint(2, sf::Vector2f(0, size));
    diamond.setPoint(3, sf::Vector2f(-size, 0));

    diamond.setPosition(x, y);
    return diamond;
}

sf::ConvexShape ProjectileShapeRenderer::createTriangle(float x, float y, float size) {
    sf::ConvexShape triangle(3);

    float height = size * std::sqrt(3.0f);

    triangle.setPoint(0, sf::Vector2f(0, -height * 0.667f));
    triangle.setPoint(1, sf::Vector2f(size, height * 0.333f));
    triangle.setPoint(2, sf::Vector2f(-size, height * 0.333f));

    triangle.setPosition(x, y);
    return triangle;
}

sf::RectangleShape ProjectileShapeRenderer::createSquare(float x, float y, float size) {
    sf::RectangleShape square(sf::Vector2f(size * 2, size * 2));
    square.setOrigin(size, size);
    square.setPosition(x, y);
    return square;
}

void ProjectileShapeRenderer::drawCross(sf::RenderWindow& window,
                                       float x, float y,
                                       float size,
                                       const sf::Color& color,
                                       float thickness) {
    sf::RectangleShape vertical(sf::Vector2f(thickness, size * 2));
    vertical.setOrigin(thickness / 2, size);
    vertical.setPosition(x, y);
    vertical.setFillColor(color);
    window.draw(vertical);

    sf::RectangleShape horizontal(sf::Vector2f(size * 2, thickness));
    horizontal.setOrigin(size, thickness / 2);
    horizontal.setPosition(x, y);
    horizontal.setFillColor(color);
    window.draw(horizontal);
}

sf::ConvexShape ProjectileShapeRenderer::createStar(float x, float y, float size) {
    sf::ConvexShape star(10);

    const float outerRadius = size;
    const float innerRadius = size * 0.4f;
    const float angleStep = 3.14159f / 5.0f;

    for (int i = 0; i < 10; ++i) {
        float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        float angle = i * angleStep - 3.14159f / 2.0f;

        float px = radius * std::cos(angle);
        float py = radius * std::sin(angle);

        star.setPoint(i, sf::Vector2f(px, py));
    }

    star.setPosition(x, y);
    return star;
}

}  // namespace accessibility
