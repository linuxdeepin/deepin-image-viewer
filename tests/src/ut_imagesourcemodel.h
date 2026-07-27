// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_IMAGESOURCEMODEL_H
#define UT_IMAGESOURCEMODEL_H

#include <gtest/gtest.h>

class ImageSourceModel;

class ut_imagesourcemodel : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_IMAGESOURCEMODEL_H
