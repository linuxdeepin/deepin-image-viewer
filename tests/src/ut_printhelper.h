// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PRINTHELPER_H
#define UT_PRINTHELPER_H

#include <gtest/gtest.h>

class PrintHelper;
class RequestedSlot;

class ut_printhelper : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_PRINTHELPER_H
