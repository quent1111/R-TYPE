#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(NetworkID, DefaultID) {
    network_id net_id;
    EXPECT_EQ(net_id.client_id, -1);
}

TEST(NetworkID, CustomID) {
    network_id net_id(42);
    EXPECT_EQ(net_id.client_id, 42);
}

TEST(NetworkID, NegativeID) {
    network_id net_id(-100);
    EXPECT_EQ(net_id.client_id, -100);
}

TEST(EntityTag, PlayerType) {
    entity_tag tag(RType::EntityType::Player);
    EXPECT_EQ(tag.type, RType::EntityType::Player);
}

TEST(EntityTag, EnemyType) {
    entity_tag tag(RType::EntityType::Enemy);
    EXPECT_EQ(tag.type, RType::EntityType::Enemy);
}

TEST(EntityTag, ProjectileType) {
    entity_tag tag(RType::EntityType::Projectile);
    EXPECT_EQ(tag.type, RType::EntityType::Projectile);
}

TEST(EntityTag, BossType) {
    entity_tag tag(RType::EntityType::Boss);
    EXPECT_EQ(tag.type, RType::EntityType::Boss);
}

TEST(ExplosionTag, DefaultLifetime) {
    explosion_tag exp;
    EXPECT_FLOAT_EQ(exp.lifetime, 0.5f);
    EXPECT_FLOAT_EQ(exp.elapsed, 0.0f);
}

TEST(ExplosionTag, CustomLifetime) {
    explosion_tag exp(1.0f);
    EXPECT_FLOAT_EQ(exp.lifetime, 1.0f);
    EXPECT_FLOAT_EQ(exp.elapsed, 0.0f);
}

TEST(ExplosionTag, TimeProgression) {
    explosion_tag exp(1.0f);
    exp.elapsed = 0.5f;
    
    EXPECT_FLOAT_EQ(exp.elapsed, 0.5f);
    EXPECT_LT(exp.elapsed, exp.lifetime);
}

TEST(ExplosionTag, ExplosionComplete) {
    explosion_tag exp(1.0f);
    exp.elapsed = 1.0f;
    
    EXPECT_GE(exp.elapsed, exp.lifetime);
}
