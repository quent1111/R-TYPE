#include <gtest/gtest.h>
#include <cmath>

// Mock structures pour simuler le système de prédiction client
// Ces structures imitent ce qui existe dans client/src/game/Game.cpp

struct PredictionState {
    float predicted_player_x = 0.0f;
    float predicted_player_y = 0.0f;
    float server_player_x = 0.0f;
    float server_player_y = 0.0f;
    float correction_speed = 10.0f;
    float snap_threshold = 50.0f;
    
    // Appliquer une prédiction de mouvement
    void apply_prediction(float dx, float dy, float dt) {
        predicted_player_x += dx * dt;
        predicted_player_y += dy * dt;
    }
    
    // Recevoir une mise à jour du serveur
    void receive_server_update(float server_x, float server_y) {
        server_player_x = server_x;
        server_player_y = server_y;
    }
    
    // Appliquer la correction (smooth ou snap)
    void apply_correction(float dt) {
        float dx = server_player_x - predicted_player_x;
        float dy = server_player_y - predicted_player_y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance > snap_threshold) {
            // Snap immédiat si trop loin
            predicted_player_x = server_player_x;
            predicted_player_y = server_player_y;
        } else if (distance > 0.1f) {
            // Correction progressive
            float correction_amount = correction_speed * dt;
            float t = std::min(1.0f, correction_amount / distance);
            predicted_player_x += dx * t;
            predicted_player_y += dy * t;
        }
    }
    
    float get_prediction_error() const {
        float dx = server_player_x - predicted_player_x;
        float dy = server_player_y - predicted_player_y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    void reset() {
        predicted_player_x = 0.0f;
        predicted_player_y = 0.0f;
        server_player_x = 0.0f;
        server_player_y = 0.0f;
    }
};

class ClientPredictionTest : public ::testing::Test {
protected:
    PredictionState state_;
    const float dt_ = 1.0f / 60.0f;  // 60 FPS
};

// ============================================================================
// Tests de Prédiction de Base
// ============================================================================

TEST_F(ClientPredictionTest, InitialState) {
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 0.0f);
    EXPECT_FLOAT_EQ(state_.predicted_player_y, 0.0f);
    EXPECT_FLOAT_EQ(state_.server_player_x, 0.0f);
    EXPECT_FLOAT_EQ(state_.server_player_y, 0.0f);
}

TEST_F(ClientPredictionTest, SimplePrediction) {
    // Mouvement à droite : vitesse 300 px/s
    state_.apply_prediction(300.0f, 0.0f, dt_);
    
    EXPECT_NEAR(state_.predicted_player_x, 5.0f, 0.01f);  // 300 * (1/60)
    EXPECT_FLOAT_EQ(state_.predicted_player_y, 0.0f);
}

TEST_F(ClientPredictionTest, ContinuousPrediction) {
    // Simuler 1 seconde de mouvement (60 frames)
    for (int i = 0; i < 60; ++i) {
        state_.apply_prediction(100.0f, 50.0f, dt_);
    }
    
    EXPECT_NEAR(state_.predicted_player_x, 100.0f, 0.1f);
    EXPECT_NEAR(state_.predicted_player_y, 50.0f, 0.1f);
}

TEST_F(ClientPredictionTest, DiagonalMovement) {
    // Mouvement diagonal (Pythagore: vitesse effective ~424 px/s)
    float speed = 300.0f;
    float dx = speed * std::cos(M_PI / 4);  // 45 degrés
    float dy = speed * std::sin(M_PI / 4);
    
    for (int i = 0; i < 60; ++i) {
        state_.apply_prediction(dx, dy, dt_);
    }
    
    float expected_distance = speed;
    float actual_distance = std::sqrt(
        state_.predicted_player_x * state_.predicted_player_x +
        state_.predicted_player_y * state_.predicted_player_y
    );
    
    EXPECT_NEAR(actual_distance, expected_distance, 1.0f);
}

// ============================================================================
// Tests de Correction Smooth (< 50px)
// ============================================================================

