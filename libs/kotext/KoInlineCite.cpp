
/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoInlineCite.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoTextLoader.h>
#include <KoTextWriter.h>
#include <KoTextDocument.h>
#include <KoText.h>

#include <KDebug>

#include <QTextDocument>
#include <QTextFrame>
#include <QTextCursor>
#include <QString>
#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QTextOption>
#include <QDateTime>
#include <QWeakPointer>
#include <QMessageBox>

class KoInlineCite::Private
{
public:
    Private()
        : textFrame(0)
    {
    }
    QTextFrame *textFrame;
    QString bibliographyType;
    QString identifier;
    QString address;
    QString annote;
    QString author;
    QString booktitle;
    QString chapter;
    QString edition;
    QString editor;
    QString publicationType;
    QString institution;
    QString journal;
    QString month;
    QString note;
    QString number;
    QString organisation;
    QString pages;
    QString publisher;
    QString school;             //university in UI file  // saved under text:school XML format
    QString series;
    QString title;
    QString reportType;
    QString volume;
    QString year;
    QString url;
    QString isbn;
    QString custom1;
    QString custom2;
    QString custom3;
    QString custom4;
    QString custom5;
};

KoInlineCite::KoInlineCite()
    :KoInlineObject(true)
    ,d(new Private())
{
}

KoInlineCite::~KoInlineCite()
{
    delete d;
}

void KoInlineCite::setMotherFrame(QTextFrame *motherFrame)
{
    QTextCursor cursor(motherFrame->lastCursorPosition());
    QTextFrameFormat format;
    format.setProperty(KoText::SubFrameType, KoText::NoteFrameType);

    d->textFrame = cursor.insertFrame(format);
}

QTextFrame *KoInlineCite::textFrame() const
{
    return d->textFrame;
}

void KoInlineCite::setIdentifier(const QString &identifier)
{
    d->identifier = identifier;
}

QString KoInlineCite::identifier() const
{
    return d->identifier;
}

void KoInlineCite::setAddress(const QString &addr)
{
    d->address = addr;
}

void KoInlineCite::setAnnotation(const QString &annotation)
{
    d->annote = annotation;
}

void KoInlineCite::setAuthor(const QString &author)
{
    d->author = author;
}

void KoInlineCite::setBibliographyType(const QString &bibliographyType)
{
    d->bibliographyType = bibliographyType;
}

void KoInlineCite::setBookTitle(const QString &booktitle)
{
    d->booktitle = booktitle;
}

void KoInlineCite::setChapter(const QString &chapter)
{
    d->chapter = chapter;
}

void KoInlineCite::setCustom1(const QString &custom1)
{
    d->custom1 = custom1;
}

void KoInlineCite::setCustom2(const QString &custom2)
{
    d->custom2 = custom2;
}

void KoInlineCite::setCustom3(const QString &custom3)
{
    d->custom3 = custom3;
}

void KoInlineCite::setCustom4(const QString &custom4)
{
    d->custom4 = custom4;
}

void KoInlineCite::setCustom5(const QString &custom5)
{
    d->custom5 = custom5;
}

void KoInlineCite::setEdition(const QString &edition)
{
    d->edition = edition;
}

void KoInlineCite::setEditor(const QString &editor)
{
    d->editor = editor;
}

void KoInlineCite::setInstitution(const QString &institution)
{
    d->institution = institution;
}

void KoInlineCite::setISBN(const QString &isbn)
{
    d->isbn = isbn;
}

void KoInlineCite::setJournal(const QString &journal)
{
    d->journal = journal;
}

void KoInlineCite::setMonth(const QString &month)
{
    d->month = month;
}

void KoInlineCite::setNote(const QString &note)
{
    d->note = note;
}

void KoInlineCite::setNumber(const QString &number)
{
    d->number = number;
}

void KoInlineCite::setOrganisation(const QString &organisation)
{
    d->organisation = organisation;
}

void KoInlineCite::setPages(const QString &pages)
{
    d->pages = pages;
}

void KoInlineCite::setPublicationType(const QString &publicationType)
{
    d->publicationType = publicationType;
}

void KoInlineCite::setPublisher(const QString &publisher)
{
    d->publisher = publisher;
}

void KoInlineCite::setReportType(const QString &reportType)
{
    d->reportType = reportType;
}

void KoInlineCite::setSchool(const QString &school)
{
    d->school = school;
}

void KoInlineCite::setSeries(const QString &series)
{
    d->series = series;
}

void KoInlineCite::setTitle(const QString &title)
{
    d->title = title;
}

void KoInlineCite::setURL(const QString &url)
{
    d->url = url;
}

void KoInlineCite::setVolume(const QString &volume)
{
    d->volume = volume;
}

void KoInlineCite::setYear(const QString &year)
{
    d->year = year;
}

void KoInlineCite::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
}

void KoInlineCite::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    if (d->identifier.isEmpty())
        return;
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth(fm.width(d->identifier));
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
}

void KoInlineCite::paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);

    if (d->identifier.isEmpty())
        return;

    QFont font(format.font(), pd);
    QTextLayout layout(d->identifier, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = d->identifier.length();
    range.format = format;
    range.format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
}

