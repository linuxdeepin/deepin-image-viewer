// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_OCRINTERFACE_H
#define UT_OCRINTERFACE_H

#include <gtest/gtest.h>

class OcrInterface;

class ut_ocrinterface : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_OCRINTERFACE_H
