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

TEST(Lpc, QuantizeWorkedExample) {
    const lac::QuantizedLpc q = lac::quantizeLpc(std::vector<double>{0.5, -0.25}, 15);
    EXPECT_EQ(q.shift, 14u);
    EXPECT_EQ(q.coefficients, (std::vector<std::int32_t>{8192, -4096}));
}

TEST(Lpc, QuantizedCoefficientsStayCloseAndFitThePrecision) {
    const std::vector<double> a = {1.9, -0.95, 0.123456, -1e-7};
    const lac::QuantizedLpc q = lac::quantizeLpc(a, 12);
    EXPECT_EQ(q.shift, 10u);
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_LE(std::abs(q.coefficients[i]), 2047);
        EXPECT_NEAR(std::ldexp(q.coefficients[i], -10), a[i], std::ldexp(1.0, -10));
    }
}

TEST(Lpc, ResidualWorkedExample) {
    const lac::QuantizedLpc line{{2048, -1024}, 10};
    const std::vector<std::int32_t> squares = {1, 4, 9, 16, 25};
    EXPECT_EQ(lac::lpcResidual(squares, line), (std::vector<std::int32_t>{2, 2, 2}));
}

TEST(Lpc, ResidualRejectsOverflow) {
    EXPECT_FALSE(lac::lpcResidual(std::vector<std::int32_t>{8388607, 8388607}, lac::QuantizedLpc{{16383}, 0}));
}
