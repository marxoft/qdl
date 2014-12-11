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
#include "../shared/selectionmodels.h"
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
#ifdef TABLE_TRANSFER_VIEW
#include <QHeaderView>
#endif
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_actionToolBar(new QToolBar(this)),
    m_filterToolBar(new QToolBar(this)),
    m_bottomToolBar(new QToolBar(this)),
    m_startAction(new QAction(QIcon::fromTheme("media-playback-start"), tr("Start"), this)),
    m_pauseAction(new QAction(QIcon::fromTheme("media-playback-pause"), tr("Pause"), this)),
    m_searchEdit(new QLineEdit(this)),
    m_filterComboBox(new QComboBox(this)),
    m_nextActionComboBox(new QComboBox(this)),
    m_optionsButton(new QToolButton(this)),
    m_optionsMenu(new QMenu(m_optionsButton)),
    m_concurrentMenu(m_optionsMenu->addMenu(tr("Maximum concurrent downloads"))),
    m_concurrentGroup(new QActionGroup(this)),
    m_connectionsMenu(m_optionsMenu->addMenu(tr("Maximum connections per download"))),
    m_connectionsGroup(new QActionGroup(this)),
    m_rateLimitMenu(m_optionsMenu->addMenu(tr("Maximum download speed"))),
    m_rateLimitGroup(new QActionGroup(this)),
    m_activeTransfersLabel(new QLabel(QString("0 DLs"), this)),
    m_speedLabel(new QLabel("0 kB/s", this)),
    m_model(TransferModel::instance()),
    m_filterModel(new TransferFilterModel(this)),
    m_view(new QTreeView(this)),
    m_urlMenu(this->menuBar()->addMenu(tr("File"))),
    m_addUrlsAction(m_urlMenu->addAction(QIcon::fromTheme("list-add"), tr("Add URLs"),
                                         this, SLOT(showAddUrlsDialog()), Qt::CTRL + Qt::Key_A)),
    m_importUrlsAction(m_urlMenu->addAction(QIcon::fromTheme("document-open"), tr("Import URLs"),
                                            this, SLOT(showTextFileDialog()), Qt::CTRL + Qt::Key_I)),
    m_retrieveUrlsAction(m_urlMenu->addAction(QIcon::fromTheme("applications-internet"), tr("Retrieve URLs"),
                                              this, SLOT(showRetrieveUrlsDialog()), Qt::CTRL + Qt::Key_R)),
    m_quitAction(m_urlMenu->addAction(QIcon::fromTheme("system-shutdown"), tr("Quit"),
                                      QCoreApplication::instance(), SLOT(quit()), Qt::CTRL + Qt::Key_Q)),
    m_transferMenu(this->menuBar()->addMenu(tr("Download"))),
    m_transferPropertiesAction(m_transferMenu->addAction(QIcon::fromTheme("document-properties"), tr("Properties"),
                                                         this, SLOT(showCurrentTransferProperties()))),
    m_transferConvertToAudioAction(m_transferMenu->addAction(tr("Convert to audio file"),
                                                             this, SLOT(setConvertCurrentTransferToAudio()))),
    m_transferStartAction(m_transferMenu->addAction(QIcon::fromTheme("media-playback-start"), tr("Start"),
                                                    this, SLOT(startCurrentTransfer()))),
    m_transferPauseAction(m_transferMenu->addAction(QIcon::fromTheme("media-playback-pause"), tr("Pause"),
                                                    this, SLOT(pauseCurrentTransfer()))),
    m_transferConnectionsMenu(m_transferMenu->addMenu(tr("Connections"))),
    m_transferConnectionsGroup(new QActionGroup(this)),
    m_transferCategoryMenu(m_transferMenu->addMenu(tr("Category"))),
    m_transferCategoryGroup(new QActionGroup(this)),
    m_transferPriorityMenu(m_transferMenu->addMenu(tr("Priority"))),
    m_transferPriorityGroup(new QActionGroup(this)),
    m_transferHighPriorityAction(m_transferPriorityMenu->addAction(tr("High"),
                                                                   this, SLOT(setCurrentTransferPriority()))),
    m_transferNormalPriorityAction(m_transferPriorityMenu->addAction(tr("Normal"),
                                                                     this, SLOT(setCurrentTransferPriority()))),
    m_transferLowPriorityAction(m_transferPriorityMenu->addAction(tr("Low"),
                                                                  this, SLOT(setCurrentTransferPriority()))),
    m_transferRemoveAction(m_transferMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"),
                                                     this, SLOT(removeCurrentTransfer()))),
    m_packageMenu(this->menuBar()->addMenu(tr("Package"))),
    m_packagePropertiesAction(m_packageMenu->addAction(QIcon::fromTheme("document-properties"), tr("Properties"),
                                                       this, SLOT(showCurrentPackageProperties()))),
    m_packageStartAction(m_packageMenu->addAction(QIcon::fromTheme("media-playback-start"), tr("Start"),
                                                  this, SLOT(startCurrentPackage()))),
    m_packagePauseAction(m_packageMenu->addAction(QIcon::fromTheme("media-playback-pause"), tr("Pause"),
                                                  this, SLOT(pauseCurrentPackage()))),
    m_packageCategoryMenu(m_packageMenu->addMenu(tr("Category"))),
    m_packageCategoryGroup(new QActionGroup(this)),
    m_packagePriorityMenu(m_packageMenu->addMenu(tr("Priority"))),
    m_packagePriorityGroup(new QActionGroup(this)),
    m_packageHighPriorityAction(m_packagePriorityMenu->addAction(tr("High"), this, SLOT(setCurrentPackagePriority()))),
    m_packageNormalPriorityAction(m_packagePriorityMenu->addAction(tr("Normal"), this, SLOT(setCurrentPackagePriority()))),
    m_packageLowPriorityAction(m_packagePriorityMenu->addAction(tr("Low"), this, SLOT(setCurrentPackagePriority()))),
    m_packageRemoveAction(m_packageMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"),
                                                   this, SLOT(removeCurrentPackage()))),
    m_editMenu(this->menuBar()->addMenu(tr("Edit"))),
    m_preferencesAction(m_editMenu->addAction(QIcon::fromTheme("preferences-desktop"), tr("Preferences"),
                                              this, SLOT(showSettingsDialog()), Qt::CTRL + Qt::Key_P)),
    m_helpMenu(this->menuBar()->addMenu(tr("Help"))),
    m_aboutAction(m_helpMenu->addAction(QIcon::fromTheme("help-about"), tr("About"), this, SLOT(showAboutDialog()))),
    m_checkDialog(new CheckUrlsDialog(this)),
    m_progressDialog(new QProgressDialog(this)),
    m_progressDialogHasCancelButton(false),
    m_trayIcon(new QSystemTrayIcon(QIcon::fromTheme("qdl"), this)),
    m_trayMenu(new QMenu(this)),
    m_trayAddUrlsAction(new QAction(QIcon::fromTheme("list-add"), tr("Add URLs"), this)),
    m_trayImportUrlsAction(new QAction(QIcon::fromTheme("document-open"), tr("Import URLs"), this)),
    m_trayRetrieveUrlsAction(new QAction(QIcon::fromTheme("applications-internet"), tr("Retrieve URLs"), this)),
    m_trayPreferencesAction(new QAction(QIcon::fromTheme("preferences-desktop"), tr("Preferences"), this)),
    m_trayQuitAction(new QAction(QIcon::fromTheme("system-shutdown"), tr("Quit"), this))
{
    this->setWindowTitle("QDL");
    this->setCentralWidget(m_view);
    this->setAcceptDrops(true);
#ifdef TABLE_TRANSFER_VIEW
    this->setMinimumSize(1100, 600);
#else
    this->setMinimumSize(600, 600);
#endif

    for (int i = 1; i <= MAX_CONCURRENT_TRANSFERS; i++) {
        QAction *action = m_concurrentMenu->addAction(QString::number(i),
                                                      this, SLOT(setMaximumConcurrentTransfers()));
        action->setData(i);
        action->setCheckable(true);

        if (i == Settings::instance()->maximumConcurrentTransfers()) {
            action->setChecked(true);
        }

        m_concurrentGroup->addAction(action);
    }

    for (int i = 1; i <= MAX_CONNECTIONS; i++) {
        QAction *action = m_transferConnectionsMenu->addAction(QString::number(i),
                                                               this, SLOT(setCurrentTransferConnections()));
        QAction *globalAction = m_connectionsMenu->addAction(QString::number(i),
                                                             this, SLOT(setGlobalTransferConnections()));
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
        QAction *action = m_rateLimitMenu->addAction(QString::number(RATE_LIMITS.at(i) / 1000) + " kB/s",
                                                     this, SLOT(setDownloadRateLimit()));
        action->setData(RATE_LIMITS.at(i));
        action->setCheckable(true);

        if (RATE_LIMITS.at(i) == Settings::instance()->downloadRateLimit()) {
            action->setChecked(true);
        }

        m_rateLimitGroup->addAction(action);
    }

    m_searchEdit->setPlaceholderText(tr("Search"));

    m_filterComboBox->setModel(new StatusFilterModel(m_filterComboBox));

    m_nextActionComboBox->setModel(new TransferActionModel(m_nextActionComboBox));

    m_actionToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_actionToolBar->setMovable(false);
    m_actionToolBar->addAction(m_addUrlsAction);
    m_actionToolBar->addAction(m_importUrlsAction);
    m_actionToolBar->addAction(m_retrieveUrlsAction);
    m_actionToolBar->addSeparator();
    m_actionToolBar->addAction(m_startAction);
    m_actionToolBar->addAction(m_pauseAction);
    m_actionToolBar->addSeparator();
    m_actionToolBar->addWidget(new QLabel(tr("After current download(s)") + ":", this));
    m_actionToolBar->addWidget(m_nextActionComboBox);

    m_filterToolBar->setAllowedAreas(Qt::TopToolBarArea);
    m_filterToolBar->setMovable(false);
    m_filterToolBar->addWidget(new QLabel(tr("Show") + ":", this));
    m_filterToolBar->addWidget(m_filterComboBox);
    m_filterToolBar->addWidget(m_searchEdit);

    this->addToolBar(m_actionToolBar);
#ifndef TABLE_TRANSFER_VIEW
    this->addToolBarBreak();
#endif
    this->addToolBar(m_filterToolBar);

    m_optionsButton->setIcon(QIcon::fromTheme("document-properties"));
    m_optionsButton->setToolTip(tr("Options"));

    m_speedLabel->setMinimumWidth(m_speedLabel->fontMetrics().width(" 90000 kB/s"));
    m_speedLabel->setAlignment(Qt::AlignCenter);

    QLabel *speedIcon = new QLabel(this);
    speedIcon->setPixmap(QIcon::fromTheme("go-down").pixmap(m_bottomToolBar->iconSize()));

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QWidget *spacer2 = new QWidget(this);
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_bottomToolBar->setAllowedAreas(Qt::BottomToolBarArea);
    m_bottomToolBar->setMovable(false);
    m_bottomToolBar->addWidget(m_optionsButton);
    m_bottomToolBar->addWidget(spacer);
    m_bottomToolBar->addWidget(m_activeTransfersLabel);
    m_bottomToolBar->addWidget(spacer2);
    m_bottomToolBar->addWidget(m_speedLabel);
    m_bottomToolBar->addWidget(speedIcon);
    this->addToolBar(Qt::BottomToolBarArea, m_bottomToolBar);

    m_startAction->setToolTip(tr("Start all downloads"));
    m_pauseAction->setToolTip(tr("Pause all downloads"));

    m_startAction->setIconVisibleInMenu(true);
    m_pauseAction->setIconVisibleInMenu(true);
    m_addUrlsAction->setIconVisibleInMenu(true);
    m_importUrlsAction->setIconVisibleInMenu(true);
    m_retrieveUrlsAction->setIconVisibleInMenu(true);
    m_quitAction->setIconVisibleInMenu(true);
    m_transferPropertiesAction->setIconVisibleInMenu(true);
    m_transferConvertToAudioAction->setCheckable(true);
    m_transferStartAction->setIconVisibleInMenu(true);
    m_transferPauseAction->setIconVisibleInMenu(true);
    m_transferRemoveAction->setIconVisibleInMenu(true);
    m_packagePropertiesAction->setIconVisibleInMenu(true);
    m_packageStartAction->setIconVisibleInMenu(true);
    m_packagePauseAction->setIconVisibleInMenu(true);
    m_packageRemoveAction->setIconVisibleInMenu(true);
    m_preferencesAction->setIconVisibleInMenu(true);
    m_aboutAction->setIconVisibleInMenu(true);

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

    m_view->setModel(m_filterModel);
    m_view->setAlternatingRowColors(true);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setDragEnabled(true);
    m_view->setAcceptDrops(true);
    m_view->setDropIndicatorShown(true);
    m_view->setDragDropMode(QTreeView::InternalMove);
    m_view->setDefaultDropAction(Qt::MoveAction);
    m_view->setExpandsOnDoubleClick(true);
    m_view->setItemsExpandable(true);
    m_view->setIndentation(16);
    m_view->setUniformRowHeights(true);
    m_view->setAllColumnsShowFocus(true);
    m_view->setItemDelegate(new TransferItemDelegate(m_view));
#ifdef TABLE_TRANSFER_VIEW
    QHeaderView *header = m_view->header();
    QFontMetrics fm = header->fontMetrics();
    header->resizeSection(0, 350);
    header->resizeSection(1, fm.width("A long category name") + 20);
    header->resizeSection(2, fm.width(m_model->headerData(2).toString()) + 20);
    header->resizeSection(3, fm.width(m_model->headerData(3).toString()) + 20);
    header->resizeSection(4, fm.width("999.99MB of 999.99MB (99.99%)") + 20);
#else
    m_view->setHeaderHidden(true);
#endif

    m_progressDialog->setWindowTitle(tr("Please wait"));

    m_trayAddUrlsAction->setIconVisibleInMenu(true);
    m_trayImportUrlsAction->setIconVisibleInMenu(true);
    m_trayRetrieveUrlsAction->setIconVisibleInMenu(true);
    m_trayPreferencesAction->setIconVisibleInMenu(true);
    m_trayQuitAction->setIconVisibleInMenu(true);

    m_trayMenu->addAction(m_trayAddUrlsAction);
    m_trayMenu->addAction(m_trayImportUrlsAction);
    m_trayMenu->addAction(m_trayRetrieveUrlsAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_startAction);
    m_trayMenu->addAction(m_pauseAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addMenu(m_concurrentMenu);
    m_trayMenu->addMenu(m_connectionsMenu);
    m_trayMenu->addMenu(m_rateLimitMenu);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_trayPreferencesAction);
    m_trayMenu->addAction(m_trayQuitAction);
    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();

    this->connect(m_startAction, SIGNAL(triggered()), m_model, SLOT(start()));
    this->connect(m_pauseAction, SIGNAL(triggered()), m_model, SLOT(pause()));
    this->connect(m_searchEdit, SIGNAL(textChanged(QString)), m_filterModel, SLOT(setSearchQuery(QString)));
    this->connect(m_filterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onFilterBoxIndexChanged(int)));
    this->connect(m_nextActionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onNextActionIndexChanged(int)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_transferMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    this->connect(m_packageMenu, SIGNAL(aboutToShow()), this, SLOT(setPackageMenuActions()));
    this->connect(m_optionsButton, SIGNAL(clicked()), this, SLOT(showOptionsMenu()));
    this->connect(m_model, SIGNAL(countChanged(int)), this, SLOT(onPackageCountChanged(int)));
    this->connect(m_model, SIGNAL(nextActionChanged(Transfers::Action)),
                  this, SLOT(onNextActionChanged(Transfers::Action)));
    this->connect(m_trayAddUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    this->connect(m_trayImportUrlsAction, SIGNAL(triggered()), this, SLOT(showTextFileDialog()));
    this->connect(m_trayRetrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    this->connect(m_trayPreferencesAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    this->connect(m_trayQuitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));
    this->connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                  this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    this->connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int,int)),
                  this, SLOT(onMaximumConcurrentTransfersChanged(int, int)));
    this->connect(Settings::instance(), SIGNAL(maximumConnectionsPerTransferChanged(int,int)),
                  this, SLOT(onGlobalTransferConnectionsChanged(int, int)));
    this->connect(Settings::instance(), SIGNAL(downloadRateLimitChanged(int)),
                  this, SLOT(onDownloadRateLimitChanged(int)));
    this->connect(Database::instance(), SIGNAL(categoriesChanged()), this, SLOT(setCategoryMenuActions()));
    this->connect(ClipboardMonitor::instance(), SIGNAL(clipboardUrlsReady(QString)),
                  this, SLOT(showAddUrlsDialog(QString)));
    this->connect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(onPluginsReady()));

    this->onWindowStateChanged();
    this->onPackageCountChanged(m_model->rowCount());
    this->setCategoryMenuActions();
    
    PluginManager::instance()->loadPlugins();
}

