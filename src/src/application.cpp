// SPDX-FileCopyrightText: 2020-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

Application::~Application()
{
    emit sigQuit();
}
