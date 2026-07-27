// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_ROTATEIMAGEHELPER_H
#define UT_ROTATEIMAGEHELPER_H

#include <gtest/gtest.h>

class RotateImageHelper;

class ut_rotateimagehelper : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_ROTATEIMAGEHELPER_H
