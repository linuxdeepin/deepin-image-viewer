// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_IMAGEEDITCONTROLLER_H
#define UT_IMAGEEDITCONTROLLER_H

#include <gtest/gtest.h>

class ImageEditController;

class ut_imageeditcontroller : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_IMAGEEDITCONTROLLER_H