TEST_F(ClientPredictionTest, SmallErrorSmoothCorrection) {
    state_.predicted_player_x = 100.0f;
    state_.predicted_player_y = 100.0f;
    state_.receive_server_update(110.0f, 100.0f);  // Écart de 10px
    
    EXPECT_NEAR(state_.get_prediction_error(), 10.0f, 0.1f);
    
    // Appliquer correction
    state_.apply_correction(dt_);
    
    // Devrait avoir corrigé partiellement, pas instantanément
    EXPECT_GT(state_.predicted_player_x, 100.0f);
    EXPECT_LT(state_.predicted_player_x, 110.0f);
}

TEST_F(ClientPredictionTest, SmoothCorrectionConverges) {
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(20.0f, 0.0f);
    
    // Appliquer correction sur plusieurs frames
    for (int i = 0; i < 100; ++i) {
        state_.apply_correction(dt_);
    }
    
    // Devrait converger vers la position serveur
    EXPECT_NEAR(state_.predicted_player_x, 20.0f, 0.5f);
}

TEST_F(ClientPredictionTest, CorrectionSpeed) {
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(50.0f, 0.0f);
    
    // Mesurer combien de frames pour corriger
    int frames = 0;
    while (state_.get_prediction_error() > 1.0f && frames < 200) {
        state_.apply_correction(dt_);
        frames++;
    }
    
    // Avec correction_speed=10, devrait converger rapidement
    EXPECT_LT(frames, 100);
    std::cout << "[Correction] Converged in " << frames << " frames" << std::endl;
}

TEST_F(ClientPredictionTest, NoUnnecessaryCorrection) {
    state_.predicted_player_x = 100.0f;
    state_.predicted_player_y = 100.0f;
    state_.receive_server_update(100.0f, 100.0f);  // Pas d'écart
    
    float before_x = state_.predicted_player_x;
    state_.apply_correction(dt_);
    
    // Position ne devrait pas changer
    EXPECT_FLOAT_EQ(state_.predicted_player_x, before_x);
}

// ============================================================================
// Tests de Snap (>= 50px)
// ============================================================================

TEST_F(ClientPredictionTest, LargeErrorSnapCorrection) {
    state_.predicted_player_x = 0.0f;
    state_.predicted_player_y = 0.0f;
    state_.receive_server_update(100.0f, 0.0f);  // Écart de 100px
    
    EXPECT_GT(state_.get_prediction_error(), state_.snap_threshold);
    
    state_.apply_correction(dt_);
    
    // Devrait avoir snap instantanément
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 100.0f);
    EXPECT_FLOAT_EQ(state_.predicted_player_y, 0.0f);
}

TEST_F(ClientPredictionTest, SnapThresholdExact) {
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(50.0f, 0.0f);  // Exactement au threshold
    
    state_.apply_correction(dt_);
    
    // Devrait snap car >= threshold
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 50.0f);
}

TEST_F(ClientPredictionTest, SnapDiagonal) {
    state_.predicted_player_x = 0.0f;
    state_.predicted_player_y = 0.0f;
    state_.receive_server_update(40.0f, 40.0f);  // Distance ~56px > 50px
    
    float error = state_.get_prediction_error();
    EXPECT_GT(error, state_.snap_threshold);
    
    state_.apply_correction(dt_);
    
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 40.0f);
    EXPECT_FLOAT_EQ(state_.predicted_player_y, 40.0f);
}

// ============================================================================
// Tests de Scénarios Réels
// ============================================================================

TEST_F(ClientPredictionTest, TypicalGameplay) {
    // Simuler un gameplay normal :
    // Client prédit mouvement, serveur confirme avec petit délai
    
    // Frame 1-5 : prédiction
    for (int i = 0; i < 5; ++i) {
        state_.apply_prediction(300.0f, 0.0f, dt_);
    }
    
    float predicted_x = state_.predicted_player_x;
    
    // Frame 6 : serveur répond (légèrement en retard)
    state_.receive_server_update(predicted_x - 2.0f, 0.0f);
    
    // Frame 6-10 : correction smooth
    for (int i = 0; i < 5; ++i) {
        state_.apply_prediction(300.0f, 0.0f, dt_);
        state_.apply_correction(dt_);
    }
    
    // Erreur devrait être minime
    EXPECT_LT(state_.get_prediction_error(), 5.0f);
}

