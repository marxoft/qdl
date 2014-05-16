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

#include "selectionmodels.h"
#include "definitions.h"

SelectionModel::SelectionModel(QObject *parent) :
    QStandardItemModel(parent)
{
#if QT_VERSION >= 0x040600
    m_roleNames[Qt::DisplayRole] = "name";
    m_roleNames[Qt::UserRole + 1] = "value";
#if QT_VERSION < 0x050000
    this->setRoleNames(m_roleNames);
#endif
#endif
}

SelectionModel::~SelectionModel() {}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> SelectionModel::roleNames() const {
    return m_roleNames;
}
#endif

#if QT_VERSION >= 0x040600
QVariant SelectionModel::data(int row, const QByteArray &role) const {
    return QStandardItemModel::data(this->index(row, 0), this->roleNames().key(role));
}
#endif

QString SelectionModel::name(int row) const {
    return QStandardItemModel::data(this->index(row, 0), Qt::DisplayRole).toString();
}

QVariant SelectionModel::value(int row) const {
    return QStandardItemModel::data(this->index(row, 0), Qt::UserRole + 1);
}

void SelectionModel::addItem(const QString &name, const QVariant &value) {
    QStandardItem *item = new QStandardItem(name);
    item->setData(value);
    item->setTextAlignment(Qt::AlignCenter);
    item->setEditable(false);
    this->appendRow(item);
    emit countChanged(this->rowCount());
}

void SelectionModel::clear() {
    QStandardItemModel::clear();
    emit countChanged(this->rowCount());
}

ScreenOrientationModel::ScreenOrientationModel(QObject *parent) :
    SelectionModel(parent)
{
#ifdef SAILFISH_OS
    this->addItem(tr("Automatic"), ScreenOrientation::Automatic);
    this->addItem(tr("Portrait"), ScreenOrientation::LockPortrait);
    this->addItem(tr("Portrait (inverted)"), ScreenOrientation::LockPortraitInverted);
    this->addItem(tr("Landscape"), ScreenOrientation::LockLandscape);
    this->addItem(tr("Landscape (inverted)"), ScreenOrientation::LockLandcapeInverted);
#else
    this->addItem(tr("Automatic"), ScreenOrientation::Automatic);
    this->addItem(tr("Portrait"), ScreenOrientation::LockPortrait);
    this->addItem(tr("Landscape"), ScreenOrientation::LockLandscape);
#endif
}

TransferPriorityModel::TransferPriorityModel(QObject *parent) :
    SelectionModel(parent)
{
    this->addItem(tr("High"), Transfers::HighPriority);
    this->addItem(tr("Normal"), Transfers::NormalPriority);
    this->addItem(tr("Low"), Transfers::LowPriority);
}

