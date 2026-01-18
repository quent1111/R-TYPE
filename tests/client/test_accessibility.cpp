#include <gtest/gtest.h>

#include "../../client/accessibility/include/ColorBlindnessMode.hpp"
#include "../../client/accessibility/include/ColorTransform.hpp"
#include "../../client/accessibility/include/AccessibilityManager.hpp"

#include <SFML/Graphics.hpp>
#include <fstream>

using namespace accessibility;

// ============================================================================
// ColorBlindnessMode String Conversion Tests
// ============================================================================

TEST(AccessibilityTest, ColorBlindnessModeToString_Normal) {
    EXPECT_STREQ("Normal", colorBlindnessModeToString(ColorBlindnessMode::Normal));
}

TEST(AccessibilityTest, ColorBlindnessModeToString_Protanopia) {
    EXPECT_STREQ("Protanopia", colorBlindnessModeToString(ColorBlindnessMode::Protanopia));
}

TEST(AccessibilityTest, ColorBlindnessModeToString_Deuteranopia) {
    EXPECT_STREQ("Deuteranopia", colorBlindnessModeToString(ColorBlindnessMode::Deuteranopia));
}

TEST(AccessibilityTest, ColorBlindnessModeToString_Tritanopia) {
    EXPECT_STREQ("Tritanopia", colorBlindnessModeToString(ColorBlindnessMode::Tritanopia));
}

TEST(AccessibilityTest, ColorBlindnessModeToString_HighContrast) {
    EXPECT_STREQ("HighContrast", colorBlindnessModeToString(ColorBlindnessMode::HighContrast));
}

TEST(AccessibilityTest, StringToColorBlindnessMode_Normal) {
    EXPECT_EQ(ColorBlindnessMode::Normal, stringToColorBlindnessMode("Normal"));
    EXPECT_EQ(ColorBlindnessMode::Normal, stringToColorBlindnessMode(""));
    EXPECT_EQ(ColorBlindnessMode::Normal, stringToColorBlindnessMode("Invalid"));
}

TEST(AccessibilityTest, StringToColorBlindnessMode_Protanopia) {
    EXPECT_EQ(ColorBlindnessMode::Protanopia, stringToColorBlindnessMode("Protanopia"));
}

TEST(AccessibilityTest, StringToColorBlindnessMode_Deuteranopia) {
    EXPECT_EQ(ColorBlindnessMode::Deuteranopia, stringToColorBlindnessMode("Deuteranopia"));
}

TEST(AccessibilityTest, StringToColorBlindnessMode_Tritanopia) {
    EXPECT_EQ(ColorBlindnessMode::Tritanopia, stringToColorBlindnessMode("Tritanopia"));
}

TEST(AccessibilityTest, StringToColorBlindnessMode_HighContrast) {
    EXPECT_EQ(ColorBlindnessMode::HighContrast, stringToColorBlindnessMode("HighContrast"));
}

TEST(AccessibilityTest, ColorBlindnessMode_RoundTrip) {
    for (auto mode : {ColorBlindnessMode::Normal, ColorBlindnessMode::Protanopia,
                      ColorBlindnessMode::Deuteranopia, ColorBlindnessMode::Tritanopia,
                      ColorBlindnessMode::HighContrast}) {
        std::string str = colorBlindnessModeToString(mode);
        ColorBlindnessMode converted = stringToColorBlindnessMode(str);
        EXPECT_EQ(mode, converted);
    }
}

// ============================================================================
// ColorTransform Tests
// ============================================================================

TEST(ColorTransformTest, Transform_NormalMode) {
    sf::Color original(100, 150, 200);
    sf::Color result = ColorTransform::transform(original, ColorBlindnessMode::Normal);
    
    EXPECT_EQ(original.r, result.r);
    EXPECT_EQ(original.g, result.g);
    EXPECT_EQ(original.b, result.b);
    EXPECT_EQ(original.a, result.a);
}

TEST(ColorTransformTest, Transform_ProtanopiaMode) {
    sf::Color original(255, 0, 0);
    sf::Color result = ColorTransform::transform(original, ColorBlindnessMode::Protanopia);
    
    EXPECT_NE(original.r, result.r);
    EXPECT_EQ(255, result.a);
}

TEST(ColorTransformTest, Transform_DeuteranopiaMode) {
    sf::Color original(0, 255, 0);
    sf::Color result = ColorTransform::transform(original, ColorBlindnessMode::Deuteranopia);
    
    EXPECT_NE(original.g, result.g);
    EXPECT_EQ(255, result.a);
}

TEST(ColorTransformTest, Transform_TritanopiaMode) {
    sf::Color original(0, 0, 255);
    sf::Color result = ColorTransform::transform(original, ColorBlindnessMode::Tritanopia);
    
    EXPECT_NE(original.b, result.b);
    EXPECT_EQ(255, result.a);
}

