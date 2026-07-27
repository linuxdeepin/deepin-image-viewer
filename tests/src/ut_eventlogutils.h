// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_EVENTLOGUTILS_H
#define UT_EVENTLOGUTILS_H

#include <gtest/gtest.h>

class Eventlogutils;

class ut_eventlogutils : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_EVENTLOGUTILS_H
