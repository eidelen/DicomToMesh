#include <gtest/gtest.h>
#include <cstdio>
#include "volumeVisualizer.h"

TEST(VolumeEntry, DefaultCtor)
{
    VolumeRenderingColoringEntry* defaultEntry = new VolumeRenderingColoringEntry();
    ASSERT_EQ(defaultEntry->m_red, 255u);
    ASSERT_EQ(defaultEntry->m_green, 255u);
    ASSERT_EQ(defaultEntry->m_blue, 255u);
    ASSERT_EQ(defaultEntry->m_alpha, 128u);
    ASSERT_EQ(defaultEntry->m_voxelValue, 0);
    delete defaultEntry;
}

TEST(VolumeEntry, Ctor)
{
    VolumeRenderingColoringEntry* defaultEntry = new VolumeRenderingColoringEntry(1u, 2u, 3u, 4u, 5);
    ASSERT_EQ(defaultEntry->m_red, 1u);
    ASSERT_EQ(defaultEntry->m_green, 2u);
    ASSERT_EQ(defaultEntry->m_blue, 3u);
    ASSERT_EQ(defaultEntry->m_alpha, 4u);
    ASSERT_EQ(defaultEntry->m_voxelValue, 5);
    delete defaultEntry;
}

