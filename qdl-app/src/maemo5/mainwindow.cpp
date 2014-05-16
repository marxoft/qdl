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
#include "valueselectoraction.h"
#include "valuedialog.h"
#include "transferpropertiesdialog.h"
#include "packagepropertiesdialog.h"
#include "addurlsdialog.h"
#include "retrieveurlsdialog.h"
#include "settingsdialog.h"
#include "aboutdialog.h"
#include "checkurlsdialog.h"
#include "../shared/settings.h"
#include "../shared/urlchecker.h"
#include "../shared/urlretriever.h"
#include "../shared/pluginmanager.h"
#include "../shared/database.h"
#include "../shared/transfermodel.h"
#include "../shared/transfer.h"
#include "../shared/selectionmodels.h"
#include "../shared/clipboardmonitor.h"
#include <QTreeView>
#include <QMenu>
#include <QLineEdit>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFile>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QWidgetAction>
#include <QProgressDialog>
#include <QTimer>
#include <QMaemo5InformationBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_filterAction(new ValueSelectorAction(this)),
    m_nextAction(new ValueSelectorAction(this)),
    m_concurrentTransfersAction(new ValueSelectorAction(this)),
    m_connectionsAction(new ValueSelectorAction(this)),
    m_rateLimitAction(new ValueSelectorAction(this)),
    m_transferPropertiesAction(new QAction(tr("Download properties"), this)),
    m_packagePropertiesAction(new QAction(tr("Package properties"), this)),
    m_settingsAction(new QAction(tr("Settings"), this)),
    m_aboutAction(new QAction(tr("About"), this)),
    m_toolBar(new QToolBar(this)),
    m_addUrlsAction(new QAction(QIcon::fromTheme("general_add"), tr("Add URLs"), this)),
    m_importUrlsAction(new QAction(QIcon::fromTheme("general_move_to_folder"), tr("Import URLs"), this)),
    m_retrieveUrlsAction(new QAction(QIcon::fromTheme("general_search"), tr("Retrieve URLs"), this)),
    m_startAction(new QAction(QIcon("/etc/hildon/theme/mediaplayer/Play.png"), "", this)),
    m_pauseAction(new QAction(QIcon("/etc/hildon/theme/mediaplayer/Pause.png"), "", this)),
    m_searchEdit(new QLineEdit(this)),
    m_speedLabel(new QLabel("0 kB/s", this)),
    m_model(TransferModel::instance()),
    m_view(new QTreeView(this)),
    m_contextMenu(new QMenu(this)),
    m_transferConvertToAudioAction(m_contextMenu->addAction(tr("Convert to audio"), this, SLOT(setConvertCurrentTransferToAudio()))),
    m_transferStartAction(m_contextMenu->addAction(tr("Start"), this, SLOT(startCurrentTransfer()))),
    m_transferPauseAction(m_contextMenu->addAction(tr("Pause"), this, SLOT(pauseCurrentTransfer()))),
    m_transferConnectionsAction(m_contextMenu->addAction(tr("Connections"), this, SLOT(showTransferConnectionsDialog()))),
    m_transferCategoryAction(m_contextMenu->addAction(tr("Category"), this, SLOT(showTransferCategoryDialog()))),
    m_transferPriorityAction(m_contextMenu->addAction(tr("Priority"), this, SLOT(showTransferPriorityDialog()))),
    m_transferRemoveAction(m_contextMenu->addAction(tr("Remove"), this, SLOT(removeCurrentTransfer()))),
    m_checkDialog(new CheckUrlsDialog(this)),
    m_progressDialog(new QProgressDialog(this)),
    m_cancelButton(new QPushButton(tr("Cancel"), m_progressDialog))
{
    this->setWindowTitle(tr("QDL"));
    this->setCentralWidget(m_view);
    this->setAttribute(Qt::WA_Maemo5StackedWindow, true);
    this->addToolBar(Qt::BottomToolBarArea, m_toolBar);

    m_nextAction->setText(tr("After current download(s)"));
    m_nextAction->setModel(new TransferActionModel(this));
    m_nextAction->setValue(Transfers::Continue);

    m_filterAction->setText(tr("Show"));
    m_filterAction->setModel(new StatusFilterModel(this));
    m_filterAction->setValue(Transfers::Unknown);

    m_concurrentTransfersAction->setText(tr("Concurrent downloads"));
    m_concurrentTransfersAction->setModel(new ConcurrentTransfersModel(this));
    m_concurrentTransfersAction->setValue(Settings::instance()->maximumConcurrentTransfers());

    m_connectionsAction->setText(tr("Connections per download"));
    m_connectionsAction->setModel(new ConnectionsModel(this));
    m_connectionsAction->setValue(Settings::instance()->maximumConnectionsPerTransfer());

    m_rateLimitAction->setText(tr("Maximum download speed"));
    m_rateLimitAction->setModel(new DownloadRateLimitModel(this));
    m_rateLimitAction->setValue(Settings::instance()->downloadRateLimit());

    this->menuBar()->addAction(m_filterAction);
    this->menuBar()->addAction(m_nextAction);
    this->menuBar()->addAction(m_concurrentTransfersAction);
    this->menuBar()->addAction(m_connectionsAction);
    this->menuBar()->addAction(m_rateLimitAction);
    this->menuBar()->addAction(m_transferPropertiesAction);
    this->menuBar()->addAction(m_packagePropertiesAction);
    this->menuBar()->addAction(m_settingsAction);
    this->menuBar()->addAction(m_aboutAction);

    m_searchEdit->setPlaceholderText(tr("Search"));

    QLabel *speedIcon = new QLabel(this);
    speedIcon->setPixmap(QIcon::fromTheme("general_received").pixmap(m_toolBar->iconSize()));

    m_speedLabel->setMinimumWidth(m_speedLabel->fontMetrics().width(" 90000 kB/s"));
    m_speedLabel->setAlignment(Qt::AlignCenter);

    m_toolBar->addAction(m_addUrlsAction);
    m_toolBar->addAction(m_importUrlsAction);
    m_toolBar->addAction(m_retrieveUrlsAction);
    m_toolBar->addAction(m_startAction);
    m_toolBar->addAction(m_pauseAction);
    m_toolBar->addWidget(m_searchEdit);
    m_toolBar->addWidget(m_speedLabel);
    m_toolBar->addWidget(speedIcon);

    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QTreeView::SelectRows);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    m_view->setEditTriggers(QTreeView::NoEditTriggers);
    m_view->setHeaderHidden(true);
    m_view->setExpandsOnDoubleClick(true);
    m_view->setItemsExpandable(true);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setItemDelegate(new TransferItemDelegate(m_view));
    m_view->setFocus(Qt::OtherFocusReason);

    m_transferConvertToAudioAction->setCheckable(true);

    m_progressDialog->setWindowTitle(tr("Please wait"));
    m_progressDialog->setCancelButton(m_cancelButton);

    this->connect(m_filterAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setTransferFilter(QVariant)));
    this->connect(m_concurrentTransfersAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setMaximumConcurrentTransfers(QVariant)));
    this->connect(m_connectionsAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setGlobalTransferConnections(QVariant)));
    this->connect(m_rateLimitAction, SIGNAL(valueChanged(QVariant)), this, SLOT(setDownloadRateLimit(QVariant)));
    this->connect(m_transferPropertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentTransferProperties()));
    this->connect(m_packagePropertiesAction, SIGNAL(triggered()), this, SLOT(showCurrentPackageProperties()));
    this->connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));
    this->connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    this->connect(m_addUrlsAction, SIGNAL(triggered()), this, SLOT(showAddUrlsDialog()));
    this->connect(m_importUrlsAction, SIGNAL(triggered()), this, SLOT(showTextFileDialog()));
    this->connect(m_retrieveUrlsAction, SIGNAL(triggered()), this, SLOT(showRetrieveUrlsDialog()));
    this->connect(m_startAction, SIGNAL(triggered()), m_model, SLOT(start()));
    this->connect(m_pauseAction, SIGNAL(triggered()), m_model, SLOT(pause()));
    this->connect(m_searchEdit, SIGNAL(textChanged(QString)), m_model, SLOT(setSearchQuery(QString)));
    this->connect(m_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    this->connect(m_model, SIGNAL(totalDownloadSpeedChanged(int)), this, SLOT(updateSpeed(int)));
    this->connect(m_model, SIGNAL(countChanged(int)), this, SLOT(onPackageCountChanged(int)));
    this->connect(m_contextMenu, SIGNAL(aboutToShow()), this, SLOT(setTransferMenuActions()));
    this->connect(Settings::instance(), SIGNAL(maximumConcurrentTransfersChanged(int,int)), this, SLOT(onMaximumConcurrentTransfersChanged(int, int)));
    this->connect(Settings::instance(), SIGNAL(maximumConnectionsPerTransferChanged(int,int)), this, SLOT(onGlobalTransferConnectionsChanged(int, int)));
    this->connect(Settings::instance(), SIGNAL(downloadRateLimitChanged(int)), this, SLOT(onDownloadRateLimitChanged(int)));
    this->connect(ClipboardMonitor::instance(), SIGNAL(clipboardUrlsReady(QString)), this, SLOT(showAddUrlsDialog(QString)));
    this->connect(PluginManager::instance(), SIGNAL(busy(QString,int)), this, SLOT(showProgressDialog(QString,int)));
    this->connect(PluginManager::instance(), SIGNAL(progressChanged(int)), this, SLOT(updateProgressDialog(int)));
    this->connect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(onPluginsReady()));
    this->connect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(hideProgressDialog()));

    this->onPackageCountChanged(m_model->rowCount());
}