LanguageModel::LanguageModel(QObject *parent) :
    SelectionModel(parent)
{
    this->addItem(tr("All languages"), "");
    this->addItem(tr("Afar"), "aa");
    this->addItem(tr("Abkhazian"), "ab");
    this->addItem(tr("Afrikaans"), "af");
    this->addItem(tr("Akan"), "af");
    this->addItem(tr("Albanian"), "sq");
    this->addItem(tr("Amharic"), "am");
    this->addItem(tr("Arabic"), "ar");
    this->addItem(tr("Aragonese"), "an");
    this->addItem(tr("Armenian"), "hy");
    this->addItem(tr("Assamese"), "as");
    this->addItem(tr("Avaric"), "av");
    this->addItem(tr("Avestan"), "ae");
    this->addItem(tr("Aymara"), "ay");
    this->addItem(tr("Azerbaijani"), "az");
    this->addItem(tr("Bashkir"), "ba");
    this->addItem(tr("Bambara"), "bm");
    this->addItem(tr("Basque"), "eu");
    this->addItem(tr("Belarusian"), "be");
    this->addItem(tr("Bengali"), "bn");
    this->addItem(tr("Bihari languages"), "bh");
    this->addItem(tr("Bislama"), "bi");
    this->addItem(tr("Bosnian"), "bs");
    this->addItem(tr("Breton"), "br");
    this->addItem(tr("Bulgarian"), "bg");
    this->addItem(tr("Burmese"), "my");
    this->addItem(tr("Catalan"), "ca");
    this->addItem(tr("Central Khmer"), "km");
    this->addItem(tr("Chamorro"), "ch");
    this->addItem(tr("Chechen"), "ce");
    this->addItem(tr("Chichewa"), "ny");
    this->addItem(tr("Chinese"), "zh");
    this->addItem(tr("Church Slavic"), "cu");
    this->addItem(tr("Chuvash"), "cv");
    this->addItem(tr("Cornish"), "kw");
    this->addItem(tr("Corsican"), "co");
    this->addItem(tr("Cree"), "cr");
    this->addItem(tr("Croatian"), "hr");
    this->addItem(tr("Czech"), "cs");
    this->addItem(tr("Danish"), "da");
    this->addItem(tr("Divehi"), "dv");
    this->addItem(tr("Dutch"), "nl");
    this->addItem(tr("Dzongkha"), "dz");
    this->addItem(tr("English"), "en");
    this->addItem(tr("Esperanto"), "eo");
    this->addItem(tr("Estonian"), "et");
    this->addItem(tr("Ewe"), "ee");
    this->addItem(tr("Faroese"), "fo");
    this->addItem(tr("Fijian"), "fj");
    this->addItem(tr("Finnish"), "fi");
    this->addItem(tr("French"), "fr");
    this->addItem(tr("Fulah"), "ff");
    this->addItem(tr("Gaelic"), "gd");
    this->addItem(tr("Galician"), "gl");
    this->addItem(tr("Ganda"), "lg");
    this->addItem(tr("Georgian"), "ka");
    this->addItem(tr("German"), "de");
    this->addItem(tr("Greek"), "el");
    this->addItem(tr("Guarani"), "gn");
    this->addItem(tr("Gujarati"), "gu");
    this->addItem(tr("Haitian"), "ht");
    this->addItem(tr("Hausa"), "ha");
    this->addItem(tr("Hebrew"), "he");
    this->addItem(tr("Herero"), "hz");
    this->addItem(tr("Hindi"), "hi");
    this->addItem(tr("Hiri Motu"), "ho");
    this->addItem(tr("Hungarian"), "hu");
    this->addItem(tr("Icelandic"), "is");
    this->addItem(tr("Ido"), "io");
    this->addItem(tr("Igbo"), "ig");
    this->addItem(tr("Indonesian"), "id");
    this->addItem(tr("Inuktitut"), "iu");
    this->addItem(tr("Interlingua"), "ia");
    this->addItem(tr("Interlingue"), "ie");
    this->addItem(tr("Inupiaq"), "ik");
    this->addItem(tr("Irish"), "ga");
    this->addItem(tr("Italian"), "it");
    this->addItem(tr("Japanese"), "ja");
    this->addItem(tr("Javanese"), "jv");
    this->addItem(tr("Kalaallisut"), "kl");
    this->addItem(tr("Kannada"), "kn");
    this->addItem(tr("Kanuri"), "kr");
    this->addItem(tr("Kashmiri"), "ks");
    this->addItem(tr("Kazakh"), "kk");
    this->addItem(tr("Kikuyu"), "ki");
    this->addItem(tr("Kinyarwanda"), "rw");
    this->addItem(tr("Kirghiz"), "ky");
    this->addItem(tr("Komi"), "kv");
    this->addItem(tr("Kongo"), "kg");
    this->addItem(tr("Korean"), "ko");
    this->addItem(tr("Kuanyama"), "kj");
    this->addItem(tr("Kurdish"), "ku");
    this->addItem(tr("Lao"), "lo");
    this->addItem(tr("Latin"), "la");
    this->addItem(tr("Latvian"), "lv");
    this->addItem(tr("Limburgan"), "li");
    this->addItem(tr("Lingala"), "ln");
    this->addItem(tr("Lithuanian"), "lt");
    this->addItem(tr("Luxembourgish"), "lb");
    this->addItem(tr("Luba-Katanga"), "lu");
    this->addItem(tr("Macedonian"), "mk");
    this->addItem(tr("Malagasy"), "mg");
    this->addItem(tr("Malay"), "ms");
    this->addItem(tr("Malayalam"), "ml");
    this->addItem(tr("Maltese"), "mt");
    this->addItem(tr("Manx"), "gv");
    this->addItem(tr("Maori"), "mi");
    this->addItem(tr("Marathi"), "mr");
    this->addItem(tr("Marshallese"), "mh");
    this->addItem(tr("Mongolian"), "mn");
    this->addItem(tr("Nauru"), "na");
    this->addItem(tr("Navajo"), "nv");
    this->addItem(tr("Ndebele, North"), "nd");
    this->addItem(tr("Ndebele, South"), "nr");
    this->addItem(tr("Ndonga"), "ng");
    this->addItem(tr("Nepali"), "ne");
    this->addItem(tr("Northern Sami"), "se");
    this->addItem(tr("Norwegian"), "no");
    this->addItem(tr("Norwegian Bokmål"), "nb");
    this->addItem(tr("Norwegian Nynorsk"), "nn");
    this->addItem(tr("Occitan"), "oc");
    this->addItem(tr("Ojibwa"), "oj");
    this->addItem(tr("Oriya"), "or");
    this->addItem(tr("Oromo"), "om");
    this->addItem(tr("Ossetian"), "os");
    this->addItem(tr("Pali"), "pi");
    this->addItem(tr("Persian"), "fa");
    this->addItem(tr("Polish"), "pl");
    this->addItem(tr("Portuguese"), "pt");
    this->addItem(tr("Punjabi"), "pa");
    this->addItem(tr("Pushto"), "ps");
    this->addItem(tr("Quechua"), "qu");
    this->addItem(tr("Romansh"), "rm");
    this->addItem(tr("Rundi"), "rn");
    this->addItem(tr("Russian"), "ru");
    this->addItem(tr("Samoan"), "sm");
    this->addItem(tr("Sango"), "sg");
    this->addItem(tr("Sanskrit"), "sa");
    this->addItem(tr("Sardinian"), "sc");
    this->addItem(tr("Serbian"), "sr");
    this->addItem(tr("Shona"), "sn");
    this->addItem(tr("Sindhi"), "sd");
    this->addItem(tr("Sinhala"), "si");
    this->addItem(tr("Sichuan Yi"), "ii");
    this->addItem(tr("Slovak"), "sk");
    this->addItem(tr("Slovenian"), "sl");
    this->addItem(tr("Somali"), "so");
    this->addItem(tr("Sotho, Southern"), "st");
    this->addItem(tr("Spanish"), "es");
    this->addItem(tr("Sundanese"), "su");
    this->addItem(tr("Swahili"), "sw");
    this->addItem(tr("Swati"), "ss");
    this->addItem(tr("Swedish"), "sv");
    this->addItem(tr("Tagalog"), "tl");
    this->addItem(tr("Tahitian"), "ty");
    this->addItem(tr("Tajik"), "tg");
    this->addItem(tr("Tamil"), "ta");
    this->addItem(tr("Tatar"), "tt");
    this->addItem(tr("Telugu"), "te");
    this->addItem(tr("Thai"), "th");
    this->addItem(tr("Tibetan"), "bo");
    this->addItem(tr("Tigrinya"), "ti");
    this->addItem(tr("Tonga"), "to");
    this->addItem(tr("Tsonga"), "ts");
    this->addItem(tr("Tswana"), "tn");
    this->addItem(tr("Turkish"), "tr");
    this->addItem(tr("Turkmen"), "tk");
    this->addItem(tr("Twi"), "tw");
    this->addItem(tr("Uighur"), "ug");
    this->addItem(tr("Ukrainian"), "uk");
    this->addItem(tr("Urdu"), "ur");
    this->addItem(tr("Uzbek"), "uz");
    this->addItem(tr("Venda"), "ve");
    this->addItem(tr("Vietnamese"), "vi");
    this->addItem(tr("Volapük"), "vo");
    this->addItem(tr("Walloon"), "wa");
    this->addItem(tr("Welsh"), "cy");
    this->addItem(tr("Western Frisian"), "fy");
    this->addItem(tr("Wolof"), "wo");
    this->addItem(tr("Xhosa"), "xh");
    this->addItem(tr("Yiddish"), "yi");
    this->addItem(tr("Yoruba"), "yo");
    this->addItem(tr("Zhuang"), "za");
    this->addItem(tr("Zulu"), "zu");
}

