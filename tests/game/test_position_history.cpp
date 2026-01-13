#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Mock structure pour simuler l'historique de position (Position History System)
// Ces structures imitent ce qui existe dans game/include/components/logic_components.hpp

template<typename T, size_t MaxSize>
class CircularBuffer {
private:
    std::vector<T> buffer_;
    size_t head_ = 0;
    size_t size_ = 0;
    
public:
    CircularBuffer() : buffer_(MaxSize) {}
    
    void push(const T& item) {
        buffer_[head_] = item;
        head_ = (head_ + 1) % MaxSize;
        if (size_ < MaxSize) {
            size_++;
        }
    }
    
    T get(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        size_t actual_index = (head_ + MaxSize - size_ + index) % MaxSize;
        return buffer_[actual_index];
    }
    
    T get_latest() const {
        if (size_ == 0) {
            throw std::runtime_error("Buffer is empty");
        }
        size_t latest_index = (head_ + MaxSize - 1) % MaxSize;
        return buffer_[latest_index];
    }
    
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == MaxSize; }
    
    void clear() {
        head_ = 0;
        size_ = 0;
    }
};

struct PositionSnapshot {
    float x = 0.0f;
    float y = 0.0f;
    uint32_t frame = 0;
    
    PositionSnapshot() = default;
    PositionSnapshot(float x_, float y_, uint32_t frame_) 
        : x(x_), y(y_), frame(frame_) {}
};

class PositionHistory {
private:
    static constexpr size_t MAX_HISTORY = 60;  // 1 seconde à 60 FPS
    CircularBuffer<PositionSnapshot, MAX_HISTORY> history_;
    uint32_t current_frame_ = 0;
    
public:
    void add_position(float x, float y) {
        history_.push(PositionSnapshot(x, y, current_frame_));
        current_frame_++;
    }
    
    bool get_delayed_position(uint32_t delay_frames, float& out_x, float& out_y) const {
        if (history_.empty()) {
            return false;
        }
        
        if (delay_frames == 0) {
            auto latest = history_.get_latest();
            out_x = latest.x;
            out_y = latest.y;
            return true;
        }
        
        if (delay_frames >= history_.size()) {
            // Trop de délai, retourner la plus ancienne position
            auto oldest = history_.get(0);
            out_x = oldest.x;
            out_y = oldest.y;
            return true;
        }
        
        // Récupérer position il y a N frames
        size_t index = history_.size() - 1 - delay_frames;
        auto snapshot = history_.get(index);
        out_x = snapshot.x;
        out_y = snapshot.y;
        return true;
    }
    
    size_t get_history_size() const {
        return history_.size();
    }
    
    uint32_t get_current_frame() const {
        return current_frame_;
    }
    
    void clear() {
        history_.clear();
        current_frame_ = 0;
    }
    
    bool is_full() const {
        return history_.full();
    }
};

class PositionHistoryTest : public ::testing::Test {
protected:
    PositionHistory history_;
};

// ============================================================================
// Tests de Base du Buffer Circulaire
// ============================================================================

TEST_F(PositionHistoryTest, InitialState) {
    EXPECT_EQ(history_.get_history_size(), 0);
    EXPECT_EQ(history_.get_current_frame(), 0);
    EXPECT_FALSE(history_.is_full());
}

TEST_F(PositionHistoryTest, AddSinglePosition) {
    history_.add_position(100.0f, 200.0f);
    
    EXPECT_EQ(history_.get_history_size(), 1);
    EXPECT_EQ(history_.get_current_frame(), 1);
}

TEST_F(PositionHistoryTest, AddMultiplePositions) {
    for (int i = 0; i < 10; ++i) {
        history_.add_position(i * 10.0f, i * 20.0f);
    }
    
    EXPECT_EQ(history_.get_history_size(), 10);
    EXPECT_EQ(history_.get_current_frame(), 10);
}

