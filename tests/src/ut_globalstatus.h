// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_GLOBALSTATUS_H
#define UT_GLOBALSTATUS_H

#include <gtest/gtest.h>

class GlobalStatus;

class ut_globalstatus : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_GLOBALSTATUS_H