MainWindow::~MainWindow() {}

void MainWindow::showEvent(QShowEvent *event) {
    if (PluginManager::instance()->servicePlugins().isEmpty()) {
        QTimer::singleShot(1000, PluginManager::instance(), SLOT(loadPlugins()));
    }

    QMainWindow::showEvent(event);
}

void MainWindow::onPluginsReady() {
    this->disconnect(PluginManager::instance(), SIGNAL(pluginsReady()), this, SLOT(onPluginsReady()));
    m_model->restoreStoredTransfers();
}

void MainWindow::setNextAction(const QVariant &value) {
    m_model->setNextAction(static_cast<Transfers::Action>(value.toInt()));
}

void MainWindow::onNextActionChanged(Transfers::Action action) {
    m_nextAction->setValue(action);
}

void MainWindow::setTransferFilter(const QVariant &value) {
    m_model->setStatusFilter(static_cast<Transfers::Status>(value.toInt()));
}

void MainWindow::setMaximumConcurrentTransfers(const QVariant &value) {
    Settings::instance()->setMaximumConcurrentTransfers(value.toInt());
}

void MainWindow::onMaximumConcurrentTransfersChanged(int oldMax, int newMax) {
    if (oldMax == newMax) {
        return;
    }

    m_concurrentTransfersAction->setValue(newMax);
}

