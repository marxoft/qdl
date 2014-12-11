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

#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include <QObject>
#include <QProcess>

class ArchiveExtractor : public QObject
{
    Q_OBJECT

public:
    explicit ArchiveExtractor(QObject *parent = 0);
    ~ArchiveExtractor();

    QString fileName() const;
    void setFileName(const QString &fileName);

    QString outputDirectory() const;
    void setOutputDirectory(const QString &directory);

    QString password() const;
    void setPassword(const QString &password);

    bool createSubfolder() const;
    void setCreateSubfolder(bool subfolder);

    QString errorString() const;

public slots:
    void start();
    void start(const QString &fileName, const QString &outputDirectory, const QString &password = QString(),
               bool createSubfolder = true);

private:
    void setErrorString(const QString &errorString);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

signals:
    void finished();
    void error();
    
private:
    QProcess *m_process;
    QString m_fileName;
    QString m_outputDirectory;
    QString m_password;
    QString m_errorString;
    bool m_createSubfolder;
};

#endif // ARCHIVEEXTRACTOR_H
