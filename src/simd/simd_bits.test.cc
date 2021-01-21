#include "gtest/gtest.h"
#include "simd_bits.h"
#include "simd_util.h"
#include "../test_util.test.h"

TEST(simd_bits, move) {
    simd_bits a(512);
    auto ptr = a.u64;

    simd_bits b(std::move(a));
    ASSERT_EQ(b.u64, ptr);

    simd_bits c(1);
    c = std::move(b);
    ASSERT_EQ(c.u64, ptr);
}

TEST(simd_bits, construct) {
    simd_bits d(1024);
    ASSERT_NE(d.ptr_simd, nullptr);
    ASSERT_EQ(d.num_simd_words, 4);
    ASSERT_EQ(d.num_bits_padded(), 1024);
    ASSERT_EQ(d.num_u8_padded(), 128);
    ASSERT_EQ(d.num_u16_padded(), 64);
    ASSERT_EQ(d.num_u32_padded(), 32);
    ASSERT_EQ(d.num_u64_padded(), 16);
}

TEST(simd_bits, aliased_editing_and_bit_refs) {
    simd_bits d(1024);
    auto c = (char *)d.u8;
    const simd_bits &cref = d;

    ASSERT_EQ(c[0], 0);
    ASSERT_EQ(c[13], 0);
    d[5] = true;
    ASSERT_EQ(c[0], 32);
    d[0] = true;
    ASSERT_EQ(c[0], 33);
    d[100] = true;
    ASSERT_EQ(d[100], true);
    ASSERT_EQ(c[12], 16);
    c[12] = 0;
    ASSERT_EQ(d[100], false);
    ASSERT_EQ(cref[100], d[100]);
}

TEST(simd_util, min_bits_to_num_bits_padded) {
    const auto &f = &simd_bits::min_bits_to_num_bits_padded;
    ASSERT_EQ(f(0), 0);
    ASSERT_EQ(f(1), 256);
    ASSERT_EQ(f(100), 256);
    ASSERT_EQ(f(255), 256);
    ASSERT_EQ(f(256), 256);
    ASSERT_EQ(f(257), 512);
    ASSERT_EQ(f((1 << 30) - 1), 1 << 30);
    ASSERT_EQ(f(1 << 30), 1 << 30);
    ASSERT_EQ(f((1 << 30) + 1), (1 << 30) + 256);
}

TEST(simd_bits, str) {
    simd_bits d(256);
    ASSERT_EQ(d.str(),
            "________________________________________________________________"
            "________________________________________________________________"
            "________________________________________________________________"
            "________________________________________________________________");
    d[5] = 1;
    ASSERT_EQ(d.str(),
            "_____1__________________________________________________________"
            "________________________________________________________________"
            "________________________________________________________________"
            "________________________________________________________________");
}

TEST(simd_bits, randomize) {
    simd_bits d(1024);

    d.randomize(64 + 57, SHARED_TEST_RNG());
    uint64_t mask = (1ULL << 57) - 1;
    // Randomized.
    ASSERT_NE(d.u64[0], 0);
    ASSERT_NE(d.u64[0], SIZE_MAX);
    ASSERT_NE(d.u64[1] & mask, 0);
    ASSERT_NE(d.u64[1] & mask, 0);
    // Not touched.
    ASSERT_EQ(d.u64[1] & ~mask, 0);
    ASSERT_EQ(d.u64[2], 0);
    ASSERT_EQ(d.u64[3], 0);

    for (size_t k = 0; k < d.num_u64_padded(); k++) {
        d.u64[k] = UINT64_MAX;
    }
    d.randomize(64 + 57, SHARED_TEST_RNG());
    // Randomized.
    ASSERT_NE(d.u64[0], 0);
    ASSERT_NE(d.u64[0], SIZE_MAX);
    ASSERT_NE(d.u64[1] & mask, 0);
    ASSERT_NE(d.u64[1] & mask, 0);
    // Not touched.
    ASSERT_EQ(d.u64[1] & ~mask, SIZE_MAX & ~mask);
    ASSERT_EQ(d.u64[2], SIZE_MAX);
    ASSERT_EQ(d.u64[3], SIZE_MAX);
}

TEST(simd_bits, xor_assignment) {
    simd_bits m0 = simd_bits::random(512, SHARED_TEST_RNG());
    simd_bits m1 = simd_bits::random(512, SHARED_TEST_RNG());
    simd_bits m2(512);
    m2 ^= m0;
    ASSERT_EQ(m0, m2);
    m2 ^= m1;
    for (size_t k = 0; k < m0.num_u64_padded(); k++) {
        ASSERT_EQ(m2.u64[k], m0.u64[k] ^ m1.u64[k]);
    }
}

TEST(simd_bits, assignment) {
    simd_bits m0(512);
    simd_bits m1(512);
    m0.randomize(512, SHARED_TEST_RNG());
    m1.randomize(512, SHARED_TEST_RNG());
    auto old_m1 = m1.u64[0];
    ASSERT_NE(m0, m1);
    m0 = m1;
    ASSERT_EQ(m0, m1);
    ASSERT_EQ(m0.u64[0], old_m1);
    ASSERT_EQ(m1.u64[0], old_m1);
}

TEST(simd_bits, equality) {
    simd_bits m0(512);
    simd_bits m1(512);
    simd_bits m4(1024);

    ASSERT_TRUE(m0 == m1);
    ASSERT_FALSE(m0 != m1);
    ASSERT_FALSE(m0 == m4);
    ASSERT_TRUE(m0 != m4);

    m1[505] = 1;
    ASSERT_FALSE(m0 == m1);
    ASSERT_TRUE(m0 != m1);
    m0[505] = 1;
    ASSERT_TRUE(m0 == m1);
    ASSERT_FALSE(m0 != m1);
}

TEST(simd_bits, swap_with) {
    simd_bits m0(512);
    simd_bits m1(512);
    simd_bits m2(512);
    simd_bits m3(512);
    m0.randomize(512, SHARED_TEST_RNG());
    m1.randomize(512, SHARED_TEST_RNG());
    m2 = m0;
    m3 = m1;
    ASSERT_EQ(m0, m2);
    ASSERT_EQ(m1, m3);
    m0.swap_with(m1);
    ASSERT_EQ(m0, m3);
    ASSERT_EQ(m1, m2);
}

TEST(simd_bits, clear) {
    simd_bits m0(512);
    m0.randomize(512, SHARED_TEST_RNG());
    ASSERT_TRUE(m0.not_zero());
    m0.clear();
    ASSERT_TRUE(!m0.not_zero());
}

TEST(simd_bits, not_zero256) {
    simd_bits m0(512);
    ASSERT_FALSE(m0.not_zero());
    m0[5] = 1;
    ASSERT_TRUE(m0.not_zero());
    m0[511] = 1;
    ASSERT_TRUE(m0.not_zero());
    m0[5] = 0;
    ASSERT_TRUE(m0.not_zero());
}

TEST(simd_bits, word_range_ref) {
    simd_bits d(1024);
    const simd_bits &cref = d;
    auto r1 = d.word_range_ref(1, 2);
    auto r2 = d.word_range_ref(2, 2);
    r1[1] = 1;
    ASSERT_TRUE(!r2.not_zero());
    ASSERT_EQ(r1[257], false);
    r2[1] = 1;
    ASSERT_EQ(r1[257], true);
    ASSERT_EQ(cref.word_range_ref(1, 2)[257], true);
}