TEST_F(ClientPredictionTest, NetworkSpike) {
    // Client prédit pendant longtemps sans update serveur
    for (int i = 0; i < 60; ++i) {  // 1 seconde
        state_.apply_prediction(100.0f, 0.0f, dt_);
    }
    
    EXPECT_NEAR(state_.predicted_player_x, 100.0f, 0.5f);
    
    // Serveur envoie finalement position très différente (lag spike)
    state_.receive_server_update(150.0f, 0.0f);
    
    // Snap car > 50px
    state_.apply_correction(dt_);
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 150.0f);
}

TEST_F(ClientPredictionTest, CollisionCorrection) {
    // Client prédit mouvement dans un mur
    state_.predicted_player_x = 100.0f;
    for (int i = 0; i < 10; ++i) {
        state_.apply_prediction(300.0f, 0.0f, dt_);
    }
    
    EXPECT_NEAR(state_.predicted_player_x, 150.0f, 1.0f);
    
    // Serveur corrige : collision avec mur à x=110
    state_.receive_server_update(110.0f, 0.0f);
    
    // Snap car > 50px
    state_.apply_correction(dt_);
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 110.0f);
}

TEST_F(ClientPredictionTest, RapidDirectionChanges) {
    // Simuler changements rapides de direction
    for (int i = 0; i < 10; ++i) {
        state_.apply_prediction(100.0f, 0.0f, dt_);    // Droite
    }
    for (int i = 0; i < 10; ++i) {
        state_.apply_prediction(-100.0f, 0.0f, dt_);   // Gauche
    }
    for (int i = 0; i < 10; ++i) {
        state_.apply_prediction(0.0f, 100.0f, dt_);    // Haut
    }
    
    // Serveur confirme position approximative
    state_.receive_server_update(state_.predicted_player_x + 3.0f,
                                  state_.predicted_player_y - 2.0f);
    
    state_.apply_correction(dt_);
    
    // Erreur devrait rester petite
    EXPECT_LT(state_.get_prediction_error(), 5.0f);
}

// ============================================================================
// Tests de Configuration
// ============================================================================

TEST_F(ClientPredictionTest, CorrectionSpeedConfiguration) {
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(30.0f, 0.0f);
    
    // Test avec correction_speed = 5 (lent)
    state_.correction_speed = 5.0f;
    state_.apply_correction(dt_);
    float slow_x = state_.predicted_player_x;
    
    // Reset et test avec correction_speed = 20 (rapide)
    state_.reset();
    state_.receive_server_update(30.0f, 0.0f);
    state_.correction_speed = 20.0f;
    state_.apply_correction(dt_);
    float fast_x = state_.predicted_player_x;
    
    // Correction rapide devrait avancer plus
    EXPECT_GT(fast_x, slow_x);
}

TEST_F(ClientPredictionTest, SnapThresholdConfiguration) {
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(40.0f, 0.0f);
    
    // Avec threshold = 30, devrait snap
    state_.snap_threshold = 30.0f;
    state_.apply_correction(dt_);
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 40.0f);
    
    // Reset avec threshold = 50, devrait smooth
    state_.reset();
    state_.receive_server_update(40.0f, 0.0f);
    state_.snap_threshold = 50.0f;
    state_.apply_correction(dt_);
    EXPECT_GT(state_.predicted_player_x, 0.0f);
    EXPECT_LT(state_.predicted_player_x, 40.0f);
}

// ============================================================================
// Tests de Limites et Edge Cases
// ============================================================================

TEST_F(ClientPredictionTest, ZeroVelocity) {
    state_.apply_prediction(0.0f, 0.0f, dt_);
    
    EXPECT_FLOAT_EQ(state_.predicted_player_x, 0.0f);
    EXPECT_FLOAT_EQ(state_.predicted_player_y, 0.0f);
}

