#include <gtest/gtest.h>

#include <stdexcept>

#include "rest/SymbolMap.h"

using namespace hyperliquid;

TEST(SymbolMapTest, ResolvesExactIdItWasAddedWith)
{
    SymbolMap map;
    map.add("ETH", 4);
    EXPECT_EQ(map.resolve("ETH"), 4);
}

TEST(SymbolMapTest, UnknownSymbolThrows)
{
    SymbolMap map;
    map.add("ETH", 4);
    EXPECT_THROW(map.resolve("BTC"), std::invalid_argument);
}

TEST(SymbolMapTest, ResolvingOnEmptyMapThrows)
{
    SymbolMap map;
    EXPECT_THROW(map.resolve("ETH"), std::invalid_argument);
}

TEST(SymbolMapTest, SymbolLookupIsCaseSensitive)
{
    SymbolMap map;
    map.add("ETH", 4);
    EXPECT_THROW(map.resolve("eth"), std::invalid_argument);
}

TEST(SymbolMapTest, FirstAddWinsOnDuplicateSymbol)
{
    // unordered_map::insert used by SymbolMap::add keeps the first mapping and
    // silently ignores later ones for the same key.
    SymbolMap map;
    map.add("ETH", 4);
    map.add("ETH", 999);
    EXPECT_EQ(map.resolve("ETH"), 4);
}

// --- Asset-id range scheme mirrored from ExchangeRequestBuilder::initializeMapping ---
//
// Perp assets (default meta universe): raw index, e.g. 0, 1, 2, ...
// Spot tokens (spotMeta): token.index + 10000
// Dex-scoped perp assets (per-dex meta): 100000 + perpIdx * 10000 + index,
// where perpIdx is the 1-based position of the dex in perpDexs().dexes.

TEST(SymbolMapTest, PerpAssetsUseRawUniverseIndex)
{
    SymbolMap map;
    map.add("BTC", 0);
    map.add("ETH", 1);
    map.add("SOL", 2);

    EXPECT_EQ(map.resolve("BTC"), 0);
    EXPECT_EQ(map.resolve("ETH"), 1);
    EXPECT_EQ(map.resolve("SOL"), 2);
}

TEST(SymbolMapTest, SpotTokensAreOffsetByTenThousand)
{
    SymbolMap map;
    // spotMeta token.index values start at 0, same as perp indices, but must not
    // collide with perp asset ids once offset.
    map.add("PURR", 0 + 10000);
    map.add("HFUN", 1 + 10000);

    EXPECT_EQ(map.resolve("PURR"), 10000);
    EXPECT_EQ(map.resolve("HFUN"), 10001);
}

TEST(SymbolMapTest, PerpAndSpotIdsDoNotCollide)
{
    SymbolMap map;
    map.add("BTC", 0);        // perp asset index 0
    map.add("SOMECOIN", 0 + 10000); // spot token index 0

    EXPECT_EQ(map.resolve("BTC"), 0);
    EXPECT_EQ(map.resolve("SOMECOIN"), 10000);
}

TEST(SymbolMapTest, DexScopedPerpAssetsUsePerpIdxFormula)
{
    SymbolMap map;
    // First builder-dex (perpIdx = 1), two assets at index 0 and 1.
    int perpIdx = 1;
    map.add("DEX1-A", 100000 + (perpIdx * 10000) + 0);
    map.add("DEX1-B", 100000 + (perpIdx * 10000) + 1);

    // Second builder-dex (perpIdx = 2), asset at index 0.
    perpIdx = 2;
    map.add("DEX2-A", 100000 + (perpIdx * 10000) + 0);

    EXPECT_EQ(map.resolve("DEX1-A"), 110000);
    EXPECT_EQ(map.resolve("DEX1-B"), 110001);
    EXPECT_EQ(map.resolve("DEX2-A"), 120000);
}

TEST(SymbolMapTest, DexScopedIdsDoNotCollideWithPerpOrSpot)
{
    SymbolMap map;
    map.add("BTC", 0);
    map.add("PURR", 10000);
    map.add("DEX1-A", 100000 + (1 * 10000) + 0);

    EXPECT_EQ(map.resolve("BTC"), 0);
    EXPECT_EQ(map.resolve("PURR"), 10000);
    EXPECT_EQ(map.resolve("DEX1-A"), 110000);
}
