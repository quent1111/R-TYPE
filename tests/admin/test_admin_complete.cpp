#include <gtest/gtest.h>
#include "../../admin-client/include/AdminClient.hpp"
#include "../../admin-client/include/AdminUI.hpp"
#include "../../admin-client/include/LoginScreen.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cstdlib>

// ============================================================================
// PlayerInfo, LobbyInfo, ServerStatus Struct Tests
// ============================================================================
TEST(AdminStructs, PlayerInfoCreation) {
    PlayerInfo player;
    player.id = 42;
    player.address = "192.168.1.1";
    player.port = 8080;
    
    EXPECT_EQ(player.id, 42);
    EXPECT_EQ(player.address, "192.168.1.1");
    EXPECT_EQ(player.port, 8080);
}

TEST(AdminStructs, LobbyInfoCreation) {
    LobbyInfo lobby;
    lobby.id = 10;
    lobby.name = "TestLobby";
    lobby.current_players = 3;
    lobby.max_players = 8;
    lobby.state = 2;
    
    EXPECT_EQ(lobby.id, 10);
    EXPECT_EQ(lobby.name, "TestLobby");
    EXPECT_EQ(lobby.current_players, 3);
    EXPECT_EQ(lobby.max_players, 8);
    EXPECT_EQ(lobby.state, 2);
}

TEST(AdminStructs, ServerStatusCreation) {
    ServerStatus status;
    status.uptime = "5h 30m";
    status.player_count = 15;
    status.lobby_count = 3;
    
    EXPECT_EQ(status.uptime, "5h 30m");
    EXPECT_EQ(status.player_count, 15);
    EXPECT_EQ(status.lobby_count, 3);
}

// ============================================================================
// LoginScreen Tests
// ============================================================================
TEST(LoginScreen, InitialState) {
    LoginScreen screen;
    
    EXPECT_FALSE(screen.is_login_requested());
    EXPECT_EQ(screen.get_password(), "");
}

TEST(LoginScreen, TextInput) {
    LoginScreen screen;
    
    sf::Event event;
    event.type = sf::Event::TextEntered;
    event.text.unicode = 'a';
    screen.handle_event(event);
    
    event.text.unicode = 'b';
    screen.handle_event(event);
    
    event.text.unicode = 'c';
    screen.handle_event(event);
    
    EXPECT_EQ(screen.get_password(), "abc");
}

TEST(LoginScreen, BackspaceHandling) {
    LoginScreen screen;
    
    sf::Event event;
    event.type = sf::Event::TextEntered;
    event.text.unicode = 'x';
    screen.handle_event(event);
    event.text.unicode = 'y';
    screen.handle_event(event);
    event.text.unicode = 'z';
    screen.handle_event(event);
    
    event.text.unicode = '\b';
    screen.handle_event(event);
    
    EXPECT_EQ(screen.get_password(), "xy");
}

TEST(LoginScreen, EnterKeyTriggersLogin) {
    LoginScreen screen;
    
    sf::Event event;
    event.type = sf::Event::TextEntered;
    event.text.unicode = '\r';
    screen.handle_event(event);
    
    EXPECT_TRUE(screen.is_login_requested());
}

TEST(LoginScreen, NewlineTriggersLogin) {
    LoginScreen screen;
    
    sf::Event event;
    event.type = sf::Event::TextEntered;
    event.text.unicode = '\n';
    screen.handle_event(event);
    
    EXPECT_TRUE(screen.is_login_requested());
}

TEST(LoginScreen, ResetLoginRequest) {
    LoginScreen screen;
    
    sf::Event event;
    event.type = sf::Event::TextEntered;
    event.text.unicode = '\r';
    screen.handle_event(event);
    
    EXPECT_TRUE(screen.is_login_requested());
    
    screen.reset_login_request();
    EXPECT_FALSE(screen.is_login_requested());
}

