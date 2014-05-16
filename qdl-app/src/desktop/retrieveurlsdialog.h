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

#ifndef RETRIEVEURLSDIALOG_H
#define RETRIEVEURLSDIALOG_H

#include <QDialog>

class QTextEdit;
class QDialogButtonBox;
class QVBoxLayout;

class RetrieveUrlsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RetrieveUrlsDialog(QWidget *parent = 0);
    ~RetrieveUrlsDialog();

    QString text() const;

public slots:
    void setText(const QString &text);
    void parseUrlsFromTextFile(const QString &fileName);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void accept();
    void onTextChanged();

signals:
    void urlsAvailable(const QString &urls);

private:
    QTextEdit *m_urlsEdit;
    QDialogButtonBox *m_buttonBox;
    QVBoxLayout *m_vbox;
};

#endif // RETRIEVEURLSDIALOG_H
