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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../shared/enums.h"
#include <QMainWindow>

class CheckUrlsDialog;
class QTreeView;
class QMenu;
class QModelIndex;
class QLineEdit;
class QLabel;
class QProgressDialog;
class TransferModel;
class TransferFilterModel;
class QWidgetAction;
class QPushButton;
class ValueSelectorAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onPluginsReady();
    void setNextAction(const QVariant &value);
    void onNextActionChanged(Transfers::Action action);
    void setTransferFilter(const QVariant &value);
    void setMaximumConcurrentTransfers(const QVariant &value);
    void onMaximumConcurrentTransfersChanged(int oldMax, int newMax);
    void setGlobalTransferConnections(const QVariant &value);
    void onGlobalTransferConnectionsChanged(int oldMax, int newMax);
    void setDownloadRateLimit(const QVariant &value);
    void onDownloadRateLimitChanged(int limit);
    void updateSpeed(int speed);
    void onPackageCountChanged(int count);
    void setTransferMenuActions();
    void showContextMenu(const QPoint &pos);
    void setConvertCurrentTransferToAudio();
    void startCurrentTransfer();
    void pauseCurrentTransfer();
    void removeCurrentTransfer();
    void showTransferConnectionsDialog();
    void showTransferPriorityDialog();
    void showTransferCategoryDialog();
    void showCurrentTransferProperties();
    void showCurrentPackageProperties();
    void setCurrentTransferConnections(const QVariant &connections);
    void setCurrentTransferCategory(const QVariant &category);
    void setCurrentTransferPriority(const QVariant &priority);
    void showAddUrlsDialog(const QString &text = QString(), const QString &fileName = QString());
    void addUrlsFromText(const QString &text, const QString &service);
    void showRetrieveUrlsDialog(const QString &text = QString(), const QString &fileName = QString());
    void retrieveUrlsFromText(const QString &text);
    void onUrlRetrieverFinished();
    void showTextFileDialog();
    void showAboutDialog();
    void showSettingsDialog();
    void showProgressDialog(const QString &message, int maximum = 100);
    void updateProgressDialog(int progress);
    void hideProgressDialog();
    
private:
    ValueSelectorAction *m_filterAction;
    ValueSelectorAction *m_nextAction;
    ValueSelectorAction *m_concurrentTransfersAction;
    ValueSelectorAction *m_connectionsAction;
    ValueSelectorAction *m_rateLimitAction;
    QAction *m_transferPropertiesAction;
    QAction *m_packagePropertiesAction;
    QAction *m_settingsAction;
    QAction *m_aboutAction;
    QToolBar *m_toolBar;
    QAction *m_addUrlsAction;
    QAction *m_importUrlsAction;
    QAction *m_retrieveUrlsAction;
    QAction *m_startAction;
    QAction *m_pauseAction;
    QLineEdit *m_searchEdit;
    QLabel *m_speedLabel;
    TransferModel *m_model;
    TransferFilterModel *m_filterModel;
    QTreeView *m_view;
    QMenu *m_contextMenu;
    QAction *m_transferConvertToAudioAction;
    QAction *m_transferStartAction;
    QAction *m_transferPauseAction;
    QAction *m_transferConnectionsAction;
    QAction *m_transferCategoryAction;
    QAction *m_transferPriorityAction;
    QAction *m_transferRemoveAction;
    CheckUrlsDialog *m_checkDialog;
    QProgressDialog *m_progressDialog;
    QPushButton *m_cancelButton;
};

#endif // MAINWINDOW_H