TEST(LoginScreen, UpdateCursorBlink) {
    LoginScreen screen;
    
    screen.update(0.6f);
    screen.update(0.6f);
    
    EXPECT_EQ(screen.get_password(), "");
}

TEST(LoginScreen, SetErrorMessage) {
    LoginScreen screen;
    screen.set_error_message("Invalid password");
    
    EXPECT_FALSE(screen.is_login_requested());
}

TEST(LoginScreen, MouseClickOnLoginButton) {
    LoginScreen screen;
    
    sf::Event event;
    event.type = sf::Event::MouseButtonPressed;
    event.mouseButton.x = 600;
    event.mouseButton.y = 500;
    screen.handle_event(event);
    
    EXPECT_TRUE(screen.is_login_requested());
}

// ============================================================================
// AdminClient Tests
// ============================================================================
TEST(AdminClient, InitialState) {
    AdminClient client("127.0.0.1", 8080);
    
    EXPECT_FALSE(client.is_connected());
    EXPECT_FALSE(client.is_authenticated());
}

TEST(AdminClient, HasResponseInitiallyFalse) {
    AdminClient client("127.0.0.1", 8080);
    
    EXPECT_FALSE(client.has_response());
}

TEST(AdminClient, GetResponseWhenEmpty) {
    AdminClient client("127.0.0.1", 8080);
    
    std::string response = client.get_response();
    EXPECT_EQ(response, "");
}


TEST(AdminClient, GetPlayersEmptyResponse) {
    AdminClient client("127.0.0.1", 8080);
    
    auto players = client.get_players();
    EXPECT_TRUE(players.empty());
}

TEST(AdminClient, GetLobbiesEmptyResponse) {
    AdminClient client("127.0.0.1", 8080);
    
    auto lobbies = client.get_lobbies();
    EXPECT_TRUE(lobbies.empty());
}

TEST(AdminClient, GetServerStatusDefaultValues) {
    AdminClient client("127.0.0.1", 8080);
    
    auto status = client.get_server_status();
    EXPECT_EQ(status.uptime, "0h 0m 0s");
    EXPECT_EQ(status.player_count, 0);
    EXPECT_EQ(status.lobby_count, 0);
}

// ============================================================================
// AdminUI Tests (basic, without rendering)
// ============================================================================
class AdminUITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Skip SFML window tests in CI environment (no X11 display)
        if (std::getenv("CI") != nullptr) {
            GTEST_SKIP() << "Skipping UI tests in CI environment (no display)";
        }
        
        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode(800, 600), "TestWindow", sf::Style::None
        );
        window->setVisible(false);
        
        client = std::make_shared<AdminClient>("127.0.0.1", 8080);
    }
    
    void TearDown() override {
        if (window) {
            window->close();
        }
    }
    
    std::unique_ptr<sf::RenderWindow> window;
    std::shared_ptr<AdminClient> client;
};

TEST_F(AdminUITest, Construction) {
    EXPECT_NO_THROW({
        AdminUI ui(*window, client);
    });
}

TEST_F(AdminUITest, HandleMouseMove) {
    AdminUI ui(*window, client);
    
    sf::Event event;
    event.type = sf::Event::MouseMoved;
    event.mouseMove.x = 100;
    event.mouseMove.y = 200;
    
    EXPECT_NO_THROW(ui.handle_event(event));
}

TEST_F(AdminUITest, UpdateWithDeltaTime) {
    AdminUI ui(*window, client);
    
    EXPECT_NO_THROW(ui.update(0.016f));
    EXPECT_NO_THROW(ui.update(1.0f));
    EXPECT_NO_THROW(ui.update(3.0f));
}

TEST_F(AdminUITest, RenderWithoutCrash) {
    AdminUI ui(*window, client);
    
    EXPECT_NO_THROW(ui.render());
}

TEST_F(AdminUITest, RefreshDataWithoutAuth) {
    AdminUI ui(*window, client);
    
    EXPECT_NO_THROW(ui.refresh_data());
}