MainWindow::~MainWindow() {}

bool MainWindow::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::WindowStateChange:
        this->onWindowStateChanged();
        break;
    default:
        break;
    }

    return QMainWindow::event(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if ((event->mimeData()->hasUrls()) && (event->mimeData()->urls().first().path().toLower().endsWith(".txt"))) {
        event->acceptProposedAction();
    }
    else if ((event->mimeData()->hasText()) && (QUrl::fromUserInput(event->mimeData()->text()).scheme().startsWith("http"))) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QString fileName = event->mimeData()->urls().first().path();

        if ((QFile::exists(fileName)) && (fileName.toLower().endsWith(".txt"))) {
            this->showAddUrlsDialog(QString(), fileName);
        }
    }
    else if (event->mimeData()->hasText()) {
        QUrl url = QUrl::fromUserInput(event->mimeData()->text());

        if (url.scheme().startsWith("http")) {
            this->showAddUrlsDialog(url.toString());
        }
    }
}

void MainWindow::onWindowStateChanged() {
    if (this->isMinimized()) {
        this->updateWindowTitle();
        this->connect(m_model, SIGNAL(activeTransfersChanged(int)), this, SLOT(updateWindowTitle()));
        this->connect(m_model, SIGNAL(totalDownloadSpeedChanged(int)), this, SLOT(updateWindowTitle()));
        this->disconnect(m_model, SIGNAL(totalDownloadSpeedChanged(int)), this, SLOT(updateSpeed(int)));
        this->disconnect(m_model, SIGNAL(activeTransfersChanged(int)), this, SLOT(updateActiveTransfers(int)));
    }
    else {
        this->setWindowTitle("QDL");
        this->updateSpeed(m_model->totalDownloadSpeed());
        this->updateActiveTransfers(m_model->activeTransfers());
        this->disconnect(m_model, SIGNAL(activeTransfersChanged(int)), this, SLOT(updateWindowTitle()));
        this->disconnect(m_model, SIGNAL(totalDownloadSpeedChanged(int)), this, SLOT(updateWindowTitle()));
        this->connect(m_model, SIGNAL(totalDownloadSpeedChanged(int)), this, SLOT(updateSpeed(int)));
        this->connect(m_model, SIGNAL(activeTransfersChanged(int)), this, SLOT(updateActiveTransfers(int)));
    }
}

