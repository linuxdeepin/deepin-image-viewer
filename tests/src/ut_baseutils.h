// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BASEUTILS_H
#define UT_BASEUTILS_H

#include <gtest/gtest.h>

class ut_baseutils : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_BASEUTILS_H
