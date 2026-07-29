// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_IMAGEFILEWATCHER_H
#define UT_IMAGEFILEWATCHER_H

#include <gtest/gtest.h>

class ImageFileWatcher;

class ut_imagefilewatcher : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_IMAGEFILEWATCHER_H