void MainWindow::onPluginsReady() {
    this->disconnect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(onPluginsReady()));
    m_model->restoreStoredTransfers();
}

void MainWindow::onFilterBoxIndexChanged(int index) {
    m_filterModel->setStatusFilter(static_cast<Transfers::Status>(m_filterComboBox->itemData(index, Qt::UserRole + 1).toInt()));
}

void MainWindow::onNextActionIndexChanged(int index) {
    m_model->setNextAction(static_cast<Transfers::Action>(m_nextActionComboBox->itemData(index, Qt::UserRole + 1).toInt()));
}

void MainWindow::onNextActionChanged(Transfers::Action action) {
    m_nextActionComboBox->setCurrentIndex(m_nextActionComboBox->findData(action, Qt::UserRole + 1));
}

void MainWindow::onPackageCountChanged(int count) {
    if (count > 0) {
        m_view->setContextMenuPolicy(Qt::CustomContextMenu);
        m_transferMenu->setEnabled(true);
        m_packageMenu->setEnabled(true);
        m_startAction->setEnabled(true);
        m_pauseAction->setEnabled(true);
        m_nextActionComboBox->setEnabled(true);
        m_filterComboBox->setEnabled(true);
        m_searchEdit->setEnabled(true);
    }
    else {
        m_view->setContextMenuPolicy(Qt::NoContextMenu);
        m_transferMenu->setEnabled(false);
        m_packageMenu->setEnabled(false);
        m_startAction->setEnabled(false);
        m_pauseAction->setEnabled(false);
        m_nextActionComboBox->setEnabled(false);
        m_filterComboBox->setEnabled(false);
        m_searchEdit->setEnabled(false);
        m_searchEdit->clear();
    }
}

