// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MOUSETRACKITEM_H
#define UT_MOUSETRACKITEM_H

#include <gtest/gtest.h>

class MouseTrackItem;

class ut_mousetrackitem : public testing::Test
{
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

#endif  // UT_MOUSETRACKITEM_H