TEST_F(PositionHistoryTest, BufferFillToCapacity) {
    for (int i = 0; i < 60; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    EXPECT_EQ(history_.get_history_size(), 60);
    EXPECT_TRUE(history_.is_full());
}

TEST_F(PositionHistoryTest, BufferWraparound) {
    // Ajouter plus que la capacité
    for (int i = 0; i < 100; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    // Taille devrait rester à 60 (capacité max)
    EXPECT_EQ(history_.get_history_size(), 60);
    EXPECT_TRUE(history_.is_full());
    EXPECT_EQ(history_.get_current_frame(), 100);
}

// ============================================================================
// Tests de Récupération Sans Délai
// ============================================================================

TEST_F(PositionHistoryTest, GetLatestPositionNoDelay) {
    history_.add_position(100.0f, 200.0f);
    
    float x, y;
    bool success = history_.get_delayed_position(0, x, y);
    
    EXPECT_TRUE(success);
    EXPECT_FLOAT_EQ(x, 100.0f);
    EXPECT_FLOAT_EQ(y, 200.0f);
}

TEST_F(PositionHistoryTest, GetLatestAfterMultipleAdds) {
    history_.add_position(10.0f, 20.0f);
    history_.add_position(30.0f, 40.0f);
    history_.add_position(50.0f, 60.0f);
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    // Devrait retourner la dernière position
    EXPECT_FLOAT_EQ(x, 50.0f);
    EXPECT_FLOAT_EQ(y, 60.0f);
}

TEST_F(PositionHistoryTest, EmptyBufferReturnsFailure) {
    float x, y;
    bool success = history_.get_delayed_position(0, x, y);
    
    EXPECT_FALSE(success);
}

// ============================================================================
// Tests de Récupération Avec Délai
// ============================================================================

TEST_F(PositionHistoryTest, GetPositionOneFrameDelay) {
    history_.add_position(10.0f, 10.0f);  // Frame 0
    history_.add_position(20.0f, 20.0f);  // Frame 1
    history_.add_position(30.0f, 30.0f);  // Frame 2
    
    float x, y;
    history_.get_delayed_position(1, x, y);  // 1 frame de délai
    
    // Devrait retourner frame 1
    EXPECT_FLOAT_EQ(x, 20.0f);
    EXPECT_FLOAT_EQ(y, 20.0f);
}

TEST_F(PositionHistoryTest, GetPositionMultipleFramesDelay) {
    for (int i = 0; i < 10; ++i) {
        history_.add_position(i * 10.0f, i * 10.0f);
    }
    
    float x, y;
    history_.get_delayed_position(5, x, y);  // 5 frames de délai
    
    // Frame actuelle = 9, 5 frames en arrière = frame 4
    EXPECT_FLOAT_EQ(x, 40.0f);
    EXPECT_FLOAT_EQ(y, 40.0f);
}

TEST_F(PositionHistoryTest, DelayExceedsHistorySize) {
    for (int i = 0; i < 5; ++i) {
        history_.add_position(i * 10.0f, i * 10.0f);
    }
    
    float x, y;
    history_.get_delayed_position(10, x, y);  // Délai > taille buffer
    
    // Devrait retourner la plus ancienne position (frame 0)
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, 0.0f);
}

TEST_F(PositionHistoryTest, DelayExactlyHistorySize) {
    for (int i = 0; i < 10; ++i) {
        history_.add_position(i * 10.0f, i * 10.0f);
    }
    
    float x, y;
    history_.get_delayed_position(9, x, y);  // Exactement taille-1
    
    // Devrait retourner frame 0
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, 0.0f);
}

// ============================================================================
// Tests de Wraparound du Buffer Circulaire
// ============================================================================