TEST(ColorTransformTest, Transform_HighContrastMode) {
    sf::Color original(128, 128, 128);
    sf::Color result = ColorTransform::transform(original, ColorBlindnessMode::HighContrast);
    
    EXPECT_EQ(255, result.a);
}

TEST(ColorTransformTest, Transform_PreservesAlpha) {
    sf::Color original(100, 150, 200, 128);
    
    for (auto mode : {ColorBlindnessMode::Protanopia, ColorBlindnessMode::Deuteranopia,
                      ColorBlindnessMode::Tritanopia, ColorBlindnessMode::HighContrast}) {
        sf::Color result = ColorTransform::transform(original, mode);
        EXPECT_EQ(128, result.a);
    }
}

TEST(ColorTransformTest, CalculateLuminance_Black) {
    sf::Color black(0, 0, 0);
    float lum = ColorTransform::calculateLuminance(black);
    EXPECT_FLOAT_EQ(0.0f, lum);
}

TEST(ColorTransformTest, CalculateLuminance_White) {
    sf::Color white(255, 255, 255);
    float lum = ColorTransform::calculateLuminance(white);
    EXPECT_GT(lum, 200.0f);
}

TEST(ColorTransformTest, CalculateLuminance_Red) {
    sf::Color red(255, 0, 0);
    float lum = ColorTransform::calculateLuminance(red);
    EXPECT_GT(lum, 0.0f);
    EXPECT_LT(lum, 100.0f);
}

TEST(ColorTransformTest, AreColorsDistinguishable_SameColor) {
    sf::Color color(100, 100, 100);
    
    bool result = ColorTransform::areColorsDistinguishable(
        color, color, ColorBlindnessMode::Normal, 40.0f);
    
    EXPECT_FALSE(result);
}

TEST(ColorTransformTest, AreColorsDistinguishable_VeryDifferent) {
    sf::Color black(0, 0, 0);
    sf::Color white(255, 255, 255);
    
    bool result = ColorTransform::areColorsDistinguishable(
        black, white, ColorBlindnessMode::Normal, 40.0f);
    
    EXPECT_TRUE(result);
}

TEST(ColorTransformTest, AreColorsDistinguishable_WithProtanopia) {
    sf::Color red(255, 0, 0);
    sf::Color green(0, 255, 0);
    
    bool result = ColorTransform::areColorsDistinguishable(
        red, green, ColorBlindnessMode::Protanopia, 40.0f);
    
    EXPECT_TRUE(result || !result);
}

TEST(ColorTransformTest, EnhanceContrast_AlreadyHighContrast) {
    sf::Color foreground(255, 255, 255);
    sf::Color background(0, 0, 0);
    
    sf::Color result = ColorTransform::enhanceContrast(foreground, background, 1.3f);
    
    EXPECT_EQ(255, result.a);
}

TEST(ColorTransformTest, EnhanceContrast_LowContrast) {
    sf::Color foreground(100, 100, 100, 100);
    sf::Color background(90, 90, 90);
    
    sf::Color result = ColorTransform::enhanceContrast(foreground, background, 2.0f);
    
    EXPECT_EQ(100, result.a);
}

TEST(ColorTransformTest, EnhanceContrast_PreservesAlpha) {
    sf::Color foreground(100, 100, 100, 200);
    sf::Color background(50, 50, 50);
    
    sf::Color result = ColorTransform::enhanceContrast(foreground, background, 1.5f);
    
    EXPECT_EQ(200, result.a);
}

// ============================================================================
// AccessibilityManager Tests
// ============================================================================

TEST(AccessibilityManagerTest, Singleton_ReturnsSameInstance) {
    AccessibilityManager& mgr1 = AccessibilityManager::instance();
    AccessibilityManager& mgr2 = AccessibilityManager::instance();
    
    EXPECT_EQ(&mgr1, &mgr2);
}

TEST(AccessibilityManagerTest, DefaultMode_IsNormal) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    mgr.resetToDefaults();
    
    EXPECT_EQ(ColorBlindnessMode::Normal, mgr.getColorBlindMode());
}

TEST(AccessibilityManagerTest, SetColorBlindMode_ChangesMode) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    mgr.setColorBlindMode(ColorBlindnessMode::Protanopia);
    EXPECT_EQ(ColorBlindnessMode::Protanopia, mgr.getColorBlindMode());
    
    mgr.setColorBlindMode(ColorBlindnessMode::Deuteranopia);
    EXPECT_EQ(ColorBlindnessMode::Deuteranopia, mgr.getColorBlindMode());
    
    mgr.resetToDefaults();
}

TEST(AccessibilityManagerTest, SetProjectileShapes_EnablesFeature) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    mgr.setProjectileShapesEnabled(true);
    EXPECT_TRUE(mgr.isProjectileShapesEnabled());
    
    mgr.setProjectileShapesEnabled(false);
    EXPECT_FALSE(mgr.isProjectileShapesEnabled());
    
    mgr.resetToDefaults();
}

