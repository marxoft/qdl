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

#include "storage.h"
#include "connection.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <QFile>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif
#include <QDebug>

#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN)
static const QString DIRECTORY("/home/user/.QDL/");
#elif (defined Q_OS_SYMBIAN)
static const QString DIRECTORY("C:/.config/QDL/");
#elif QT_VERSION >= 0x050000
static const QString DIRECTORY(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.QDL/");
#else
static const QString DIRECTORY(QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.QDL/");
#endif

Storage::Storage(QObject *parent) :
    QObject(parent)
{
}

Storage::~Storage() {}

bool Storage::storeTransfers(QList<Transfer*> transfers, bool deleteWhenStored) {
    if (transfers.isEmpty()) {
        return false;
    }

    QDir dir;

    if (!dir.mkpath(DIRECTORY)) {
        qDebug() << "Storage::storeTransfers(): Cannot create directory" << DIRECTORY;
        return false;
    }

    QFile file(DIRECTORY + "downloads.xml");

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Storage::storeTransfers():" << file.errorString();
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("packages");

    foreach (Transfer* transfer, transfers) {
        writer.writeStartElement("package");
        writer.writeAttribute("id", transfer->packageId());
        writer.writeAttribute("name", transfer->packageName());
        writer.writeAttribute("suffix", transfer->packageSuffix());

        writer.writeStartElement("transfer");
        writer.writeTextElement("id", transfer->id());
        writer.writeTextElement("url", transfer->url().toString());
        writer.writeTextElement("serviceName", transfer->serviceName());
        writer.writeTextElement("downloadPath", transfer->downloadPath());
        writer.writeTextElement("fileName", transfer->fileName());
        writer.writeTextElement("category", transfer->category());
        writer.writeTextElement("priority", QString::number(Transfers::Priority(transfer->priority())));
        writer.writeTextElement("size", QString::number(transfer->size()));
        writer.writeTextElement("convertToAudio", transfer->convertToAudio() ? "1" : "0");
        writer.writeTextElement("position", QString::number(transfer->position()));
        writer.writeTextElement("preferredConnections", QString::number(transfer->preferredConnections()));

        foreach (Connection* connection, transfer->connections()) {
            writer.writeStartElement("connection");
            writer.writeAttribute("position", QString::number(connection->position()));
            writer.writeAttribute("end", QString::number(connection->contentRangeEnd()));
            writer.writeEndElement();
        }

        writer.writeEndElement();

        foreach (Transfer *childTransfer, transfer->childTransfers()) {
            writer.writeStartElement("transfer");
            writer.writeTextElement("id", childTransfer->id());
            writer.writeTextElement("url", childTransfer->url().toString());
            writer.writeTextElement("serviceName", childTransfer->serviceName());
            writer.writeTextElement("downloadPath", childTransfer->downloadPath());
            writer.writeTextElement("fileName", childTransfer->fileName());
            writer.writeTextElement("category", childTransfer->category());
            writer.writeTextElement("priority", QString::number(Transfers::Priority(childTransfer->priority())));
            writer.writeTextElement("size", QString::number(childTransfer->size()));
            writer.writeTextElement("convertToAudio", childTransfer->convertToAudio() ? "1" : "0");
            writer.writeTextElement("position", QString::number(childTransfer->position()));
            writer.writeTextElement("preferredConnections", QString::number(childTransfer->preferredConnections()));

            foreach (Connection* connection, childTransfer->connections()) {
                writer.writeStartElement("connection");
                writer.writeAttribute("position", QString::number(connection->position()));
                writer.writeAttribute("end", QString::number(connection->contentRangeEnd()));
                writer.writeEndElement();
            }

            if (deleteWhenStored) {
                childTransfer->deleteLater();
            }

            writer.writeEndElement();
        }

        if (deleteWhenStored) {
            transfer->deleteLater();
        }

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return true;
}

QList<Transfer*> Storage::restoreTransfers() {
    QList<Transfer*> transfers;

    if (!QFile::exists(DIRECTORY + "downloads.xml")) {
        return transfers;
    }

    QFile file(DIRECTORY + "downloads.xml");

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Storage::restoreTransfers():" << file.errorString();
        return transfers;
    }

    QXmlStreamReader reader(&file);

    while ((!reader.atEnd()) && (!reader.hasError())) {
        while ((!reader.atEnd()) && (!reader.hasError()) && (reader.name() != "package")) {
            while (reader.readNext() != QXmlStreamReader::Invalid) {
                if ((reader.isStartElement()) || (reader.isEndElement())) {
                    break;
                }
            }
        }

        if (reader.name() == "package") {
            QString packageId = reader.attributes().value("id").toString();
            QString packageName = reader.attributes().value("name").toString();
            QString packageSuffix = reader.attributes().value("suffix").toString();

            QList<Transfer*> packageTransfers;

            while (reader.readNext() != QXmlStreamReader::Invalid) {
                if ((reader.isStartElement()) || (reader.isEndElement())) {
                    break;
                }
            }

            if (reader.name() == "transfer") {
                while ((!reader.atEnd()) && (!reader.hasError()) && (reader.name() != "package")) {
                    Transfer *transfer = new Transfer;

                    while (reader.readNext() != QXmlStreamReader::Invalid) {
                        if ((reader.isStartElement()) || (reader.isEndElement())) {
                            break;
                        }
                    }

                    while ((!reader.atEnd()) && (!reader.hasError()) && (reader.name() != "transfer")) {
                        if (reader.name() == "id") {
                            transfer->setId(reader.readElementText());
                        }
                        else if (reader.name() == "url") {
                            transfer->setUrl(QUrl(reader.readElementText()));
                        }
                        else if (reader.name() == "serviceName") {
                            transfer->setServiceName(reader.readElementText());
                        }
                        else if (reader.name() == "downloadPath") {
                            transfer->setDownloadPath(reader.readElementText());
                        }
                        else if (reader.name() == "fileName") {
                            transfer->setFileName(reader.readElementText());
                        }
                        else if (reader.name() == "category") {
                            transfer->setCategory(reader.readElementText());
                        }
                        else if (reader.name() == "priority") {
                            transfer->setPriority(static_cast<Transfers::Priority>(reader.readElementText().toInt()));
                        }
                        else if (reader.name() == "size") {
                            transfer->setSize(reader.readElementText().toLongLong());
                        }
                        else if (reader.name() == "convertToAudio") {
                            transfer->setConvertToAudio(reader.readElementText().toInt());
                        }
                        else if (reader.name() == "position") {
                            transfer->setResumePosition(reader.readElementText().toLongLong());
                        }
                        else if (reader.name() == "preferredConnections") {
                            transfer->setPreferredConnections(reader.readElementText().toInt());
                        }
                        else if (reader.name() == "connection") {
                            transfer->restoreConnection(reader.attributes().value("position").toString().toLongLong(),
                                                        reader.attributes().value("end").toString().toLongLong());

                            while (reader.readNext() != QXmlStreamReader::Invalid) {
                                if ((reader.isStartElement()) || (reader.isEndElement())) {
                                    break;
                                }
                            }
                        }

                        while (reader.readNext() != QXmlStreamReader::Invalid) {
                            if ((reader.isStartElement()) || (reader.isEndElement())) {
                                break;
                            }
                        }
                    }

                    while (reader.readNext() != QXmlStreamReader::Invalid) {
                        if ((reader.isStartElement()) || (reader.isEndElement())) {
                            break;
                        }
                    }

                    packageTransfers.append(transfer);
                }
            }

            if (!packageTransfers.isEmpty()) {
                Transfer *package = packageTransfers.takeFirst();
                package->setPackageId(packageId);
                package->setPackageName(packageName);
                package->setPackageSuffix(packageSuffix);

                while (!packageTransfers.isEmpty()) {
                    package->addChildTransfer(packageTransfers.takeFirst());
                }

                transfers.append(package);
            }
        }
    }

    return transfers;
}

bool Storage::clearStoredTransfers() {
    return QFile::remove(DIRECTORY + "downloads.xml");
}
