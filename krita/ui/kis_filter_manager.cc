/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "qsignalmapper.h"
#include <QLayout>
#include <QFrame>
#include <QTimer>
#include <QCursor>
#include <QApplication>
#include <QGridLayout>

#include <kmessagebox.h>
#include <kguiitem.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>

#include "KoID.h"

#include "kis_doc2.h"
#include "kis_filter.h"
#include "kis_filter_config_widget.h"
#include "kis_filter_manager.h"
#include "kis_filter_registry.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_previewdialog.h"
#include "kis_previewwidget.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_view2.h"
#include "kis_progress_display_interface.h"
#include "kis_threaded_applicator.h"
/*
class FilterJob : public KisJob
{
public:


};

class FilterJobFactory : public KisJobFactory
{
public:

    KisJobFactory()
        : KisJobFactory()
        , m_filter( filter )
        , m_config( config )
        {
        }
    ThreadWeaver::Job * createJob(QObject * parent, KisPaintDeviceSP dev,  const QRect & rc, int margin)
        {
        }
};
*/

class KisFilterManager::KisFilterManagerPrivate {
public:
    KisView2 * view;
    KisDoc2 * doc;

    QAction * reapplyAction;

    Q3PtrList<KAction> filterActions;

    KisFilterConfiguration * lastFilterConfig;
    KisFilter * lastFilter;
    KisPreviewDialog * lastDialog;
    KisFilterConfigWidget * lastWidget;

    QList<QString> filterList; // Map the actions in the signalmapper to the filters
    QSignalMapper * filterMapper;

    Q3Dict<KActionMenu> filterActionMenus;

    QTimer * timer;


    ~KisFilterManagerPrivate()
        {
            delete timer;
        }
};

KisFilterManager::KisFilterManager(KisView2 * view, KisDoc2 * doc)
{
    m_d = new KisFilterManagerPrivate();
    m_d->view = view;
    m_d->doc = doc;

// XXX: Store & restore last filter & last filter configuration in session settings
    m_d->reapplyAction = 0;
    m_d->lastFilterConfig = 0;
    m_d->lastDialog = 0;
    m_d->lastFilter = 0;
    m_d->lastWidget = 0;

    m_d->filterMapper = new QSignalMapper(this);
    m_d->timer = new QTimer( this );
    m_d->timer->setSingleShot( true );
    connect( m_d->timer, SIGNAL( timeout() ), this, SLOT(slotDelayedRefreshPreview()) );

    connect(m_d->filterMapper, SIGNAL(mapped(int)), this, SLOT(slotApplyFilter(int)));
}

KisFilterManager::~KisFilterManager()
{
    delete m_d;
}

