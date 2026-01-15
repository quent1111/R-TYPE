#include "../include/ColorTransform.hpp"
#include <cmath>
#include <algorithm>

namespace accessibility {


const std::array<std::array<float, 3>, 3> ColorTransform::PROTANOPIA_MATRIX = {{
    {0.567f, 0.433f, 0.0f},
    {0.558f, 0.442f, 0.0f},
    {0.0f,   0.242f, 0.758f}
}};

const std::array<std::array<float, 3>, 3> ColorTransform::DEUTERANOPIA_MATRIX = {{
    {0.625f, 0.375f, 0.0f},
    {0.7f,   0.3f,   0.0f},
    {0.0f,   0.3f,   0.7f}
}};

const std::array<std::array<float, 3>, 3> ColorTransform::TRITANOPIA_MATRIX = {{
    {0.95f,  0.05f,  0.0f},
    {0.0f,   0.433f, 0.567f},
    {0.0f,   0.475f, 0.525f}
}};

sf::Color ColorTransform::transform(const sf::Color& original, ColorBlindnessMode mode) {
    if (mode == ColorBlindnessMode::Normal) {
        return original;
    }

    if (mode == ColorBlindnessMode::HighContrast) {
        float r = original.r / 255.0f;
        float g = original.g / 255.0f;
        float b = original.b / 255.0f;

        r = (r - 0.5f) * 1.5f + 0.5f;
        g = (g - 0.5f) * 1.5f + 0.5f;
        b = (b - 0.5f) * 1.5f + 0.5f;

        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);

        return sf::Color(
            static_cast<sf::Uint8>(r * 255),
            static_cast<sf::Uint8>(g * 255),
            static_cast<sf::Uint8>(b * 255),
            original.a
        );
    }

    auto linear = rgbToLinear(original);

    const std::array<std::array<float, 3>, 3>* matrix = nullptr;
    switch (mode) {
        case ColorBlindnessMode::Protanopia:
            matrix = &PROTANOPIA_MATRIX;
            break;
        case ColorBlindnessMode::Deuteranopia:
            matrix = &DEUTERANOPIA_MATRIX;
            break;
        case ColorBlindnessMode::Tritanopia:
            matrix = &TRITANOPIA_MATRIX;
            break;
        default:
            return original;
    }

    auto transformed = applyMatrix(linear, *matrix);

    sf::Color result = linearToRgb(transformed);
    result.a = original.a;

    return result;
}

bool ColorTransform::areColorsDistinguishable(const sf::Color& color1,
                                               const sf::Color& color2,
                                               ColorBlindnessMode mode,
                                               float threshold) {
    sf::Color transformed1 = transform(color1, mode);
    sf::Color transformed2 = transform(color2, mode);

    float dr = static_cast<float>(transformed1.r) - static_cast<float>(transformed2.r);
    float dg = static_cast<float>(transformed1.g) - static_cast<float>(transformed2.g);
    float db = static_cast<float>(transformed1.b) - static_cast<float>(transformed2.b);

    float distance = std::sqrt(dr * dr + dg * dg + db * db);

    return distance >= threshold;
}

sf::Color ColorTransform::enhanceContrast(const sf::Color& foreground,
                                          const sf::Color& background,
                                          float factor) {
    float fg_lum = calculateLuminance(foreground);
    float bg_lum = calculateLuminance(background);

    float current_contrast = std::abs(fg_lum - bg_lum);
    if (current_contrast > 180.0f) {
        return foreground;
    }

    float target_lum = fg_lum;
    if (fg_lum > bg_lum) {
        target_lum = std::min(255.0f, fg_lum + (255.0f - fg_lum) * (factor - 1.0f));
    } else {
        target_lum = std::max(0.0f, fg_lum - fg_lum * (factor - 1.0f));
    }

    float lum_ratio = target_lum / std::max(1.0f, fg_lum);

    return sf::Color(
        static_cast<sf::Uint8>(std::clamp(foreground.r * lum_ratio, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(std::clamp(foreground.g * lum_ratio, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(std::clamp(foreground.b * lum_ratio, 0.0f, 255.0f)),
        foreground.a
    );
}

std::array<float, 3> ColorTransform::rgbToLinear(const sf::Color& color) {
    auto toLinear = [](float channel) {
        channel /= 255.0f;
        if (channel <= 0.04045f) {
            return channel / 12.92f;
        }
        return std::pow((channel + 0.055f) / 1.055f, 2.4f);
    };

    return {
        toLinear(static_cast<float>(color.r)),
        toLinear(static_cast<float>(color.g)),
        toLinear(static_cast<float>(color.b))
    };
}

sf::Color ColorTransform::linearToRgb(const std::array<float, 3>& linear) {
    auto toGamma = [](float channel) {
        if (channel <= 0.0031308f) {
            return channel * 12.92f;
        }
        return 1.055f * std::pow(channel, 1.0f / 2.4f) - 0.055f;
    };

    return sf::Color(
        static_cast<sf::Uint8>(std::clamp(toGamma(linear[0]) * 255.0f, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(std::clamp(toGamma(linear[1]) * 255.0f, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(std::clamp(toGamma(linear[2]) * 255.0f, 0.0f, 255.0f))
    );
}

std::array<float, 3> ColorTransform::applyMatrix(
    const std::array<float, 3>& rgb,
    const std::array<std::array<float, 3>, 3>& matrix) {

    return {
        matrix[0][0] * rgb[0] + matrix[0][1] * rgb[1] + matrix[0][2] * rgb[2],
        matrix[1][0] * rgb[0] + matrix[1][1] * rgb[1] + matrix[1][2] * rgb[2],
        matrix[2][0] * rgb[0] + matrix[2][1] * rgb[1] + matrix[2][2] * rgb[2]
    };
}

float ColorTransform::calculateLuminance(const sf::Color& color) {
    return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

}  // namespace accessibility
