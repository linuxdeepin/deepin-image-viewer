// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_FILETRASHHELPER_H
#define UT_FILETRASHHELPER_H

#include <gtest/gtest.h>

class FileTrashHelper;

class ut_filetrashhelper : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_FILETRASHHELPER_H