void KisFilterManager::setup(KActionCollection * ac)
{
    KisFilterSP f;
    int i = 0;

    // Only create the submenu's we've actually got filters for.
    // XXX: Make this list extensible after 1.5

    KActionMenu * other = 0;
    KActionMenu * am = 0;

    m_d->filterList = KisFilterRegistry::instance()->keys();

    for ( QList<QString>::Iterator it = m_d->filterList.begin(); it != m_d->filterList.end(); ++it ) {
        f = KisFilterRegistry::instance()->value(*it);
        if (!f) break;

        QString s = f->menuCategory();
        if (s == "adjust" && !m_d->filterActionMenus.find("adjust")) {
            am  = new KActionMenu(i18n("Adjust"), this);
            ac->addAction("adjust_filters", am );
            m_d->filterActionMenus.insert("adjust", am);
        }

        else if (s == "artistic" && !m_d->filterActionMenus.find("artistic")) {
            am  = new KActionMenu(i18n("Artistic"), this);
            ac->addAction("artistic_filters", am );
            m_d->filterActionMenus.insert("artistic", am);
        }

        else if (s == "blur" && !m_d->filterActionMenus.find("blur")) {
            am  = new KActionMenu(i18n("Blur"), this);
            ac->addAction("blur_filters", am );
            m_d->filterActionMenus.insert("blur", am);
        }

        else if (s == "colors" && !m_d->filterActionMenus.find("colors")) {
            am  = new KActionMenu(i18n("Colors"), this);
            ac->addAction("color_filters", am );
            m_d->filterActionMenus.insert("colors", am);
        }

        else if (s == "decor" && !m_d->filterActionMenus.find("decor")) {
            am  = new KActionMenu(i18n("Decor"), this);
            ac->addAction("decor_filters", am );
            m_d->filterActionMenus.insert("decor", am);
        }

        else if (s == "edge" && !m_d->filterActionMenus.find("edge")) {
            am  = new KActionMenu(i18n("Edge Detection"), this);
            ac->addAction("edge_filters", am );
            m_d->filterActionMenus.insert("edge", am);
        }

        else if (s == "emboss" && !m_d->filterActionMenus.find("emboss")) {
            am  = new KActionMenu(i18n("Emboss"), this);
            ac->addAction("emboss_filters", am );
            m_d->filterActionMenus.insert("emboss", am);
        }

        else if (s == "enhance" && !m_d->filterActionMenus.find("enhance")) {
            am  = new KActionMenu(i18n("Enhance"), this);
            ac->addAction("enhance_filters", am );
            m_d->filterActionMenus.insert("enhance", am);
        }

        else if (s == "map" && !m_d->filterActionMenus.find("map")) {
            am  = new KActionMenu(i18n("Map"), this);
            ac->addAction("map_filters", am );
            m_d->filterActionMenus.insert("map", am);
        }

        else if (s == "nonphotorealistic" && !m_d->filterActionMenus.find("nonphotorealistic")) {
            am  = new KActionMenu(i18n("Non-photorealistic"), this);
            ac->addAction("nonphotorealistic_filters", am );
            m_d->filterActionMenus.insert("nonphotorealistic", am);
        }

        else if (s == "other" && !m_d->filterActionMenus.find("other")) {
            other  = new KActionMenu(i18n("Other"), this);
            ac->addAction("misc_filters", other );
            m_d->filterActionMenus.insert("other", am);
        }

    }

    m_d->reapplyAction  = new KAction(i18n("Apply Filter Again"), this);
    ac->addAction("filter_apply_again", m_d->reapplyAction );
    m_d->reapplyAction->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_F));
    connect(m_d->reapplyAction, SIGNAL(triggered()), this , SLOT(slotApply()));

    m_d->reapplyAction->setEnabled(false);

    f = 0;
    i = 0;
    for ( QList<QString>::Iterator it = m_d->filterList.begin(); it != m_d->filterList.end(); ++it ) {
        f = KisFilterRegistry::instance()->value(*it);

        if (!f) break;

        // Create action
        KAction * a = new KAction(f->menuEntry(), this);
        ac->addAction(QString("krita_filter_%1").arg((*it)).toAscii(), a);
        connect(a, SIGNAL(triggered()), m_d->filterMapper, SLOT(map()));


        // Add action to the right submenu
        KActionMenu * m = m_d->filterActionMenus.find( f->menuCategory() );
        if (m) {
            m->addAction(a);
        }
        else {
            if (!other) {
                other  = new KActionMenu(i18n("Other"), this);
                ac->addAction("misc_filters", other );
                m_d->filterActionMenus.insert("other", am);
            }
            other->addAction(a);
        }

        // Add filter to list of filters for mapper
        m_d->filterMapper->setMapping( a, i );

        m_d->filterActions.append( a );
        ++i;
    }
}

