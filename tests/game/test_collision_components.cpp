#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(DamageComponent, BasicDamage) {
    damage dmg(25);
    EXPECT_EQ(dmg.amount, 25);
}

TEST(DamageOnContact, DestroyOnHit) {
    damage_on_contact doc(15, true);
    EXPECT_EQ(doc.damage_amount, 15);
    EXPECT_TRUE(doc.destroy_on_hit);

    damage_on_contact no_destroy(10, false);
    EXPECT_FALSE(no_destroy.destroy_on_hit);
}

TEST(CollisionBox, DefaultValues) {
    collision_box box;
    EXPECT_FLOAT_EQ(box.width, 50.0f);
    EXPECT_FLOAT_EQ(box.height, 50.0f);
    EXPECT_FLOAT_EQ(box.offset_x, 0.0f);
    EXPECT_FLOAT_EQ(box.offset_y, 0.0f);
}

TEST(CollisionBox, CustomValues) {
    collision_box box(100.0f, 80.0f, 5.0f, 10.0f);
    EXPECT_FLOAT_EQ(box.width, 100.0f);
    EXPECT_FLOAT_EQ(box.height, 80.0f);
    EXPECT_FLOAT_EQ(box.offset_x, 5.0f);
    EXPECT_FLOAT_EQ(box.offset_y, 10.0f);
}

TEST(CollisionBox, AABBCollision) {
    collision_box box1(50.0f, 50.0f, 0.0f, 0.0f);
    collision_box box2(50.0f, 50.0f, 0.0f, 0.0f);

    float e1_x = 0.0f, e1_y = 0.0f;
    float e2_x = 25.0f, e2_y = 25.0f;

    float left1 = e1_x + box1.offset_x;
    float right1 = left1 + box1.width;
    float top1 = e1_y + box1.offset_y;
    float bottom1 = top1 + box1.height;

    float left2 = e2_x + box2.offset_x;
    float right2 = left2 + box2.width;
    float top2 = e2_y + box2.offset_y;
    float bottom2 = top2 + box2.height;

    bool collision = !(right1 < left2 || right2 < left1 || bottom1 < top2 || bottom2 < top1);
    EXPECT_TRUE(collision);

    e2_x = 100.0f;
    e2_y = 100.0f;
    left2 = e2_x + box2.offset_x;
    right2 = left2 + box2.width;
    top2 = e2_y + box2.offset_y;
    bottom2 = top2 + box2.height;

    collision = !(right1 < left2 || right2 < left1 || bottom1 < top2 || bottom2 < top1);
    EXPECT_FALSE(collision);
}

TEST(MultiHitbox, MultipleParts) {
    multi_hitbox mh;
    mh.parts.push_back(multi_hitbox::hitbox_part(30.0f, 30.0f, 0.0f, 0.0f));
    mh.parts.push_back(multi_hitbox::hitbox_part(40.0f, 20.0f, 50.0f, 0.0f));
    mh.parts.push_back(multi_hitbox::hitbox_part(20.0f, 40.0f, 0.0f, 50.0f));

    EXPECT_EQ(mh.parts.size(), 3u);
    EXPECT_FLOAT_EQ(mh.parts[0].width, 30.0f);
    EXPECT_FLOAT_EQ(mh.parts[1].offset_x, 50.0f);
}
