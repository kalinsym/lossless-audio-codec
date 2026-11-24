#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "lac/lpc.hpp"

TEST(Lpc, AutocorrelationWorkedExample) {
    const std::vector<std::int32_t> x = {1, 2, 3};
    EXPECT_EQ(lac::autocorrelation(x, 3), (std::vector<double>{14, 8, 3, 0}));
}
