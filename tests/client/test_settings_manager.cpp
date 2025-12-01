#include <gtest/gtest.h>
#include "core/SettingsManager.hpp"
#include <fstream>
#include <filesystem>

class SettingsManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_file_ = "test_settings.ini";
        cleanup_test_file();
        reset_singleton();
    }

    void TearDown() override {
        cleanup_test_file();
    }

    void cleanup_test_file() {
        if (std::filesystem::exists(test_file_)) {
            std::filesystem::remove(test_file_);
        }
    }

    void reset_singleton() {
        auto& mgr = rtype::SettingsManager::get_instance();
        mgr.reset_to_defaults();
    }

    void create_test_ini(const std::string& content) {
        std::ofstream file(test_file_);
        file << content;
        file.close();
    }

    std::string test_file_;
};

TEST_F(SettingsManagerTest, SingletonBehavior) {
    auto& instance1 = rtype::SettingsManager::get_instance();
    auto& instance2 = rtype::SettingsManager::get_instance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(SettingsManagerTest, DefaultValues) {
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    EXPECT_EQ(mgr.get_resolution_width(), 1920u);
    EXPECT_EQ(mgr.get_resolution_height(), 1080u);
    EXPECT_FALSE(mgr.is_fullscreen());
    EXPECT_TRUE(mgr.is_vsync_enabled());
    EXPECT_FALSE(mgr.should_show_fps());
    EXPECT_EQ(mgr.get_graphics_quality(), 2);
    EXPECT_EQ(mgr.get_music_volume(), 70);
    EXPECT_EQ(mgr.get_sfx_volume(), 80);
}

TEST_F(SettingsManagerTest, LoadValidINI) {
    create_test_ini(
        "resolution_width=1280\n"
        "resolution_height=720\n"
        "fullscreen=true\n"
        "vsync=false\n"
        "show_fps=true\n"
        "graphics_quality=1\n"
        "music_volume=50\n"
        "sfx_volume=60\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    EXPECT_TRUE(mgr.load_from_file(test_file_));
    EXPECT_EQ(mgr.get_resolution_width(), 1280u);
    EXPECT_EQ(mgr.get_resolution_height(), 720u);
    EXPECT_TRUE(mgr.is_fullscreen());
    EXPECT_FALSE(mgr.is_vsync_enabled());
    EXPECT_TRUE(mgr.should_show_fps());
    EXPECT_EQ(mgr.get_graphics_quality(), 1);
    EXPECT_EQ(mgr.get_music_volume(), 50);
    EXPECT_EQ(mgr.get_sfx_volume(), 60);
}

TEST_F(SettingsManagerTest, LoadNonExistentFile) {
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    EXPECT_FALSE(mgr.load_from_file("nonexistent.ini"));
    EXPECT_EQ(mgr.get_resolution_width(), 1920u);
}

TEST_F(SettingsManagerTest, InvalidIntegerValues) {
    create_test_ini(
        "resolution_width=not_a_number\n"
        "resolution_height=720\n"
        "graphics_quality=invalid\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_resolution_width(), 1920u);
    EXPECT_EQ(mgr.get_resolution_height(), 720u);
    EXPECT_EQ(mgr.get_graphics_quality(), 2);
}

TEST_F(SettingsManagerTest, OutOfRangeValues) {
    create_test_ini(
        "resolution_width=999999999999999999999\n"
        "music_volume=99999999999999\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_resolution_width(), 1920u);
    EXPECT_EQ(mgr.get_music_volume(), 70);
}

TEST_F(SettingsManagerTest, BooleanFormatVariations) {
    create_test_ini(
        "fullscreen=1\n"
        "vsync=true\n"
        "show_fps=false\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_TRUE(mgr.is_fullscreen());
    EXPECT_TRUE(mgr.is_vsync_enabled());
    EXPECT_FALSE(mgr.should_show_fps());
}

TEST_F(SettingsManagerTest, CommentsAndEmptyLines) {
    create_test_ini(
        "# This is a comment\n"
        "\n"
        "resolution_width=1024\n"
        "; Another comment\n"
        "resolution_height=768\n"
        "\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_resolution_width(), 1024u);
    EXPECT_EQ(mgr.get_resolution_height(), 768u);
}

TEST_F(SettingsManagerTest, WhitespaceHandling) {
    create_test_ini(
        "  resolution_width  =  1600  \n"
        "\tresolution_height\t=\t900\t\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_resolution_width(), 1600u);
    EXPECT_EQ(mgr.get_resolution_height(), 900u);
}

TEST_F(SettingsManagerTest, SaveAndLoadRoundtrip) {
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.set_resolution(1366, 768);
    mgr.set_fullscreen(true);
    mgr.set_vsync(false);
    mgr.set_show_fps(true);
    mgr.set_graphics_quality(3);
    mgr.set_music_volume(85);
    mgr.set_sfx_volume(95);
    EXPECT_TRUE(mgr.save_to_file(test_file_));
    mgr.reset_to_defaults();
    EXPECT_TRUE(mgr.load_from_file(test_file_));
    EXPECT_EQ(mgr.get_resolution_width(), 1366u);
    EXPECT_EQ(mgr.get_resolution_height(), 768u);
    EXPECT_TRUE(mgr.is_fullscreen());
    EXPECT_FALSE(mgr.is_vsync_enabled());
    EXPECT_TRUE(mgr.should_show_fps());
    EXPECT_EQ(mgr.get_graphics_quality(), 3);
    EXPECT_EQ(mgr.get_music_volume(), 85);
    EXPECT_EQ(mgr.get_sfx_volume(), 95);
}

TEST_F(SettingsManagerTest, GraphicsQualityClamping) {
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.set_graphics_quality(-5);
    EXPECT_EQ(mgr.get_graphics_quality(), 0);
    mgr.set_graphics_quality(10);
    EXPECT_EQ(mgr.get_graphics_quality(), 3);
    mgr.set_graphics_quality(2);
    EXPECT_EQ(mgr.get_graphics_quality(), 2);
}

TEST_F(SettingsManagerTest, GraphicsQualityClampingFromFile) {
    create_test_ini(
        "graphics_quality=-10\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_graphics_quality(), 0);
    create_test_ini(
        "graphics_quality=999\n"
    );
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_graphics_quality(), 3);
}

TEST_F(SettingsManagerTest, VolumeClamping) {
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.set_music_volume(-10);
    EXPECT_EQ(mgr.get_music_volume(), 0);
    mgr.set_music_volume(150);
    EXPECT_EQ(mgr.get_music_volume(), 100);
    mgr.set_sfx_volume(-5);
    EXPECT_EQ(mgr.get_sfx_volume(), 0);
    mgr.set_sfx_volume(200);
    EXPECT_EQ(mgr.get_sfx_volume(), 100);
}

TEST_F(SettingsManagerTest, VolumeClampingFromFile) {
    create_test_ini(
        "music_volume=-50\n"
        "sfx_volume=250\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_music_volume(), 0);
    EXPECT_EQ(mgr.get_sfx_volume(), 100);
}

TEST_F(SettingsManagerTest, MalformedLines) {
    create_test_ini(
        "no_equals_sign\n"
        "resolution_width=1024\n"
        "=no_key\n"
        "resolution_height=768\n"
    );
    auto& mgr = rtype::SettingsManager::get_instance();
    mgr.reset_to_defaults();
    mgr.load_from_file(test_file_);
    EXPECT_EQ(mgr.get_resolution_width(), 1024u);
    EXPECT_EQ(mgr.get_resolution_height(), 768u);
}