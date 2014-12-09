/*
 * Copyright (C) 2014 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "mainwindow.h"
#include "transferitemdelegate.h"
#include "addurlsdialog.h"
#include "retrieveurlsdialog.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "checkurlsdialog.h"
#include "transferpropertiesdialog.h"
#include "packagepropertiesdialog.h"
#include "../shared/database.h"
#include "../shared/urlchecker.h"
#include "../shared/urlretriever.h"
#include "../shared/pluginmanager.h"
#include "../shared/transfermodel.h"
#include "../shared/transferfiltermodel.h"
#include "../shared/transfer.h"
#include "../shared/settings.h"
#include "../shared/clipboardmonitor.h"
#include "../shared/definitions.h"
#include <QMenu>
#include <QTreeView>
#include <QToolBar>
#include <QLineEdit>
#include <QFileDialog>
#include <QFile>
#include <QMenuBar>
#include <QComboBox>
#include <QToolButton>
#include <QDropEvent>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QTimer>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDesktopServices>
#ifdef TABLE_TRANSFER_VIEW
#include <QHeaderView>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_toolBar(new QToolBar(this)),
    m_addUrlsAction(new QAction(QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_list_hw_button_plus"), QString(), this)),
    m_importUrlsAction(new QAction(QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_list_gene_fldr_opn"), QString(), this)),
    m_retrieveUrlsAction(new QAction(QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_list_browser"), QString(), this)),
    m_startAction(new QAction(QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_widg_mplayer_play"), QString(), this)),
    m_pauseAction(new QAction(QIcon("/usr/share/icons/hicolor/26x26/hildon/qgn_widg_mplayer_pause"), QString(), this)),
    m_searchEdit(new QLineEdit(this)),
    m_transferMenu(this->menuBar()->addMenu(tr("Download"))),
    m_transferPropertiesAction(m_transferMenu->addAction(tr("Properties"), this, SLOT(showCurrentTransferProperties()))),
    m_transferConvertToAudioAction(m_transferMenu->addAction(tr("Convert to audio file"), this, SLOT(setConvertCurrentTransferToAudio()))),
    m_transferStartAction(m_transferMenu->addAction(tr("Start"), this, SLOT(startCurrentTransfer()))),
    m_transferPauseAction(m_transferMenu->addAction(tr("Pause"), this, SLOT(pauseCurrentTransfer()))),
    m_transferConnectionsMenu(m_transferMenu->addMenu(tr("Connections"))),
    m_transferConnectionsGroup(new QActionGroup(this)),
    m_transferCategoryMenu(m_transferMenu->addMenu(tr("Category"))),
    m_transferCategoryGroup(new QActionGroup(this)),
    m_transferPriorityMenu(m_transferMenu->addMenu(tr("Priority"))),
    m_transferPriorityGroup(new QActionGroup(this)),
    m_transferHighPriorityAction(m_transferPriorityMenu->addAction(tr("High"), this, SLOT(setCurrentTransferPriority()))),
    m_transferNormalPriorityAction(m_transferPriorityMenu->addAction(tr("Normal"), this, SLOT(setCurrentTransferPriority()))),
    m_transferLowPriorityAction(m_transferPriorityMenu->addAction(tr("Low"), this, SLOT(setCurrentTransferPriority()))),
    m_transferRemoveAction(m_transferMenu->addAction(tr("Remove"), this, SLOT(removeCurrentTransfer()))),
    m_packageMenu(this->menuBar()->addMenu(tr("Package"))),
    m_packagePropertiesAction(m_packageMenu->addAction(tr("Properties"), this, SLOT(showCurrentPackageProperties()))),
    m_packageStartAction(m_packageMenu->addAction(tr("Start"), this, SLOT(startCurrentPackage()))),
    m_packagePauseAction(m_packageMenu->addAction(tr("Pause"), this, SLOT(pauseCurrentPackage()))),
    m_packageCategoryMenu(m_packageMenu->addMenu(tr("Category"))),
    m_packageCategoryGroup(new QActionGroup(this)),
    m_packagePriorityMenu(m_packageMenu->addMenu(tr("Priority"))),
    m_packagePriorityGroup(new QActionGroup(this)),
    m_packageHighPriorityAction(m_packagePriorityMenu->addAction(tr("High"), this, SLOT(setCurrentPackagePriority()))),
    m_packageNormalPriorityAction(m_packagePriorityMenu->addAction(tr("Normal"), this, SLOT(setCurrentPackagePriority()))),
    m_packageLowPriorityAction(m_packagePriorityMenu->addAction(tr("Low"), this, SLOT(setCurrentPackagePriority()))),
    m_packageRemoveAction(m_packageMenu->addAction(tr("Remove"), this, SLOT(removeCurrentPackage()))),
    m_optionsMenu(this->menuBar()->addMenu(tr("Options"))),
    m_transferStatusFilterMenu(m_optionsMenu->addMenu(tr("Show"))),
    m_allFilterAction(m_transferStatusFilterMenu->addAction(tr("All"), this, SLOT(setTransferStatusFilter()))),
    m_downloadingFilterAction(m_transferStatusFilterMenu->addAction(tr("Downloading"), this, SLOT(setTransferStatusFilter()))),
    m_queuedFilterAction(m_transferStatusFilterMenu->addAction(tr("Queued"), this, SLOT(setTransferStatusFilter()))),
    m_shortWaitFilterAction(m_transferStatusFilterMenu->addAction(tr("Waiting (short)"), this, SLOT(setTransferStatusFilter()))),
    m_longWaitFilterAction(m_transferStatusFilterMenu->addAction(tr("Waiting (long)"), this, SLOT(setTransferStatusFilter()))),
    m_captchaRequiredFilterAction(m_transferStatusFilterMenu->addAction(tr("Captcha required"), this, SLOT(setTransferStatusFilter()))),
    m_pausedFilterAction(m_transferStatusFilterMenu->addAction(tr("Paused"), this, SLOT(setTransferStatusFilter()))),
    m_failedFilterAction(m_transferStatusFilterMenu->addAction(tr("Failed"), this, SLOT(setTransferStatusFilter()))),
    m_transferStatusFilterGroup(new QActionGroup(this)),
    m_nextActionMenu(m_optionsMenu->addMenu(tr("After current download(s)"))),
    m_continueNextAction(m_nextActionMenu->addAction(tr("Continue"), this, SLOT(setNextAction()))),
    m_pauseNextAction(m_nextActionMenu->addAction(tr("Pause"), this, SLOT(setNextAction()))),
    m_quitNextAction(m_nextActionMenu->addAction(tr("Quit"), this, SLOT(setNextAction()))),
    m_nextActionGroup(new QActionGroup(this)),
    m_concurrentMenu(m_optionsMenu->addMenu(tr("Concurrent downloads"))),
    m_concurrentGroup(new QActionGroup(this)),
    m_connectionsMenu(m_optionsMenu->addMenu(tr("Connections per download"))),
    m_connectionsGroup(new QActionGroup(this)),
    m_rateLimitMenu(m_optionsMenu->addMenu(tr("Maximum download speed"))),
    m_rateLimitGroup(new QActionGroup(this)),
    m_viewMenu(this->menuBar()->addMenu(tr("View"))),
    m_fullscreenAction(m_viewMenu->addAction(tr("Full screen"), this, SLOT(setFullScreen(bool)))),
    m_preferencesAction(this->menuBar()->addAction(tr("Settings"), this, SLOT(showSettingsDialog()))),
    m_aboutAction(this->menuBar()->addAction(tr("About"), this, SLOT(showAboutDialog()))),
    m_quitAction(this->menuBar()->addAction(tr("Close"), QCoreApplication::instance(), SLOT(quit()))),
    m_speedLabel(new QLabel("0 kB/s", this)),
    m_model(TransferModel::instance()),
    m_filterModel(new TransferFilterModel(this)),
    m_view(new QTreeView(this)),
    m_checkDialog(new CheckUrlsDialog(this)),
    m_progressDialog(new QProgressDialog(this)),
    m_progressDialogHasCancelButton(false)
{
    this->setWindowTitle("QDL");
    this->setCentralWidget(m_view);
    this->addToolBar(Qt::BottomToolBarArea, m_toolBar);

    for (int i = 1; i <= MAX_CONCURRENT_TRANSFERS; i++) {
        QAction *action = m_concurrentMenu->addAction(QString::number(i), this, SLOT(setMaximumConcurrentTransfers()));
        action->setData(i);
        action->setCheckable(true);

        if (i == Settings::instance()->maximumConcurrentTransfers()) {
            action->setChecked(true);
        }

        m_concurrentGroup->addAction(action);
    }

    for (int i = 1; i <= MAX_CONNECTIONS; i++) {
        QAction *action = m_transferConnectionsMenu->addAction(QString::number(i), this, SLOT(setCurrentTransferConnections()));
        QAction *globalAction = m_connectionsMenu->addAction(QString::number(i), this, SLOT(setGlobalTransferConnections()));
        action->setData(i);
        action->setCheckable(true);
        globalAction->setData(i);
        globalAction->setCheckable(true);

        if (i == Settings::instance()->maximumConnectionsPerTransfer()) {
            globalAction->setChecked(true);
        }

        m_transferConnectionsGroup->addAction(action);
        m_connectionsGroup->addAction(globalAction);
    }

    QAction *unlimitedAction = m_rateLimitMenu->addAction(tr("Unlimited"), this, SLOT(setDownloadRateLimit()));
    unlimitedAction->setData(0);
    unlimitedAction->setCheckable(true);
    unlimitedAction->setChecked(Settings::instance()->downloadRateLimit() == 0);
    m_rateLimitGroup->addAction(unlimitedAction);
    m_rateLimitMenu->addSeparator();

    for (int i = 1; i < RATE_LIMITS.size(); i++) {
        QAction *action = m_rateLimitMenu->addAction(QString::number(RATE_LIMITS.at(i) / 1000) + " kB/s", this, SLOT(setDownloadRateLimit()));
        action->setData(RATE_LIMITS.at(i));
        action->setCheckable(true);

        if (RATE_LIMITS.at(i) == Settings::instance()->downloadRateLimit()) {
            action->setChecked(true);
        }

        m_rateLimitGroup->addAction(action);
    }

    m_speedLabel->setMinimumWidth(m_speedLabel->fontMetrics().width(" 90000 kB/s "));
    m_speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_toolBar->addAction(m_addUrlsAction);
    m_toolBar->addAction(m_importUrlsAction);
    m_toolBar->addAction(m_retrieveUrlsAction);
    m_toolBar->addAction(m_startAction);
    m_toolBar->addAction(m_pauseAction);
    m_toolBar->addWidget(m_searchEdit);
    m_toolBar->addWidget(m_speedLabel);

    m_continueNextAction->setData(Transfers::Continue);
    m_continueNextAction->setCheckable(true);
    m_continueNextAction->setChecked(true);
    m_continueNextAction->setActionGroup(m_nextActionGroup);
    m_pauseNextAction->setData(Transfers::Pause);
    m_pauseNextAction->setCheckable(true);
    m_pauseNextAction->setActionGroup(m_nextActionGroup);
    m_quitNextAction->setData(Transfers::Quit);
    m_quitNextAction->setCheckable(true);
    m_quitNextAction->setActionGroup(m_nextActionGroup);

    m_allFilterAction->setData(Transfers::Unknown);
    m_allFilterAction->setCheckable(true);
    m_allFilterAction->setChecked(true);
    m_allFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_downloadingFilterAction->setData(Transfers::Downloading);
    m_downloadingFilterAction->setCheckable(true);
    m_downloadingFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_queuedFilterAction->setData(Transfers::Queued);
    m_queuedFilterAction->setCheckable(true);
    m_queuedFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_shortWaitFilterAction->setData(Transfers::ShortWait);
    m_shortWaitFilterAction->setCheckable(true);
    m_shortWaitFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_longWaitFilterAction->setData(Transfers::LongWait);
    m_longWaitFilterAction->setCheckable(true);
    m_longWaitFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_captchaRequiredFilterAction->setData(Transfers::CaptchaRequired);
    m_captchaRequiredFilterAction->setCheckable(true);
    m_captchaRequiredFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_pausedFilterAction->setData(Transfers::Paused);
    m_pausedFilterAction->setCheckable(true);
    m_pausedFilterAction->setActionGroup(m_transferStatusFilterGroup);
    m_failedFilterAction->setData(Transfers::Failed);
    m_failedFilterAction->setCheckable(true);
    m_failedFilterAction->setActionGroup(m_transferStatusFilterGroup);

    m_nextActionGroup->setExclusive(true);
    m_transferStatusFilterGroup->setExclusive(true);
    m_concurrentGroup->setExclusive(true);
    m_connectionsGroup->setExclusive(true);
    m_rateLimitGroup->setExclusive(true);

    m_transferConnectionsGroup->setExclusive(true);
    m_transferCategoryGroup->setExclusive(true);
    m_transferPriorityGroup->setExclusive(true);
    m_transferHighPriorityAction->setCheckable(true);
    m_transferHighPriorityAction->setActionGroup(m_transferPriorityGroup);
    m_transferNormalPriorityAction->setCheckable(true);
    m_transferNormalPriorityAction->setActionGroup(m_transferPriorityGroup);
    m_transferLowPriorityAction->setCheckable(true);
    m_transferLowPriorityAction->setActionGroup(m_transferPriorityGroup);

    m_packageCategoryGroup->setExclusive(true);
    m_packagePriorityGroup->setExclusive(true);
    m_packageHighPriorityAction->setCheckable(true);
    m_packageHighPriorityAction->setActionGroup(m_packagePriorityGroup);
    m_packageNormalPriorityAction->setCheckable(true);
    m_packageNormalPriorityAction->setActionGroup(m_packagePriorityGroup);
    m_packageLowPriorityAction->setCheckable(true);
    m_packageLowPriorityAction->setActionGroup(m_packagePriorityGroup);

    m_fullscreenAction->setCheckable(true);

    m_preferencesAction->setShortcut(Qt::CTRL + Qt::Key_S);
    m_quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);

    m_view->setModel(m_filterModel);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setExpandsOnDoubleClick(true);
    m_view->setItemsExpandable(true);
    m_view->setItemDelegate(new TransferItemDelegate(m_view));
#ifdef TABLE_TRANSFER_VIEW
    QHeaderView *header = m_view->header();
    QFontMetrics fm = header->fontMetrics();
    header->resizeSection(0, 200);
    header->resizeSection(1, fm.width(m_model->headerData(1).toString()) + 20);
    header->resizeSection(2, fm.width(m_model->headerData(2).toString()) + 20);
    header->resizeSection(3, fm.width(m_model->headerData(3).toString()) + 20);
    header->resizeSection(4, fm.width(m_model->headerData(4).toString()) + 20);
#else
    m_view->setHeaderHidden(true);
#endif

    m_progressDialog->setWindowTitle(tr("Please wait"));

    this->connect(m_addUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    this->connect(m_importUrlsAction, SIGNAL(triggered()), this, SLOT(showTextFileDialog()));
    this->connect(m_retrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    this->connect(m_startAction, SIGNAL(triggered()), m_model, SLOT(start()));
    this->connect(m_pauseAction, SIGNAL(triggered()), m_model, SLOT(pause()));
    this->connect(m_searchEdit, SIGNAL(textChanged(QString)), m_filterModel, SLOT(setSearchQuery(QString)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_transferMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    this->connect(m_packageMenu, SIGNAL(aboutToShow()), this, SLOT(setPackageMenuActions()));
    this->connect(m_model, SIGNAL(countChanged(int)), this, SLOT(onPackageCountChanged(int)));
    this->connect(m_model, SIGNAL(totalDownloadSpeedChanged(int)), this, SLOT(updateSpeed(int)));
    this->connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int,int)), this, SLOT(onMaximumConcurrentTransfersChanged(int, int)));
    this->connect(Settings::instance(), SIGNAL(maximumConnectionsPerTransferChanged(int,int)), this, SLOT(onGlobalTransferConnectionsChanged(int, int)));
    this->connect(Settings::instance(), SIGNAL(downloadRateLimitChanged(int)), this, SLOT(onDownloadRateLimitChanged(int)));
    this->connect(Database::instance(), SIGNAL(categoriesChanged()), this, SLOT(setCategoryMenuActions()));
    this->connect(ClipboardMonitor::instance(), SIGNAL(clipboardUrlsReady(QString)), this, SLOT(showAddUrlsDialog(QString)));
    this->connect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(onPluginsReady()));

    this->onPackageCountChanged(m_model->rowCount());
    this->setCategoryMenuActions();
    
    PluginManager::instance()->loadPlugins();
}

MainWindow::~MainWindow() {}

bool MainWindow::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::WindowStateChange:
        m_fullscreenAction->setChecked(this->isFullScreen());
        break;
    case QEvent::ContextMenu:
        return false;
    default:
        break;
    }

    return QMainWindow::event(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) {
        if (event->modifiers() & Qt::ControlModifier) {
            switch (event->key()) {
            case Qt::Key_S:
                m_preferencesAction->trigger();
                event->accept();
                return;
            case Qt::Key_Q:
                m_quitAction->trigger();
                event->accept();
                return;
            default:
                QMainWindow::keyPressEvent(event);
                return;
            }
        }

        switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
            m_view->setFocus(Qt::OtherFocusReason);
            event->accept();
            return;
        default:
            break;
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::setFullScreen(bool fullScreen) {
    if (fullScreen) {
        this->showFullScreen();
    }
    else {
        this->showNormal();
    }
}

void MainWindow::onPluginsReady() {
    this->disconnect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(onPluginsReady()));
    m_model->restoreStoredTransfers();
}

void MainWindow::onPackageCountChanged(int count) {
    if (count > 0) {
        m_view->setContextMenuPolicy(Qt::CustomContextMenu);
        m_transferMenu->setEnabled(true);
        m_packageMenu->setEnabled(true);
        m_startAction->setEnabled(true);
        m_pauseAction->setEnabled(true);
        m_nextActionMenu->setEnabled(true);
        m_transferStatusFilterMenu->setEnabled(true);
        m_searchEdit->setEnabled(true);
    }
    else {
        m_view->setContextMenuPolicy(Qt::NoContextMenu);
        m_transferMenu->setEnabled(false);
        m_packageMenu->setEnabled(false);
        m_startAction->setEnabled(false);
        m_pauseAction->setEnabled(false);
        m_nextActionMenu->setEnabled(false);
        m_transferStatusFilterMenu->setEnabled(false);
        m_searchEdit->setEnabled(false);
        m_searchEdit->clear();
    }
}

void MainWindow::setNextAction() {
    if (QAction *action = m_nextActionGroup->checkedAction()) {
        m_model->setNextAction(static_cast<Transfers::Action>(action->data().toInt()));
    }
}

void MainWindow::onNextActionChanged(Transfers::Action action) {
    foreach (QAction *a, m_nextActionGroup->actions()) {
        if (a->data().toInt() == action) {
            a->setChecked(true);
            return;
        }
    }
}

void MainWindow::setTransferStatusFilter() {
    if (QAction *action = m_transferStatusFilterGroup->checkedAction()) {
        m_filterModel->setStatusFilter(static_cast<Transfers::Status>(action->data().toInt()));
    }
}

void MainWindow::setMaximumConcurrentTransfers() {
    if (QAction *action = m_concurrentGroup->checkedAction()) {
        Settings::instance()->setMaximumConcurrentTransfers(action->data().toInt());
    }
}

void MainWindow::onMaximumConcurrentTransfersChanged(int oldMax, int newMax) {
    if (oldMax == newMax) {
        return;
    }

    foreach (QAction *action, m_concurrentGroup->actions()) {
        if (action->data().toInt() == newMax) {
            action->setChecked(true);
            return;
        }
    }
}

void MainWindow::setGlobalTransferConnections() {
    if (QAction *action = m_connectionsGroup->checkedAction()) {
        Settings::instance()->setMaximumConnectionsPerTransfer(action->data().toInt());
    }
}

void MainWindow::onGlobalTransferConnectionsChanged(int oldMax, int newMax) {
    if (oldMax == newMax) {
        return;
    }

    foreach (QAction *action, m_connectionsGroup->actions()) {
        if (action->data().toInt() == newMax) {
            action->setChecked(true);
            return;
        }
    }
}

void MainWindow::setDownloadRateLimit() {
    if (QAction *action = m_rateLimitGroup->checkedAction()) {
        Settings::instance()->setDownloadRateLimit(action->data().toInt());
    }
}

void MainWindow::onDownloadRateLimitChanged(int limit) {
    foreach (QAction *action, m_rateLimitGroup->actions()) {
        if (action->data().toInt() == limit) {
            action->setChecked(true);
            return;
        }
    }
}

void MainWindow::updateSpeed(int speed) {
    m_speedLabel->setText(QString::number(speed) + " kB/s");
}

void MainWindow::setTransferMenuActions() {
    QModelIndex index = m_view->currentIndex();

    if (!index.isValid()) {
        return;
    }

    m_transferConvertToAudioAction->setEnabled(index.data(Transfer::ConvertibleToAudioRole).toBool());
    m_transferConvertToAudioAction->setChecked((m_transferConvertToAudioAction->isEnabled())
                                               && (index.data(Transfer::ConvertToAudioRole).toBool()));

    switch (index.data(Transfer::StatusRole).toInt()) {
    case Transfers::Paused:
    case Transfers::Failed:
        m_transferStartAction->setEnabled(true);
        m_transferPauseAction->setEnabled(false);
        break;
    default:
        m_transferStartAction->setEnabled(false);
        m_transferPauseAction->setEnabled(true);
    }

    int preferredConnections = index.data(Transfer::PreferredConnectionsRole).toInt();
    int maximumConnections = index.data(Transfer::MaximumConnectionsRole).toInt();

    for (int i = 0; i < m_transferConnectionsMenu->actions().size(); i++) {
        m_transferConnectionsMenu->actions().at(i)->setEnabled(i < maximumConnections);
    }

    if ((preferredConnections > 0) && (m_transferConnectionsMenu->actions().count() >= preferredConnections)) {
        m_transferConnectionsMenu->actions().at(preferredConnections - 1)->setChecked(true);
    }

    switch (index.data(Transfer::PriorityRole).toInt()) {
    case Transfers::HighPriority:
        m_transferHighPriorityAction->setChecked(true);
        break;
    case Transfers::LowPriority:
        m_transferLowPriorityAction->setChecked(true);
        break;
    default:
        m_transferNormalPriorityAction->setChecked(true);
    }

    QString category = index.data(Transfer::CategoryRole).toString();
    bool found = false;
    int i = 0;

    while ((!found) && (i < m_transferCategoryMenu->actions().size())) {
        found = m_transferCategoryMenu->actions().at(i)->text() == category;
        i++;
    }

    if (found) {
        m_transferCategoryMenu->actions().at(i - 1)->setChecked(true);
    }
}

void MainWindow::setPackageMenuActions() {
    QModelIndex index = m_view->currentIndex();

    if (!index.isValid()) {
        return;
    }

    if (index.parent().isValid()) {
        index = index.parent();
    }

    switch (index.data(Transfer::StatusRole).toInt()) {
    case Transfers::Paused:
    case Transfers::Failed:
        m_packageStartAction->setEnabled(true);
        m_packagePauseAction->setEnabled(false);
        break;
    default:
        m_packageStartAction->setEnabled(false);
        m_packagePauseAction->setEnabled(true);
    }

    switch (index.data(Transfer::PriorityRole).toInt()) {
    case Transfers::HighPriority:
        m_packageHighPriorityAction->setChecked(true);
        break;
    case Transfers::LowPriority:
        m_packageLowPriorityAction->setChecked(true);
        break;
    default:
        m_packageNormalPriorityAction->setChecked(true);
    }

    QString category = index.data(Transfer::CategoryRole).toString();
    bool found = false;
    int i = 0;

    while ((!found) && (i < m_packageCategoryMenu->actions().size())) {
        found = m_packageCategoryMenu->actions().at(i)->text() == category;
        i++;
    }

    if (found) {
        m_packageCategoryMenu->actions().at(i - 1)->setChecked(true);
    }
}

void MainWindow::setCategoryMenuActions() {
    m_transferCategoryMenu->clear();
    m_packageCategoryMenu->clear();

    QStringList categories = Database::instance()->getCategoryNames();

    m_transferCategoryMenu->setEnabled(!categories.isEmpty());
    m_packageCategoryMenu->setEnabled(!categories.isEmpty());

    foreach (QString category, categories) {
        QAction *transferAction = m_transferCategoryMenu->addAction(category, this, SLOT(setCurrentTransferCategory()));
        transferAction->setCheckable(true);
        transferAction->setActionGroup(m_transferCategoryGroup);

        QAction *packageAction = m_packageCategoryMenu->addAction(category, this, SLOT(setCurrentPackageCategory()));
        packageAction->setCheckable(true);
        packageAction->setActionGroup(m_packageCategoryGroup);
    }
}

void MainWindow::showContextMenu(const QPoint &pos) {
    m_transferMenu->popup(m_view->mapToGlobal(pos), m_transferPropertiesAction);
}

void MainWindow::showCurrentTransferProperties() {
    if (m_view->currentIndex().isValid()) {
        TransferPropertiesDialog *dialog = 
        new TransferPropertiesDialog(m_model->get(m_filterModel->mapToSource(m_view->currentIndex())), this);
        dialog->open();
    }
}

void MainWindow::setConvertCurrentTransferToAudio() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), 
                         m_transferConvertToAudioAction->isChecked(), Transfer::ConvertToAudioRole);
    }
}

void MainWindow::startCurrentTransfer() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), Transfers::Queued, Transfer::StatusRole);
    }
}

void MainWindow::pauseCurrentTransfer() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), Transfers::Paused, Transfer::StatusRole);
    }
}

void MainWindow::removeCurrentTransfer() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), Transfers::Cancelled, Transfer::StatusRole);
    }
}

void MainWindow::setCurrentTransferConnections() {
    if (m_view->currentIndex().isValid()) {
        if (QAction *action = m_transferConnectionsGroup->checkedAction()) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()),
                             action->data().toInt(), Transfer::PreferredConnectionsRole);
        }
    }
}

void MainWindow::setCurrentTransferCategory() {
    if (m_view->currentIndex().isValid()) {
        if (QAction *action = qobject_cast<QAction*>(this->sender())) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), action->text(), Transfer::CategoryRole);
        }
    }
}

void MainWindow::setCurrentTransferPriority() {
    if (m_view->currentIndex().isValid()) {
        if (m_transferPriorityGroup->checkedAction() == m_transferHighPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()),
                             Transfers::HighPriority, Transfer::PriorityRole);
        }
        else if (m_transferPriorityGroup->checkedAction() == m_transferLowPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()),
                             Transfers::LowPriority, Transfer::PriorityRole);
        }
        else {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()),
                             Transfers::NormalPriority, Transfer::PriorityRole);
        }
    }
}

void MainWindow::showCurrentPackageProperties() {
    if (m_view->currentIndex().isValid()) {
        QModelIndex index;

        if (m_view->currentIndex().parent().parent().isValid()) {
            index = m_view->currentIndex().parent();
        }
        else {
            index = m_view->currentIndex();
        }

        if (Transfer *package = m_model->get(m_filterModel->mapToSource(index))) {
            PackagePropertiesDialog *dialog = new PackagePropertiesDialog(package, this);
            dialog->open();
        }
    }
}

void MainWindow::startCurrentPackage() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                    ? m_view->currentIndex().parent()
                                                    : m_view->currentIndex()),
                                                    Transfers::Queued, Transfer::PackageStatusRole);
    }
}

void MainWindow::pauseCurrentPackage() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                    ? m_view->currentIndex().parent()
                                                    : m_view->currentIndex()),
                                                    Transfers::Paused, Transfer::PackageStatusRole);
    }
}

void MainWindow::removeCurrentPackage() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                    ? m_view->currentIndex().parent()
                                                    : m_view->currentIndex()),
                                                    Transfers::Cancelled, Transfer::PackageStatusRole);
    }
}

void MainWindow::setCurrentPackageCategory() {
    if (m_view->currentIndex().isValid()) {
        if (QAction *action = qobject_cast<QAction*>(this->sender())) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                        ? m_view->currentIndex().parent()
                                                        : m_view->currentIndex()),
                                                        action->text(),
                                                        Transfer::CategoryRole);
        }
    }
}

void MainWindow::setCurrentPackagePriority() {
    if (m_view->currentIndex().isValid()) {
        if (m_packagePriorityGroup->checkedAction() == m_packageHighPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                        ? m_view->currentIndex().parent()
                                                        : m_view->currentIndex()),
                                                        Transfers::HighPriority,
                                                        Transfer::PriorityRole);
        }
        else if (m_packagePriorityGroup->checkedAction() == m_packageLowPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                        ? m_view->currentIndex().parent()
                                                        : m_view->currentIndex()),
                                                        Transfers::LowPriority,
                                                        Transfer::PriorityRole);
        }
        else {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid()
                                                        ? m_view->currentIndex().parent()
                                                        : m_view->currentIndex()),
                                                        Transfers::NormalPriority,
                                                        Transfer::PriorityRole);
        }
    }
}

void MainWindow::showAddUrlsDialog(const QString &text, const QString &fileName) {
    AddUrlsDialog *dialog = new AddUrlsDialog(this);

    if (!fileName.isEmpty()) {
        dialog->parseUrlsFromTextFile(fileName);
    }
    else {
        dialog->setText(text);
    }

    dialog->open();
    this->connect(dialog, SIGNAL(urlsAvailable(QString)), this, SLOT(addUrlsFromText(QString)));
}

void MainWindow::addUrlsFromText(const QString &text) {
    m_checkDialog->open();
    UrlChecker::instance()->parseUrlsFromText(text);
}

void MainWindow::showRetrieveUrlsDialog(const QString &text, const QString &fileName) {
    RetrieveUrlsDialog *dialog = new RetrieveUrlsDialog(this);

    if (!fileName.isEmpty()) {
        dialog->parseUrlsFromTextFile(fileName);
    }
    else {
        dialog->setText(text);
    }

    dialog->open();
    this->connect(dialog, SIGNAL(urlsAvailable(QString)), this, SLOT(retrieveUrlsFromText(QString)));
}

void MainWindow::retrieveUrlsFromText(const QString &text) {
    this->disconnect(UrlRetriever::instance(), 0, this, 0);
    this->connect(UrlRetriever::instance(), SIGNAL(busy(QString,int)), this, SLOT(showProgressDialog(QString,int)));
    this->connect(UrlRetriever::instance(), SIGNAL(progressChanged(int)), this, SLOT(updateProgressDialog(int)));
    this->connect(UrlRetriever::instance(), SIGNAL(finished()), this, SLOT(onUrlRetrieverFinished()));
    this->connect(UrlRetriever::instance(), SIGNAL(finished()), this, SLOT(hideProgressDialog()));
    this->connect(UrlRetriever::instance(), SIGNAL(cancelled()), this, SLOT(hideProgressDialog()));
    UrlRetriever::instance()->parseUrlsFromText(text);
}

void MainWindow::onUrlRetrieverFinished() {
    QString results = UrlRetriever::instance()->resultsString();

    if (results.isEmpty()) {
        QMessageBox::information(this, tr("Retrieve URLs"), tr("No supported URLs found"));
    }
    else {
        this->showAddUrlsDialog(results);
        UrlRetriever::instance()->clearResults();
    }

    this->disconnect(UrlRetriever::instance(), 0, this, 0);
}

void MainWindow::showTextFileDialog() {
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    m_importUrlsAction->text(),
                                                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation),
                                                    "*.txt");

    if (!filePath.isEmpty()) {
        this->showAddUrlsDialog(QString(), filePath);
    }
}

void MainWindow::showSettingsDialog() {
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->open();
}

void MainWindow::showAboutDialog() {
    AboutDialog *dialog = new AboutDialog(this);
    dialog->open();
}

void MainWindow::showProgressDialog(const QString &message, int maximum) {
    m_progressDialog->setLabelText(message);
    m_progressDialog->setRange(0, maximum);

    if ((this->sender()) && (this->connect(m_progressDialog, SIGNAL(canceled()), this->sender(), SLOT(cancel())))) {
        if (!m_progressDialogHasCancelButton) {
            m_progressDialog->setCancelButtonText(tr("Cancel"));
            m_progressDialogHasCancelButton = true;
        }
    }
    else {
        m_progressDialog->setCancelButtonText(QString());
        m_progressDialogHasCancelButton = false;
    }

    m_progressDialog->open();
}

void MainWindow::updateProgressDialog(int progress) {
    m_progressDialog->setValue(progress);

    if (progress >= m_progressDialog->maximum()) {
        this->hideProgressDialog();
    }
}

void MainWindow::hideProgressDialog() {
    m_progressDialog->reset();
    this->disconnect(m_progressDialog, SIGNAL(canceled()), 0, 0);
}
