// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_COMMANDPARSER_H
#define UT_COMMANDPARSER_H

#include <gtest/gtest.h>

class CommandParser;

class ut_commandparser : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_COMMANDPARSER_H
