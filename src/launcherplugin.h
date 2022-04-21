/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAUNCHERPLUGIN_H
#define LAUNCHERPLUGIN_H

#include <DQmlAppPluginInterface>

class LauncherPlugin : public QObject, public DTK_QUICK_NAMESPACE::DQmlAppPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DQmlAppPluginInterface_iid)
    Q_INTERFACES(DTK_QUICK_NAMESPACE::DQmlAppPluginInterface)
public:
    LauncherPlugin(QObject *parent = nullptr);
    ~LauncherPlugin() override;

    int main(QGuiApplication *app, QQmlApplicationEngine *engine) override;
    QGuiApplication *createApplication(int &argc, char **argv) override;
};

#endif // LAUNCHERPLUGIN_H
