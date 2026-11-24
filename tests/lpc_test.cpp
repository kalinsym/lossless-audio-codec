#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "lac/lpc.hpp"

TEST(Lpc, AutocorrelationWorkedExample) {
    const std::vector<std::int32_t> x = {1, 2, 3};
    EXPECT_EQ(lac::autocorrelation(x, 3), (std::vector<double>{14, 8, 3, 0}));
}

TEST(Lpc, LevinsonDurbinWorkedExamples) {
    const std::vector<double> r = {1.0, 0.5, 0.25};
    EXPECT_EQ(lac::levinsonDurbin(r, 1), (std::vector<double>{0.5}));
    EXPECT_EQ(lac::levinsonDurbin(r, 2), (std::vector<double>{0.5, 0.0}));
    EXPECT_EQ(lac::levinsonDurbin(std::vector<double>{0, 0, 0}, 2), (std::vector<double>{0, 0}));
}

TEST(Lpc, LevinsonDurbinSolvesTheNormalEquations) {
    std::vector<std::int32_t> x;
    for (int i = 0; i < 4096; ++i) {
        x.push_back(static_cast<std::int32_t>(std::lround(9000 * std::sin(0.05 * i) + 3000 * std::sin(0.7 * i))));
    }
    const std::vector<double> r = lac::autocorrelation(x, 8);
    const std::vector<double> a = lac::levinsonDurbin(r, 8);
    for (int i = 0; i < 8; ++i) {
        double sum = 0;
        for (int j = 0; j < 8; ++j) {
            sum += a[static_cast<std::size_t>(j)] * r[static_cast<std::size_t>(std::abs(i - j))];
        }
        EXPECT_NEAR(sum, r[static_cast<std::size_t>(i + 1)], 1e-6 * r[0]) << "row " << i;
    }
}