ConcurrentTransfersModel::ConcurrentTransfersModel(QObject *parent) :
    SelectionModel(parent)
{
    for (int i = 1; i <= MAX_CONCURRENT_TRANSFERS; i++) {
        this->addItem(QString::number(i), i);
    }
}

ConnectionsModel::ConnectionsModel(QObject *parent) :
    SelectionModel(parent)
{
    for (int i = 1; i <= MAX_CONNECTIONS; i++) {
        this->addItem(QString::number(i), i);
    }
}

DownloadRateLimitModel::DownloadRateLimitModel(QObject *parent) :
    SelectionModel(parent)
{
    this->addItem(tr("Unlimited"), 0);

    for (int i = 1; i < RATE_LIMITS.size(); i++) {
        this->addItem(QString::number(RATE_LIMITS.at(i) / 1000) + " kB/s", RATE_LIMITS.at(i));
    }
}

StatusFilterModel::StatusFilterModel(QObject *parent) :
    SelectionModel(parent)
{
    this->addItem(tr("All"), Transfers::Unknown);
    this->addItem(tr("Downloading"), Transfers::Downloading);
    this->addItem(tr("Queued"), Transfers::Queued);
    this->addItem(tr("Waiting"), Transfers::LongWait);
    this->addItem(tr("Paused"), Transfers::Paused);
    this->addItem(tr("Failed"), Transfers::Failed);
}

TransferActionModel::TransferActionModel(QObject *parent) :
    SelectionModel(parent)
{
    this->addItem(tr("Continue"), Transfers::Continue);
    this->addItem(tr("Pause"), Transfers::Pause);
    this->addItem(tr("Quit"), Transfers::Quit);
}

NetworkProxyTypeModel::NetworkProxyTypeModel(QObject *parent) :
    SelectionModel(parent)
{
    this->addItem(QString("HTTP"), NetworkProxyType::HttpProxy);
    this->addItem(QString("HTTP %1").arg(tr("caching")), NetworkProxyType::HttpCachingProxy);
    this->addItem(QString("SOCKS5"), NetworkProxyType::Socks5Proxy);
}
