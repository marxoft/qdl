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
#include <QSystemTrayIcon>

class QLineEdit;
class QComboBox;
class CheckUrlsDialog;
class QTreeView;
class QMenu;
class QActionGroup;
class QToolButton;
class QLabel;
class QProgressDialog;
class QMessageBox;
class TransferModel;
class TransferFilterModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool event(QEvent *event);
    void showEvent(QShowEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void onWindowStateChanged();
    void onPluginsReady();
    void onFilterBoxIndexChanged(int index);
    void onNextActionIndexChanged(int index);
    void onNextActionChanged(Transfers::Action action);
    void onPackageCountChanged(int count);
    void showOptionsMenu();
    void setMaximumConcurrentTransfers();
    void onMaximumConcurrentTransfersChanged(int oldMax, int newMax);
    void setGlobalTransferConnections();
    void onGlobalTransferConnectionsChanged(int oldMax, int newMax);
    void setDownloadRateLimit();
    void onDownloadRateLimitChanged(int limit);
    void updateSpeed(int speed);
    void updateActiveTransfers(int active);
    void updateWindowTitle();
    void setTransferMenuActions();
    void setPackageMenuActions();
    void setCategoryMenuActions();
    void showContextMenu(const QPoint &pos);
    void showCurrentTransferProperties();
    void setConvertCurrentTransferToAudio();
    void startCurrentTransfer();
    void pauseCurrentTransfer();
    void removeCurrentTransfer();
    void setCurrentTransferConnections();
    void setCurrentTransferCategory();
    void setCurrentTransferPriority();
    void showCurrentPackageProperties();
    void startCurrentPackage();
    void pauseCurrentPackage();
    void removeCurrentPackage();
    void setCurrentPackageCategory();
    void setCurrentPackagePriority();
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
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleWindowVisibility();
    
private:
    QToolBar *m_actionToolBar;
    QToolBar *m_filterToolBar;
    QToolBar *m_bottomToolBar;
    QAction *m_startAction;
    QAction *m_pauseAction;
    QLineEdit *m_searchEdit;
    QComboBox *m_filterComboBox;
    QComboBox *m_nextActionComboBox;
    QToolButton *m_optionsButton;
    QMenu *m_optionsMenu;
    QMenu *m_concurrentMenu;
    QActionGroup *m_concurrentGroup;
    QMenu *m_connectionsMenu;
    QActionGroup *m_connectionsGroup;
    QMenu *m_rateLimitMenu;
    QActionGroup *m_rateLimitGroup;
    QLabel *m_activeTransfersLabel;
    QLabel *m_speedLabel;
    TransferModel *m_model;
    TransferFilterModel *m_filterModel;
    QTreeView *m_view;
    QMenu *m_urlMenu;
    QAction *m_addUrlsAction;
    QAction *m_importUrlsAction;
    QAction *m_retrieveUrlsAction;
    QAction *m_quitAction;
    QMenu *m_transferMenu;
    QAction *m_transferPropertiesAction;
    QAction *m_transferConvertToAudioAction;
    QAction *m_transferStartAction;
    QAction *m_transferPauseAction;
    QMenu *m_transferConnectionsMenu;
    QActionGroup *m_transferConnectionsGroup;
    QMenu *m_transferCategoryMenu;
    QActionGroup *m_transferCategoryGroup;
    QMenu *m_transferPriorityMenu;
    QActionGroup *m_transferPriorityGroup;
    QAction *m_transferHighPriorityAction;
    QAction *m_transferNormalPriorityAction;
    QAction *m_transferLowPriorityAction;
    QAction *m_transferRemoveAction;
    QMenu *m_packageMenu;
    QAction *m_packagePropertiesAction;
    QAction *m_packageStartAction;
    QAction *m_packagePauseAction;
    QMenu *m_packageCategoryMenu;
    QActionGroup *m_packageCategoryGroup;
    QMenu *m_packagePriorityMenu;
    QActionGroup *m_packagePriorityGroup;
    QAction *m_packageHighPriorityAction;
    QAction *m_packageNormalPriorityAction;
    QAction *m_packageLowPriorityAction;
    QAction *m_packageRemoveAction;
    QMenu *m_editMenu;
    QAction *m_preferencesAction;
    QMenu *m_helpMenu;
    QAction *m_aboutAction;
    CheckUrlsDialog *m_checkDialog;
    QProgressDialog *m_progressDialog;
    bool m_progressDialogHasCancelButton;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_trayAddUrlsAction;
    QAction *m_trayImportUrlsAction;
    QAction *m_trayRetrieveUrlsAction;
    QAction *m_trayPreferencesAction;
    QAction *m_trayQuitAction;
};

#endif // MAINWINDOW_H
