// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_GLOBALCONTROL_H
#define UT_GLOBALCONTROL_H

#include <gtest/gtest.h>

class GlobalControl;

class ut_globalcontrol : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_GLOBALCONTROL_H
