// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_UNIONIMAGE_H
#define UT_UNIONIMAGE_H

#include <gtest/gtest.h>

class ut_unionimage : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_UNIONIMAGE_H
