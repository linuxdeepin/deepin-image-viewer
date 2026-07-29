// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_IMAGEPROVIDER_H
#define UT_IMAGEPROVIDER_H

#include <gtest/gtest.h>

class ProviderCache;
class ImageProvider;
class ThumbnailProvider;
class AsyncImageProvider;

class ut_imageprovider : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_IMAGEPROVIDER_H