TEST_F(PositionHistoryTest, WraparoundPreservesRecent) {
    // Remplir au-delà de la capacité
    for (int i = 0; i < 100; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    // Dernière position = frame 99
    EXPECT_FLOAT_EQ(x, 99.0f);
    EXPECT_FLOAT_EQ(y, 99.0f);
}

TEST_F(PositionHistoryTest, WraparoundOldestPosition) {
    // Remplir au-delà de la capacité
    for (int i = 0; i < 100; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    float x, y;
    history_.get_delayed_position(59, x, y);  // Plus ancien dans le buffer
    
    // Plus ancien = frame 40 (100 - 60 = 40)
    EXPECT_FLOAT_EQ(x, 40.0f);
    EXPECT_FLOAT_EQ(y, 40.0f);
}

TEST_F(PositionHistoryTest, WraparoundMiddlePosition) {
    for (int i = 0; i < 80; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    float x, y;
    history_.get_delayed_position(30, x, y);
    
    // Current = 79, -30 frames = frame 49
    EXPECT_FLOAT_EQ(x, 49.0f);
    EXPECT_FLOAT_EQ(y, 49.0f);
}

// ============================================================================
// Tests de Scénarios de Jeu (Serpent Boss)
// ============================================================================

TEST_F(PositionHistoryTest, SerpentBossTypicalDelay) {
    // Simuler mouvement du serpent pendant 2 secondes à 60 FPS
    for (int i = 0; i < 120; ++i) {
        float x = 100.0f + std::sin(i * 0.1f) * 50.0f;
        float y = 200.0f + std::cos(i * 0.1f) * 30.0f;
        history_.add_position(x, y);
    }
    
    // Récupérer position avec 15 frames de délai (250ms à 60 FPS)
    float delayed_x, delayed_y;
    history_.get_delayed_position(15, delayed_x, delayed_y);
    
    float current_x, current_y;
    history_.get_delayed_position(0, current_x, current_y);
    
    // Positions devraient être différentes
    float distance = std::sqrt(
        std::pow(current_x - delayed_x, 2) +
        std::pow(current_y - delayed_y, 2)
    );
    
    EXPECT_GT(distance, 1.0f);  // Mouvement significatif
}

TEST_F(PositionHistoryTest, SerpentSegmentChain) {
    // Simuler plusieurs segments suivant la tête avec différents délais
    for (int i = 0; i < 60; ++i) {
        history_.add_position(i * 5.0f, 100.0f);  // Mouvement linéaire
    }
    
    float head_x, head_y;
    float seg1_x, seg1_y;
    float seg2_x, seg2_y;
    float tail_x, tail_y;
    
    history_.get_delayed_position(0, head_x, head_y);   // Tête
    history_.get_delayed_position(10, seg1_x, seg1_y);  // Segment 1
    history_.get_delayed_position(20, seg2_x, seg2_y);  // Segment 2
    history_.get_delayed_position(30, tail_x, tail_y);  // Queue
    
    // Chaque segment devrait être derrière le précédent
    EXPECT_GT(head_x, seg1_x);
    EXPECT_GT(seg1_x, seg2_x);
    EXPECT_GT(seg2_x, tail_x);
    
    // Vérifier espacement constant
    float gap1 = head_x - seg1_x;
    float gap2 = seg1_x - seg2_x;
    EXPECT_NEAR(gap1, gap2, 1.0f);
}

TEST_F(PositionHistoryTest, SerpentStationary) {
    // Serpent immobile
    for (int i = 0; i < 60; ++i) {
        history_.add_position(100.0f, 200.0f);
    }
    
    float delayed_x, delayed_y;
    history_.get_delayed_position(30, delayed_x, delayed_y);
    
    // Position devrait être identique peu importe le délai
    EXPECT_FLOAT_EQ(delayed_x, 100.0f);
    EXPECT_FLOAT_EQ(delayed_y, 200.0f);
}

// ============================================================================
// Tests de Mouvement Complexe
// ============================================================================

TEST_F(PositionHistoryTest, CircularMotion) {
    float radius = 50.0f;
    float center_x = 200.0f;
    float center_y = 150.0f;
    
    for (int i = 0; i < 60; ++i) {
        float angle = i * M_PI / 30.0f;  // Cercle complet en 60 frames
        float x = center_x + radius * std::cos(angle);
        float y = center_y + radius * std::sin(angle);
        history_.add_position(x, y);
    }
    
    // Vérifier que le délai donne une position sur le cercle
    float delayed_x, delayed_y;
    history_.get_delayed_position(15, delayed_x, delayed_y);
    
    float distance_from_center = std::sqrt(
        std::pow(delayed_x - center_x, 2) +
        std::pow(delayed_y - center_y, 2)
    );
    
    EXPECT_NEAR(distance_from_center, radius, 1.0f);
}

TEST_F(PositionHistoryTest, ZigzagPattern) {
    for (int i = 0; i < 60; ++i) {
        float x = i * 2.0f;
        float y = (i % 10 < 5) ? 100.0f : 150.0f;  // Zigzag
        history_.add_position(x, y);
    }
    
    float x1, y1, x2, y2;
    history_.get_delayed_position(0, x1, y1);
    history_.get_delayed_position(5, x2, y2);
    
    // Vérifier que les positions sont différentes
    EXPECT_NE(x1, x2);
}

TEST_F(PositionHistoryTest, SuddenDirectionChange) {
    // Mouvement droit puis changement brusque
    for (int i = 0; i < 30; ++i) {
        history_.add_position(i * 10.0f, 100.0f);  // Droite
    }
    for (int i = 0; i < 30; ++i) {
        history_.add_position(300.0f, 100.0f + i * 10.0f);  // Haut
    }
    
    float x1, y1, x2, y2;
    history_.get_delayed_position(0, x1, y1);    // Position actuelle
    history_.get_delayed_position(35, x2, y2);   // Avant le changement
    
    // Position retardée devrait être dans la phase horizontale
    EXPECT_LT(x2, 300.0f);
    EXPECT_FLOAT_EQ(y2, 100.0f);
}

// ============================================================================
// Tests de Clear et Reset
// ============================================================================

TEST_F(PositionHistoryTest, ClearResetsState) {
    for (int i = 0; i < 30; ++i) {
        history_.add_position(i * 10.0f, i * 10.0f);
    }
    
    EXPECT_EQ(history_.get_history_size(), 30);
    
    history_.clear();
    
    EXPECT_EQ(history_.get_history_size(), 0);
    EXPECT_EQ(history_.get_current_frame(), 0);
    EXPECT_FALSE(history_.is_full());
}

TEST_F(PositionHistoryTest, ReusableAfterClear) {
    history_.add_position(10.0f, 10.0f);
    history_.clear();
    history_.add_position(20.0f, 20.0f);
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    EXPECT_FLOAT_EQ(x, 20.0f);
    EXPECT_FLOAT_EQ(y, 20.0f);
}

// ============================================================================
// Tests de Performance (DISABLED pour CI - peuvent varier selon la charge)
// ============================================================================

TEST_F(PositionHistoryTest, DISABLED_AddPositionPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 10000 ajouts devraient être très rapides
    EXPECT_LT(duration.count(), 5000);  // < 5ms
    
    std::cout << "[Performance] 10000 add_position() took " 
              << duration.count() << "µs" << std::endl;
}

TEST_F(PositionHistoryTest, DISABLED_GetDelayedPositionPerformance) {
    // Remplir le buffer
    for (int i = 0; i < 60; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    float x, y;
    for (int i = 0; i < 10000; ++i) {
        history_.get_delayed_position(30, x, y);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_LT(duration.count(), 5000);  // < 5ms
    
    std::cout << "[Performance] 10000 get_delayed_position() took " 
              << duration.count() << "µs" << std::endl;
}

// ============================================================================
// Tests Edge Cases
// ============================================================================

TEST_F(PositionHistoryTest, NegativeCoordinates) {
    history_.add_position(-100.0f, -200.0f);
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    EXPECT_FLOAT_EQ(x, -100.0f);
    EXPECT_FLOAT_EQ(y, -200.0f);
}

TEST_F(PositionHistoryTest, VeryLargeCoordinates) {
    history_.add_position(999999.0f, 999999.0f);
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    EXPECT_FLOAT_EQ(x, 999999.0f);
    EXPECT_FLOAT_EQ(y, 999999.0f);
}

TEST_F(PositionHistoryTest, ZeroCoordinates) {
    history_.add_position(0.0f, 0.0f);
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, 0.0f);
}

TEST_F(PositionHistoryTest, SingleFrameHistory) {
    history_.add_position(100.0f, 200.0f);
    
    float x, y;
    
    // Delay 0 devrait fonctionner
    EXPECT_TRUE(history_.get_delayed_position(0, x, y));
    EXPECT_FLOAT_EQ(x, 100.0f);
    
    // Delay 1 devrait retourner la seule position disponible
    EXPECT_TRUE(history_.get_delayed_position(1, x, y));
    EXPECT_FLOAT_EQ(x, 100.0f);
}

TEST_F(PositionHistoryTest, MaxDelayValue) {
    for (int i = 0; i < 60; ++i) {
        history_.add_position(i * 1.0f, i * 1.0f);
    }
    
    float x, y;
    // Délai énorme (> capacité)
    bool success = history_.get_delayed_position(1000, x, y);
    
    EXPECT_TRUE(success);
    // Devrait retourner la plus ancienne position
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, 0.0f);
}

// ============================================================================
// Tests de Précision
// ============================================================================

TEST_F(PositionHistoryTest, FloatingPointPrecision) {
    float precise_x = 123.456789f;
    float precise_y = 987.654321f;
    
    history_.add_position(precise_x, precise_y);
    
    float x, y;
    history_.get_delayed_position(0, x, y);
    
    EXPECT_FLOAT_EQ(x, precise_x);
    EXPECT_FLOAT_EQ(y, precise_y);
}

TEST_F(PositionHistoryTest, ConsecutiveIdenticalPositions) {
    // Ajouter 10 fois la même position
    for (int i = 0; i < 10; ++i) {
        history_.add_position(50.0f, 50.0f);
    }
    
    // Toutes les récupérations devraient retourner la même valeur
    for (uint32_t delay = 0; delay < 10; ++delay) {
        float x, y;
        history_.get_delayed_position(delay, x, y);
        EXPECT_FLOAT_EQ(x, 50.0f);
        EXPECT_FLOAT_EQ(y, 50.0f);
    }
}

// ============================================================================
// Tests de Cas d'Usage Réels
// ============================================================================

TEST_F(PositionHistoryTest, MultipleSegmentsTracking) {
    // Simuler un serpent à 5 segments
    const int NUM_SEGMENTS = 5;
    const uint32_t SEGMENT_DELAY = 8;  // 8 frames entre chaque segment
    
    // Ajouter 60 frames de mouvement
    for (int i = 0; i < 60; ++i) {
        history_.add_position(i * 3.0f, 100.0f);
    }
    
    // Récupérer positions de tous les segments
    std::vector<std::pair<float, float>> segment_positions;
    for (int seg = 0; seg < NUM_SEGMENTS; ++seg) {
        float x, y;
        history_.get_delayed_position(seg * SEGMENT_DELAY, x, y);
        segment_positions.push_back({x, y});
    }
    
    // Vérifier que chaque segment est derrière le précédent
    for (int i = 1; i < NUM_SEGMENTS; ++i) {
        EXPECT_LT(segment_positions[i].first, segment_positions[i-1].first);
    }
}

TEST_F(PositionHistoryTest, GameplayAt30FPS) {
    // Simuler jeu à 30 FPS (historique garde 2 secondes)
    for (int i = 0; i < 60; ++i) {
        history_.add_position(i * 5.0f, 200.0f);
    }
    
    // À 30 FPS, 15 frames = 500ms de délai
    float x, y;
    history_.get_delayed_position(15, x, y);
    
    // Position devrait être significativement en arrière
    float current_x, current_y;
    history_.get_delayed_position(0, current_x, current_y);
    
    float distance = current_x - x;
    EXPECT_NEAR(distance, 75.0f, 5.0f);  // 15 frames * 5 px/frame
}