void MainWindow::showOptionsMenu() {
    m_optionsMenu->popup(m_optionsButton->mapToGlobal(m_optionsButton->rect().center()));
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

void MainWindow::updateActiveTransfers(int active) {
    m_activeTransfersLabel->setText(QString::number(active) + " DLs");
}

void MainWindow::updateWindowTitle() {
    this->setWindowTitle(QString("QDL | %1 DLs | %2 kB/s").arg(m_model->activeTransfers())
                                                          .arg(m_model->totalDownloadSpeed()));
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

    const int preferredConnections = index.data(Transfer::PreferredConnectionsRole).toInt();
    const int maximumConnections = index.data(Transfer::MaximumConnectionsRole).toInt();

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
        TransferPropertiesDialog *dialog = new TransferPropertiesDialog(m_model->get(m_filterModel->mapToSource(m_view->currentIndex())), this);
        dialog->open();
    }
}

void MainWindow::setConvertCurrentTransferToAudio() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), m_transferConvertToAudioAction->isChecked(), Transfer::ConvertToAudioRole);
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
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), action->data().toInt(), Transfer::PreferredConnectionsRole);
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
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), Transfers::HighPriority, Transfer::PriorityRole);
        }
        else if (m_transferPriorityGroup->checkedAction() == m_transferLowPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), Transfers::LowPriority, Transfer::PriorityRole);
        }
        else {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex()), Transfers::NormalPriority, Transfer::PriorityRole);
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
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                              : m_view->currentIndex()),
                         Transfers::Queued, Transfer::PackageStatusRole);
    }
}

