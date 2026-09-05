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
// Spot pairs (spotMetaAndAssetCtxs): 10000 + the pair's own `index` field (mirrors the
// official Python SDK). Every pair also gets a "BASE/QUOTE" convenience alias built from its
// token names, regardless of canonical status - first pair to claim a given "BASE/QUOTE"
// string wins.
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

TEST(SymbolMapTest, SpotPairsAreOffsetByTenThousand)
{
    SymbolMap map;
    // Pair's own `index` field, offset by 10000.
    map.add("PURR/USDC", 0 + 10000);
    map.add("HFUN/USDC", 1 + 10000);

    EXPECT_EQ(map.resolve("PURR/USDC"), 10000);
    EXPECT_EQ(map.resolve("HFUN/USDC"), 10001);
}

TEST(SymbolMapTest, SpotAssetIdUsesPairsOwnIndexFieldNotArrayPosition)
{
    // Mirrors the official Python SDK: asset id is 10000 + the pair's own `index` field,
    // regardless of where the pair actually sits in the universe array.
    SymbolMap map;
    map.add("@1035", 1035 + 10000);

    EXPECT_EQ(map.resolve("@1035"), 11035);
}

TEST(SymbolMapTest, EveryPairGetsABaseQuoteAliasRegardlessOfCanonicalStatus)
{
    // First pair to claim a given "BASE/QUOTE" string wins - applies even to non-canonical,
    // auto-named pairs, matching the Python SDK's unconditional aliasing.
    SymbolMap map;
    map.add("@1", 1 + 10000);
    map.add("PUCKY/USDC", 1 + 10000);

    EXPECT_EQ(map.resolve("@1"), 10001);
    EXPECT_EQ(map.resolve("PUCKY/USDC"), 10001);
}

TEST(SymbolMapTest, PerpAndSpotIdsDoNotCollide)
{
    SymbolMap map;
    map.add("BTC", 0);        // perp asset index 0
    map.add("SOMECOIN", 0 + 10000); // spot pair position 0

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
