/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ThrottlePlugin.h"

#include <QApplication>
#include <QDesktopWidget>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <kis_action.h>
#include <kis_config.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>


#include "Throttle.h"

K_PLUGIN_FACTORY_WITH_JSON(ThrottlePluginFactory, "krita_throttle.json", registerPlugin<ThrottlePlugin>();)

class ThrottleDockerDockFactory : public KoDockFactoryBase
{
public:
    ThrottleDockerDockFactory()
    {
    }

    QString id() const override
    {
        return QStringLiteral("ThrottleDocker");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    QDockWidget* createDockWidget() override
    {
        QDockWidget *dockWidget = new BasicDocker();
        dockWidget->setObjectName(id());
        Throttle *throttle = new Throttle(dockWidget);
        dockWidget->setWidget(throttle);
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockMinimized;
    }

};


ThrottlePlugin::ThrottlePlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new ThrottleDockerDockFactory());
}

ThrottlePlugin::~ThrottlePlugin()
{
}


#include "ThrottlePlugin.moc"