void KisFilterManager::updateGUI()
{
    kDebug(41007) << "Filter actions: " << m_d->filterActions.isEmpty() << endl;

    if ( m_d->filterActions.isEmpty() ) return;

    KisImageSP img = m_d->view->image();
    if (!img) return;

    KisLayerSP layer = m_d->view->activeLayer();
    if (!layer) return;

    bool enable =  !(layer->locked() || !layer->visible());
    KisPaintLayerSP player = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>( layer.data()));
    if(!player)
    {
        enable = false;
    }
    m_d->reapplyAction->setEnabled(m_d->lastFilterConfig);
    if (m_d->lastFilterConfig)
        m_d->reapplyAction->setText(i18n("Apply Filter Again") + ": "
            + KisFilterRegistry::instance()->value(m_d->lastFilterConfig->name())->name());
    else
        m_d->reapplyAction->setText(i18n("Apply Filter Again"));

    QAction * a;
    int i = 0;
    for (a = m_d->filterActions.first(); a; a = m_d->filterActions.next(), i++) {
        // XXX: This should always be true: investigate later! BSAR
        if (i < m_d->filterList.count()) {
            KisFilterSP filter = KisFilterRegistry::instance()->value(m_d->filterList[i]);
            if(player && filter->workWith( player->paintDevice()->colorSpace()))
            {
                a->setEnabled(enable);
            } else {
                a->setEnabled(false);
            }
        }
    }

}

void KisFilterManager::slotApply()
{
    apply();
}

bool KisFilterManager::apply()
{
    if (!m_d->lastFilter) return false;

    KisImageSP img = m_d->view->image();
    if (!img) return false;

    KisPaintDeviceSP dev = m_d->view->activeDevice();
    if (!dev) return false;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    //Apply the filter
    if(m_d->lastWidget)
    {
        m_d->lastFilterConfig = m_d->lastWidget->configuration();
    } else {
        m_d->lastFilterConfig = 0;
    }

    QRect r1 = dev->extent();
    QRect r2 = img->bounds();

    // Filters should work only on the visible part of an image.
    QRect rect = r1.intersect(r2);

    if (dev->hasSelection()) {
        QRect r3 = dev->selection()->selectedExactRect();
        rect = rect.intersect(r3);
    }

    m_d->lastFilter->enableProgress();

    //m_d->view->progressDisplay()->setSubject(m_d->lastFilter, true, true);
    //m_d->lastFilter->setProgressDisplay( m_d->view->progressDisplay());

    KisTransaction * cmd = 0;
    if (img->undo()) cmd = new KisTransaction(m_d->lastFilter->name(), dev);


    if ( !m_d->lastFilter->supportsThreading() ) {
        m_d->lastFilter->process(dev, rect, m_d->lastFilterConfig);
    }
    else {
        // Chop up in rects.
            m_d->lastFilter->process(dev, rect, m_d->lastFilterConfig);
    }



    m_d->reapplyAction->setEnabled(m_d->lastFilterConfig);
    if (m_d->lastFilterConfig)
        m_d->reapplyAction->setText(i18n("Apply Filter Again") + ": "
            + KisFilterRegistry::instance()->value(m_d->lastFilterConfig->name())->name());

    else
        m_d->reapplyAction->setText(i18n("Apply Filter Again"));

    if (m_d->lastFilter->cancelRequested()) {
        delete m_d->lastFilterConfig;
        if (cmd) {
            cmd->undo();
            delete cmd;
        }
        m_d->lastFilter->disableProgress();
        QApplication::restoreOverrideCursor();
        return false;

    } else {
        dev->setDirty(rect);
        m_d->doc->setModified(true);
        if (cmd) m_d->doc->addCommand(cmd);
        m_d->lastFilter->disableProgress();
        QApplication::restoreOverrideCursor();
        m_d->lastFilter->saveToBookmark(KisFilter::ConfigLastUsed.id(), m_d->lastFilterConfig);
        return true;
    }
}