void MainWindow::setGlobalTransferConnections(const QVariant &value) {
    Settings::instance()->setMaximumConnectionsPerTransfer(value.toInt());
}

void MainWindow::onGlobalTransferConnectionsChanged(int oldMax, int newMax) {
    if (oldMax == newMax) {
        return;
    }

    m_connectionsAction->setValue(newMax);
}

void MainWindow::setDownloadRateLimit(const QVariant &value) {
    Settings::instance()->setDownloadRateLimit(value.toInt());
}

void MainWindow::onDownloadRateLimitChanged(int limit) {
    m_rateLimitAction->setValue(limit);
}

void MainWindow::updateSpeed(int speed) {
    m_speedLabel->setText(QString::number(speed) + " kB/s");
}

void MainWindow::onPackageCountChanged(int count) {
    m_filterAction->setEnabled(count > 0);
    m_nextAction->setEnabled(count > 0);
    m_transferPropertiesAction->setEnabled(count > 0);
    m_packagePropertiesAction->setEnabled(count > 0);
    m_startAction->setEnabled(count > 0);
    m_pauseAction->setEnabled(count > 0);
    m_searchEdit->setEnabled(count > 0);
}

void MainWindow::setTransferMenuActions() {
    QModelIndex index = m_view->currentIndex();

    if (!index.isValid()) {
        return;
    }

    m_transferConvertToAudioAction->setEnabled(index.data(Transfer::ConvertibleToAudioRole).toBool());
    m_transferConvertToAudioAction->setChecked((m_transferConvertToAudioAction->isEnabled()) && (index.data(Transfer::ConvertToAudioRole).toBool()));

    Transfers::Status status = static_cast<Transfers::Status>(index.data(Transfer::StatusRole).toInt());

    switch (status) {
    case Transfers::Paused:
    case Transfers::Failed:
        m_transferStartAction->setEnabled(true);
        m_transferPauseAction->setEnabled(false);
        break;
    default:
        m_transferStartAction->setEnabled(false);
        m_transferPauseAction->setEnabled(true);
    }
}

