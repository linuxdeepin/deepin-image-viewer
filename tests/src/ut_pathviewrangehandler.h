// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PATHVIEWRANGEHANDLER_H
#define UT_PATHVIEWRANGEHANDLER_H

#include <gtest/gtest.h>

class PathViewRangeHandler;

class ut_pathviewrangehandler : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_PATHVIEWRANGEHANDLER_H
