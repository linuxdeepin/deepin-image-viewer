// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CURSORTOOL_H
#define UT_CURSORTOOL_H

#include <gtest/gtest.h>

class CursorTool;

class ut_cursortool : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_CURSORTOOL_H