void MainWindow::pauseCurrentPackage() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                              : m_view->currentIndex()),
                         Transfers::Paused, Transfer::PackageStatusRole);
    }
}

void MainWindow::removeCurrentPackage() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                              : m_view->currentIndex()),
                         Transfers::Cancelled, Transfer::PackageStatusRole);
    }
}

void MainWindow::setCurrentPackageCategory() {
    if (m_view->currentIndex().isValid()) {
        if (QAction *action = qobject_cast<QAction*>(this->sender())) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                                  : m_view->currentIndex()),
                             action->text(), Transfer::CategoryRole);
        }
    }
}

void MainWindow::setCurrentPackagePriority() {
    if (m_view->currentIndex().isValid()) {
        if (m_packagePriorityGroup->checkedAction() == m_packageHighPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                                  : m_view->currentIndex()),
                             Transfers::HighPriority, Transfer::PriorityRole);
        }
        else if (m_packagePriorityGroup->checkedAction() == m_packageLowPriorityAction) {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                                  : m_view->currentIndex()),
                             Transfers::LowPriority, Transfer::PriorityRole);
        }
        else {
            m_model->setData(m_filterModel->mapToSource(m_view->currentIndex().parent().isValid() ? m_view->currentIndex().parent()
                                                                                                  : m_view->currentIndex()),
                             Transfers::NormalPriority, Transfer::PriorityRole);
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
    this->connect(dialog, SIGNAL(urlsAvailable(QString,QString)), this, SLOT(addUrlsFromText(QString,QString)));
}

void MainWindow::addUrlsFromText(const QString &text, const QString &service) {
    m_checkDialog->open();
    UrlChecker::instance()->parseUrlsFromText(text, service);
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
#if QT_VERSION >= 0x050000
    QString filePath = QFileDialog::getOpenFileName(this, m_importUrlsAction->text(),
                                                    QStandardPaths::writableLocation(QStandardPaths::HomeLocation), "*.txt");
#else
    QString filePath = QFileDialog::getOpenFileName(this, m_importUrlsAction->text(),
                                                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation), "*.txt");
#endif
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
            m_progressDialog->setCancelButton(new QPushButton(QIcon::fromTheme("process-stop"), tr("Cancel"), m_progressDialog));
            m_progressDialogHasCancelButton = true;
        }
    }
    else {
        m_progressDialog->setCancelButton(0);
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

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        this->toggleWindowVisibility();
        return;
    default:
        return;
    }
}

void MainWindow::toggleWindowVisibility() {
    if (this->isHidden()) {
        this->show();
    }
    else if (this->isMinimized()) {
        this->showNormal();
    }
    else {
        this->hide();
    }
}
