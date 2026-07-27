// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_LIVETEXTANALYZER_H
#define UT_LIVETEXTANALYZER_H

#include <gtest/gtest.h>

class LiveTextAnalyzer;

class ut_livetextanalyzer : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_LIVETEXTANALYZER_H
