// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_APPLICATIONADPATOR_H
#define UT_APPLICATIONADPATOR_H

#include <gtest/gtest.h>

class ApplicationAdaptor;

class ut_applicationadpator : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_APPLICATIONADPATOR_H