void KisFilterManager::slotApplyFilter(int i)
{
    KisPreviewDialog * oldDialog = m_d->lastDialog;
    KisFilterConfiguration * oldConfig = m_d->lastFilterConfig;
    KisFilter * oldFilter = m_d->lastFilter;

    m_d->lastFilter = KisFilterRegistry::instance()->get(m_d->filterList[i]).data();

    if (!m_d->lastFilter) {
        m_d->lastFilter = oldFilter;
        return;
    }

    KisImageSP img = m_d->view->image();
    if (!img) return;

    KisPaintDeviceSP dev = m_d->view->activeDevice();
    if (!dev) return;

    if (dev->colorSpace()->willDegrade(m_d->lastFilter->colorSpaceIndependence())) {
        // Warning bells!
        if (m_d->lastFilter->colorSpaceIndependence() == TO_LAB16) {
            if (KMessageBox::warningContinueCancel(m_d->view,
                                               i18n("The %1 filter will convert your %2 data to 16-bit L*a*b* and vice versa. "
                                                       , m_d->lastFilter->name()
                                                       , dev->colorSpace()->name()),
                                               i18n("Filter Will Convert Your Layer Data"),
                                               KGuiItem(i18n("Continue")),
                                               KStandardGuiItem::cancel(),
                                               "lab16degradation") != KMessageBox::Continue) return;

        }
        else if (m_d->lastFilter->colorSpaceIndependence() == TO_RGBA8) {
            if (KMessageBox::warningContinueCancel(m_d->view,
                                               i18n("The %1 filter will convert your %2 data to 8-bit RGBA and vice versa. "
                                                       , m_d->lastFilter->name()
                                                       , dev->colorSpace()->name()),
                                               i18n("Filter Will Convert Your Layer Data"),
                                               KGuiItem(i18n("Continue")),
                                               KStandardGuiItem::cancel(),
                                               "rgba8degradation") != KMessageBox::Continue) return;
        }
    }

    m_d->lastFilter->disableProgress();

    // Create the config dialog
    m_d->lastDialog = new KisPreviewDialog(m_d->view, m_d->lastFilter->name().toAscii(), m_d->lastFilter->name());
    Q_CHECK_PTR(m_d->lastDialog);
    m_d->lastWidget = m_d->lastFilter->createConfigurationWidget( (QWidget*)m_d->lastDialog->container(), dev );


    if( m_d->lastWidget != 0)
    {
        m_d->lastWidget->setConfiguration(m_d->lastFilter->defaultConfiguration(dev));
        connect(m_d->lastWidget, SIGNAL(sigPleaseUpdatePreview()), this, SLOT(slotConfigChanged()));

        m_d->lastDialog->previewWidget()->slotSetDevice( dev );

        connect(m_d->lastDialog->previewWidget(), SIGNAL(updated()), this, SLOT(refreshPreview()));

        QGridLayout *widgetLayout = new QGridLayout(m_d->lastDialog->container());

        widgetLayout->addWidget(m_d->lastWidget, 0 , 0);

        m_d->lastDialog->container()->setMinimumSize(m_d->lastWidget->minimumSize());

        refreshPreview();

        if(m_d->lastDialog->exec() == QDialog::Rejected )
        {
            delete m_d->lastDialog;
            m_d->lastFilterConfig = oldConfig;
            m_d->lastDialog = oldDialog;
            m_d->lastFilter = oldFilter;
            return;
        }
    }

    // apply will crash if lastWidget == 0
    if (!apply()) {
        delete m_d->lastDialog;
        m_d->lastFilterConfig = oldConfig;
        m_d->lastDialog = oldDialog;
        m_d->lastFilter = oldFilter;
    }

}

void KisFilterManager::slotConfigChanged()
{
    if( m_d->lastDialog == 0 )
        return;
    if(m_d->lastDialog->previewWidget()->getAutoUpdate())
    {
        refreshPreview();
    } else {
        m_d->lastDialog->previewWidget()->needUpdate();
    }
}


void KisFilterManager::refreshPreview( )
{
    m_d->timer->start ( 500 );
}

void KisFilterManager::slotDelayedRefreshPreview()
{
    if( m_d->lastDialog == 0 )
        return;

    KisPaintDeviceSP dev = m_d->lastDialog->previewWidget()->getDevice();
    if (!dev) return;

    KisFilterConfiguration* config = m_d->lastWidget->configuration();

    QRect rect = dev->extent();
    KisTransaction cmd("Temporary transaction", dev);
    m_d->lastFilter->process(dev, rect, config);
    m_d->lastDialog->previewWidget()->slotUpdate();
    cmd.undo();
}


#include "kis_filter_manager.moc"
