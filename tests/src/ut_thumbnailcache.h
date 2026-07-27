// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_THUMBNAILCACHE_H
#define UT_THUMBNAILCACHE_H

#include <gtest/gtest.h>

class ThumbnailCache;

class ut_thumbnailcache : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_THUMBNAILCACHE_H
