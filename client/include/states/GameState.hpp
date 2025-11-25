#pragma once

#include "ecs/registry.hpp"
#include "states/IState.hpp"
#include "texture_manager.hpp"

#include <SFML/Graphics.hpp>

namespace rtype {

class GameState : public IState {
public:
    explicit GameState(sf::RenderWindow& window);
    ~GameState() override = default;

    void on_enter() override;
    void on_exit() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    std::string get_next_state() const override { return m_next_state; }

private:
    void init_game();
    void update_background(float dt);
    void render_background(sf::RenderWindow& window);
    void render_ui(sf::RenderWindow& window);
    void run_systems(float dt);
    void explosion_system(float dt);
    void animation_system(float dt);
    void render_system(sf::RenderWindow& window);

    sf::RenderWindow& m_window;
    registry m_registry;
    TextureManager m_textures;
    sf::Texture m_bg_texture;
    float m_bg_scroll_offset{0.0f};
    const float m_bg_scroll_speed{50.0f};
    sf::Font m_font;
    sf::Clock m_enemy_spawn_clock;
    const float m_enemy_spawn_interval{3.0f};
    std::string m_next_state;
    bool m_paused{false};
};

} // namespace rtype