bool KoInlineCite::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KoTextLoader loader(context);
    QTextCursor cursor(d->textFrame->lastCursorPosition());
    //QMessageBox::critical(0,QString("loading"),QString("loading citation"),QMessageBox::Ok);
    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "bibliography-mark") {

        d->identifier = element.attributeNS(KoXmlNS::text, "identifier");
        d->bibliographyType = element.attributeNS(KoXmlNS::text, "bibliography-type");
        d->address = element.attributeNS(KoXmlNS::text, "address");
        d->annote = element.attributeNS(KoXmlNS::text, "annote");
        d->author = element.attributeNS(KoXmlNS::text, "author");
        d->booktitle = element.attributeNS(KoXmlNS::text, "booktitle");
        d->chapter = element.attributeNS(KoXmlNS::text, "chapter");
        d->edition = element.attributeNS(KoXmlNS::text, "edition");
        d->editor = element.attributeNS(KoXmlNS::text, "editor");
        d->publicationType = element.attributeNS(KoXmlNS::text, "howpublished");
        d->institution = element.attributeNS(KoXmlNS::text, "institution");
        d->journal = element.attributeNS(KoXmlNS::text, "journal");
        d->month = element.attributeNS(KoXmlNS::text, "month");
        d->note = element.attributeNS(KoXmlNS::text, "note");
        d->number = element.attributeNS(KoXmlNS::text, "number");
        d->organisation = element.attributeNS(KoXmlNS::text, "organisations");
        d->pages = element.attributeNS(KoXmlNS::text, "pages");
        d->publisher = element.attributeNS(KoXmlNS::text, "publisher");
        d->school = element.attributeNS(KoXmlNS::text, "school");
        d->series = element.attributeNS(KoXmlNS::text, "series");
        d->title = element.attributeNS(KoXmlNS::text, "title");
        d->reportType = element.attributeNS(KoXmlNS::text, "report-type");
        d->volume = element.attributeNS(KoXmlNS::text, "volume");
        d->year = element.attributeNS(KoXmlNS::text, "year");
        d->url = element.attributeNS(KoXmlNS::text, "url");
        d->isbn = element.attributeNS(KoXmlNS::text, "isbn");
        d->custom1 = element.attributeNS(KoXmlNS::text, "custom1");
        d->custom2 = element.attributeNS(KoXmlNS::text, "custom2");
        d->custom3 = element.attributeNS(KoXmlNS::text, "custom3");
        d->custom4 = element.attributeNS(KoXmlNS::text, "custom4");
        d->custom5 = element.attributeNS(KoXmlNS::text, "custom5");
    }
    else {
        return false;
    }
    //QMessageBox::critical(0,QString("loading completed"),QString("identifier is %1").arg(d->identifier),QMessageBox::Ok);
    return true;
}

void KoInlineCite::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();

    QTextCursor cursor(d->textFrame);

    writer->startElement("text:bibliography-mark", false);

    if (d->identifier != "")
        writer->addAttribute("text:identifier", d->identifier);     //can't be "" //to be changed later
    if (d->bibliographyType != "")
        writer->addAttribute("text:bibliography-type", d->bibliographyType);
    if (d->address != "")
        writer->addAttribute("text:address",d->identifier);
    if (d->annote != "")
        writer->addAttribute("text:annote",d->annote);
    if (d->author != "")
        writer->addAttribute("text:author",d->author);
    if (d->booktitle != "")
        writer->addAttribute("text:booktitle",d->booktitle);
    if (d->chapter != "")
        writer->addAttribute("text:chapter",d->chapter);
    if (d->edition != "")
        writer->addAttribute("text:edition",d->edition);
    if (d->editor != "")
        writer->addAttribute("text:editor",d->editor);
    if (d->publicationType != "")
        writer->addAttribute("text:howpublished",d->publicationType);
    if (d->institution != "")
        writer->addAttribute("text:institution",d->institution);
    if (d->journal != "")
        writer->addAttribute("text:journal",d->journal);
    if (d->month != "")
        writer->addAttribute("text:month",d->month);
    if (d->note != "")
        writer->addAttribute("text:note",d->note);
    if (d->number != "")
        writer->addAttribute("text:number",d->number);
    if (d->pages != "")
        writer->addAttribute("text:pages",d->pages);
    if (d->publisher != "")
        writer->addAttribute("text:publisher",d->publisher);
    if (d->school != "")
        writer->addAttribute("text:school",d->school);
    if (d->series != "")
        writer->addAttribute("text:series",d->series);
    if (d->title != "")
        writer->addAttribute("text:title",d->title);
    if (d->reportType != "")
        writer->addAttribute("text:report-type",d->reportType);
    if (d->volume != "")
        writer->addAttribute("text:volume",d->volume);
    if (d->year != "")
        writer->addAttribute("text:year",d->year);
    if (d->url != "")
        writer->addAttribute("text:url",d->url);
    if (d->isbn != "")
        writer->addAttribute("text:isbn",d->isbn);
    if (d->custom1 != "")
        writer->addAttribute("text:custom1",d->custom1);
    if (d->custom2 != "")
        writer->addAttribute("text:custom2",d->custom2);
    if (d->custom3 != "")
        writer->addAttribute("text:custom3",d->custom3);
    if (d->custom4 != "")
        writer->addAttribute("text:custom4",d->custom4);
    if (d->custom5 != "")
        writer->addAttribute("text:custom5",d->custom5);

    writer->addTextNode(QString("[%1]").arg(d->identifier));
    writer->endElement();
}