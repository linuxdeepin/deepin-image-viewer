// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_FILECONTROL_H
#define UT_FILECONTROL_H

#include <gtest/gtest.h>

class FileControl;

class ut_filecontrol : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_FILECONTROL_H
