// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_types.h"
#include "types.h"

void ut_types::SetUp()
{
}

void ut_types::TearDown()
{
}

// 验证枚举值与预期一致
TEST_F(ut_types, ImageTypeEnumValues)
{
    EXPECT_EQ(static_cast<int>(Types::NullImage), 0);
    EXPECT_EQ(static_cast<int>(Types::NormalImage), 1);
    EXPECT_EQ(static_cast<int>(Types::DynamicImage), 2);
    EXPECT_EQ(static_cast<int>(Types::SvgImage), 3);
    EXPECT_EQ(static_cast<int>(Types::MultiImage), 4);
    EXPECT_EQ(static_cast<int>(Types::DamagedImage), 5);
    EXPECT_EQ(static_cast<int>(Types::NonexistImage), 6);
}

TEST_F(ut_types, StackPageEnumValues)
{
    EXPECT_EQ(static_cast<int>(Types::OpenImagePage), 0);
    EXPECT_EQ(static_cast<int>(Types::ImageViewPage), 1);
    EXPECT_EQ(static_cast<int>(Types::SliderShowPage), 2);
}

TEST_F(ut_types, ItemRoleEnumValues)
{
    EXPECT_EQ(static_cast<int>(Types::ImageUrlRole), Qt::UserRole + 1);
    EXPECT_EQ(static_cast<int>(Types::FrameIndexRole), Qt::UserRole + 2);
    EXPECT_EQ(static_cast<int>(Types::ImageAngleRole), Qt::UserRole + 3);
}