TEST_F(ClientPredictionTest, NegativeCoordinates) {
    state_.predicted_player_x = 100.0f;
    state_.predicted_player_y = 100.0f;
    
    for (int i = 0; i < 60; ++i) {
        state_.apply_prediction(-200.0f, -200.0f, dt_);
    }
    
    EXPECT_LT(state_.predicted_player_x, 0.0f);
    EXPECT_LT(state_.predicted_player_y, 0.0f);
}

TEST_F(ClientPredictionTest, VeryHighSpeed) {
    // Vitesse très élevée : 10000 px/s
    for (int i = 0; i < 60; ++i) {
        state_.apply_prediction(10000.0f, 0.0f, dt_);
    }
    
    EXPECT_NEAR(state_.predicted_player_x, 10000.0f, 10.0f);
}

TEST_F(ClientPredictionTest, VerySmallDeltaTime) {
    // dt très petit (240 FPS)
    float tiny_dt = 1.0f / 240.0f;
    
    for (int i = 0; i < 240; ++i) {
        state_.apply_prediction(100.0f, 0.0f, tiny_dt);
    }
    
    EXPECT_NEAR(state_.predicted_player_x, 100.0f, 1.0f);
}

TEST_F(ClientPredictionTest, TinyError) {
    state_.predicted_player_x = 100.0f;
    state_.receive_server_update(100.05f, 0.0f);  // Erreur de 0.05px
    
    // Devrait ignorer les erreurs < 0.1px
    state_.apply_correction(dt_);
    
    EXPECT_LT(state_.get_prediction_error(), 0.1f);
}

// ============================================================================
// Tests de Performance
// ============================================================================

TEST_F(ClientPredictionTest, PredictionPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        state_.apply_prediction(300.0f, 200.0f, dt_);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 10000 prédictions devraient être très rapides
    EXPECT_LT(duration.count(), 1000);  // < 1ms
    
    std::cout << "[Performance] 10000 apply_prediction() took " 
              << duration.count() << "µs" << std::endl;
}

TEST_F(ClientPredictionTest, CorrectionPerformance) {
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(25.0f, 25.0f);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        state_.apply_correction(dt_);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_LT(duration.count(), 1000);  // < 1ms
    
    std::cout << "[Performance] 10000 apply_correction() took " 
              << duration.count() << "µs" << std::endl;
}

// ============================================================================
// Tests de Stabilité
// ============================================================================

TEST_F(ClientPredictionTest, OscillationPrevention) {
    // Vérifier qu'il n'y a pas d'oscillation lors de la correction
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(20.0f, 0.0f);
    
    float prev_error = state_.get_prediction_error();
    
    for (int i = 0; i < 50; ++i) {
        state_.apply_correction(dt_);
        float current_error = state_.get_prediction_error();
        
        // Erreur devrait toujours diminuer (pas d'oscillation)
        EXPECT_LE(current_error, prev_error + 0.01f);
        prev_error = current_error;
    }
}

TEST_F(ClientPredictionTest, MultipleUpdates) {
    // Recevoir plusieurs updates serveur consécutives
    state_.predicted_player_x = 0.0f;
    
    state_.receive_server_update(10.0f, 0.0f);
    state_.apply_correction(dt_);
    
    state_.receive_server_update(15.0f, 0.0f);
    state_.apply_correction(dt_);
    
    state_.receive_server_update(20.0f, 0.0f);
    state_.apply_correction(dt_);
    
    // Devrait converger vers la dernière position
    for (int i = 0; i < 100; ++i) {
        state_.apply_correction(dt_);
    }
    
    EXPECT_NEAR(state_.predicted_player_x, 20.0f, 1.0f);
}

TEST_F(ClientPredictionTest, PredictionDuringCorrection) {
    // Continuer à prédire pendant la correction
    state_.predicted_player_x = 0.0f;
    state_.receive_server_update(10.0f, 0.0f);
    
    for (int i = 0; i < 30; ++i) {
        state_.apply_prediction(50.0f, 0.0f, dt_);  // Continue de bouger
        state_.apply_correction(dt_);                // Corrige en même temps
    }
    
    // Devrait être à ~25px (prédiction) + correction vers serveur
    EXPECT_GT(state_.predicted_player_x, 10.0f);
    EXPECT_LT(state_.predicted_player_x, 35.0f);
}