void MainWindow::showCurrentTransferProperties() {
    if (m_view->currentIndex().isValid()) {
        TransferPropertiesDialog *dialog = new TransferPropertiesDialog(m_model->get(m_view->currentIndex()), this);
        dialog->open();
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

        if (Transfer *package = m_model->get(index)) {
            PackagePropertiesDialog *dialog = new PackagePropertiesDialog(package, this);
            dialog->open();
        }
    }
}

void MainWindow::showContextMenu(const QPoint &pos) {
    m_contextMenu->popup(pos, m_transferConvertToAudioAction);
}

void MainWindow::setConvertCurrentTransferToAudio() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_view->currentIndex(), m_transferConvertToAudioAction->isChecked(), Transfer::ConvertToAudioRole);
    }
}

void MainWindow::startCurrentTransfer() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_view->currentIndex(), Transfers::Queued, Transfer::StatusRole);
    }
}

void MainWindow::pauseCurrentTransfer() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_view->currentIndex(), Transfers::Paused, Transfer::StatusRole);
    }
}

void MainWindow::removeCurrentTransfer() {
    if (m_view->currentIndex().isValid()) {
        m_model->setData(m_view->currentIndex(), Transfers::Cancelled, Transfer::StatusRole);
    }
}

void MainWindow::showTransferConnectionsDialog() {
    if (Transfer *transfer = m_model->get(m_view->currentIndex())) {
        ValueDialog *dialog = new ValueDialog(this);
        dialog->setWindowTitle(tr("Connections"));

        SelectionModel *model = new SelectionModel(dialog);

        for (int i = 1; i <= transfer->maximumConnections(); i++) {
            model->addItem(QString::number(i), i);
        }

        dialog->setModel(model);
        dialog->setValue(m_view->currentIndex().data(Transfer::PreferredConnectionsRole));
        dialog->open();
        this->connect(dialog, SIGNAL(valueChanged(QVariant)), this, SLOT(setCurrentTransferConnections(QVariant)));
    }
}

void MainWindow::setCurrentTransferConnections(const QVariant &connections) {
    m_model->setData(m_view->currentIndex(), connections, Transfer::PreferredConnectionsRole);
}

void MainWindow::showTransferCategoryDialog() {
    ValueDialog *dialog = new ValueDialog(this);
    dialog->setWindowTitle(tr("Category"));

    SelectionModel *model = new SelectionModel(dialog);

    foreach (QString category, Database::instance()->getCategoryNames()) {
        model->addItem(category, category);
    }

    dialog->setModel(model);
    dialog->setValue(m_view->currentIndex().data(Transfer::CategoryRole));
    dialog->open();
    this->connect(dialog, SIGNAL(valueChanged(QVariant)), this, SLOT(setCurrentTransferCategory(QVariant)));
}

void MainWindow::setCurrentTransferCategory(const QVariant &category) {
    m_model->setData(m_view->currentIndex(), category, Transfer::CategoryRole);
}

void MainWindow::showTransferPriorityDialog() {
    ValueDialog *dialog = new ValueDialog(this);
    dialog->setWindowTitle(tr("Priority"));
    dialog->setModel(new TransferPriorityModel(dialog));
    dialog->setValue(m_view->currentIndex().data(Transfer::PriorityRole));
    dialog->open();
    this->connect(dialog, SIGNAL(valueChanged(QVariant)), this, SLOT(setCurrentTransferPriority(QVariant)));
}

void MainWindow::setCurrentTransferPriority(const QVariant &priority) {
    m_model->setData(m_view->currentIndex(), priority, Transfer::PriorityRole);
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
        QMaemo5InformationBox::information(this, tr("No supported URLs found"));
    }
    else {
        this->showAddUrlsDialog(results);
        UrlRetriever::instance()->clearResults();
    }

    this->disconnect(UrlRetriever::instance(), 0, this, 0);
}

void MainWindow::showTextFileDialog() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Import URLs"), QDesktopServices::storageLocation(QDesktopServices::HomeLocation), "*.txt");

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
        m_cancelButton->setEnabled(true);
    }
    else {
        m_cancelButton->setEnabled(false);
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