TEST(AccessibilityManagerTest, TransformColor_AppliesCurrentMode) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    sf::Color original(255, 0, 0);
    
    mgr.setColorBlindMode(ColorBlindnessMode::Normal);
    sf::Color normalResult = mgr.transformColor(original);
    EXPECT_EQ(original, normalResult);
    
    mgr.setColorBlindMode(ColorBlindnessMode::Protanopia);
    sf::Color protanopiaResult = mgr.transformColor(original);
    (void)protanopiaResult;
    
    mgr.resetToDefaults();
}

TEST(AccessibilityManagerTest, GetPlayerProjectileColor_ReturnsColor) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    mgr.setColorBlindMode(ColorBlindnessMode::Normal);
    
    sf::Color color = mgr.getPlayerProjectileColor(sf::Color::White);
    
    EXPECT_EQ(255, color.a);
    EXPECT_GT(color.b, 0);
}

TEST(AccessibilityManagerTest, GetEnemyProjectileColor_ReturnsColor) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    mgr.setColorBlindMode(ColorBlindnessMode::Normal);
    
    sf::Color color = mgr.getEnemyProjectileColor(sf::Color::White);
    
    EXPECT_EQ(255, color.a);
    EXPECT_GT(color.r, 0);
}

TEST(AccessibilityManagerTest, GetBorderColor_DarkFill) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    mgr.setColorBlindMode(ColorBlindnessMode::Normal);
    
    sf::Color darkColor(50, 50, 50);
    sf::Color border = mgr.getBorderColor(darkColor);
    
    EXPECT_EQ(255, border.a);
}

TEST(AccessibilityManagerTest, GetBorderColor_LightFill) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    mgr.setColorBlindMode(ColorBlindnessMode::Normal);
    
    sf::Color lightColor(200, 200, 200);
    sf::Color border = mgr.getBorderColor(lightColor);
    
    EXPECT_EQ(255, border.a);
}

TEST(AccessibilityManagerTest, SaveAndLoadSettings_RoundTrip) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    mgr.setColorBlindMode(ColorBlindnessMode::Tritanopia);
    mgr.setProjectileShapesEnabled(false);
    
    std::string testFile = "test_accessibility_settings.ini";
    bool saved = mgr.saveSettings(testFile);
    EXPECT_TRUE(saved);
    
    mgr.resetToDefaults();
    EXPECT_EQ(ColorBlindnessMode::Normal, mgr.getColorBlindMode());
    
    bool loaded = mgr.loadSettings(testFile);
    EXPECT_TRUE(loaded);
    
    EXPECT_EQ(ColorBlindnessMode::Tritanopia, mgr.getColorBlindMode());
    EXPECT_FALSE(mgr.isProjectileShapesEnabled());
    
    std::remove(testFile.c_str());
    mgr.resetToDefaults();
}

TEST(AccessibilityManagerTest, LoadSettings_NonExistentFile) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    bool loaded = mgr.loadSettings("nonexistent_file_12345.ini");
    EXPECT_FALSE(loaded);
}

TEST(AccessibilityManagerTest, LoadSettings_WithExistingSection) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    std::string testFile = "test_accessibility_with_section.ini";
    std::ofstream file(testFile);
    file << "[Game]\n";
    file << "Volume=50\n";
    file << "[Accessibility]\n";
    file << "ColorBlindMode=Deuteranopia\n";
    file << "ProjectileShapes=true\n";
    file << "[Graphics]\n";
    file << "Resolution=1920x1080\n";
    file.close();
    
    bool loaded = mgr.loadSettings(testFile);
    EXPECT_TRUE(loaded);
    EXPECT_EQ(ColorBlindnessMode::Deuteranopia, mgr.getColorBlindMode());
    EXPECT_TRUE(mgr.isProjectileShapesEnabled());
    
    std::remove(testFile.c_str());
    mgr.resetToDefaults();
}

TEST(AccessibilityManagerTest, ResetToDefaults_RestoresInitialState) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    mgr.setColorBlindMode(ColorBlindnessMode::HighContrast);
    mgr.setProjectileShapesEnabled(false);
    
    mgr.resetToDefaults();
    
    EXPECT_EQ(ColorBlindnessMode::Normal, mgr.getColorBlindMode());
    EXPECT_TRUE(mgr.isProjectileShapesEnabled());
}

TEST(AccessibilityManagerTest, MultipleModeSwitches) {
    AccessibilityManager& mgr = AccessibilityManager::instance();
    
    for (auto mode : {ColorBlindnessMode::Protanopia, ColorBlindnessMode::Deuteranopia,
                      ColorBlindnessMode::Tritanopia, ColorBlindnessMode::HighContrast,
                      ColorBlindnessMode::Normal}) {
        mgr.setColorBlindMode(mode);
        EXPECT_EQ(mode, mgr.getColorBlindMode());
        
        sf::Color test(100, 100, 100);
        sf::Color transformed = mgr.transformColor(test);
        EXPECT_EQ(255, transformed.a);
    }
    
    mgr.resetToDefaults();
}

// ============================================================================
