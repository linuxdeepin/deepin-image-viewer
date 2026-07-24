// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CONFIGSETTER_H
#define UT_CONFIGSETTER_H

#include <gtest/gtest.h>

class LibConfigSetter;

class ut_configsetter : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_CONFIGSETTER_H
