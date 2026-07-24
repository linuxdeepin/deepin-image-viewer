// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_IMAGEINFO_H
#define UT_IMAGEINFO_H

#include <gtest/gtest.h>

class ImageInfo;

class ut_imageinfo : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_IMAGEINFO_H
