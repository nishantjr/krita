/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_brush.h"

#include <QDomElement>
#include <QFile>
#include <QPoint>
#include <QFileInfo>
#include <QBuffer>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <KoColor.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_boundary.h"
#include "kis_image.h"
#include "kis_iterator_ng.h"
#include "kis_brush_registry.h"
#include <brushengine/kis_paint_information.h>
#include <kis_fixed_paint_device.h>
#include <kis_qimage_pyramid.h>
#include <brushengine/kis_paintop_lod_limitations.h>


KisBrush::ColoringInformation::~ColoringInformation()
{
}

KisBrush::PlainColoringInformation::PlainColoringInformation(const quint8* color) : m_color(color)
{
}

KisBrush::PlainColoringInformation::~PlainColoringInformation()
{
}

const quint8* KisBrush::PlainColoringInformation::color() const
{
    return m_color;
}

void KisBrush::PlainColoringInformation::nextColumn()
{
}

void KisBrush::PlainColoringInformation::nextRow()
{
}

KisBrush::PaintDeviceColoringInformation::PaintDeviceColoringInformation(const KisPaintDeviceSP source, int width)
    : m_source(source)
    , m_iterator(m_source->createHLineConstIteratorNG(0, 0, width))
{
}

KisBrush::PaintDeviceColoringInformation::~PaintDeviceColoringInformation()
{
}

const quint8* KisBrush::PaintDeviceColoringInformation::color() const
{
    return m_iterator->oldRawData();
}

void KisBrush::PaintDeviceColoringInformation::nextColumn()
{
    m_iterator->nextPixel();
}
void KisBrush::PaintDeviceColoringInformation::nextRow()
{
    m_iterator->nextRow();
}


struct KisBrush::Private {
    Private()
        : boundary(0)
        , angle(0)
        , scale(1.0)
        , hasColor(false)
        , brushType(INVALID)
        , autoSpacingActive(false)
        , autoSpacingCoeff(1.0)
    {}

    ~Private() {
        delete boundary;
    }

    mutable KisBoundary* boundary;
    qreal angle;
    qreal scale;
    bool hasColor;
    enumBrushType brushType;

    qint32 width;
    qint32 height;
    double spacing;
    QPointF hotSpot;
    QImage brushTipImage;

    bool autoSpacingActive;
    qreal autoSpacingCoeff;
};

KisBrush::KisBrush()
    : KoResource("")
    , d(new Private)
{
}

KisBrush::KisBrush(const QString& filename)
    : KoResource(filename)
    , d(new Private)
{
}

KisBrush::KisBrush(const KisBrush& rhs)
    : KoResource("")
    , KisShared()
    , d(new Private)
{
    setBrushTipImage(rhs.brushTipImage());
    d->brushType = rhs.d->brushType;
    d->width = rhs.d->width;
    d->height = rhs.d->height;
    d->spacing = rhs.d->spacing;
    d->hotSpot = rhs.d->hotSpot;
    d->hasColor = rhs.d->hasColor;
    d->angle = rhs.d->angle;
    d->scale = rhs.d->scale;
    d->autoSpacingActive = rhs.d->autoSpacingActive;
    d->autoSpacingCoeff = rhs.d->autoSpacingCoeff;
    setFilename(rhs.filename());

    // don't copy the boundary, it will be regenerated -- see bug 291910
}

KisBrush::~KisBrush()
{
    delete d;
}

QImage KisBrush::brushTipImage() const
{
    if (d->brushTipImage.isNull()) {
        const_cast<KisBrush*>(this)->load();
    }
    return d->brushTipImage;
}

qint32 KisBrush::width() const
{
    return d->width;
}

void KisBrush::setWidth(qint32 width)
{
    d->width = width;
}

qint32 KisBrush::height() const
{
    return d->height;
}

void KisBrush::setHeight(qint32 height)
{
    d->height = height;
}

void KisBrush::setHotSpot(QPointF pt)
{
    double x = pt.x();
    double y = pt.y();

    if (x < 0)
        x = 0;
    else if (x >= width())
        x = width() - 1;

    if (y < 0)
        y = 0;
    else if (y >= height())
        y = height() - 1;

    d->hotSpot = QPointF(x, y);
}

QPointF KisBrush::hotSpot(double scaleX, double scaleY, double rotation, const KisPaintInformation& info) const
{
    Q_UNUSED(info);

    QSizeF metric = characteristicSize(scaleX, scaleY, rotation);

    qreal w = metric.width();
    qreal h = metric.height();

    // The smallest brush we can produce is a single pixel.
    if (w < 1) {
        w = 1;
    }

    if (h < 1) {
        h = 1;
    }

    // XXX: This should take d->hotSpot into account, though it
    // isn't specified by gimp brushes so it would default to the center
    // anyway.
    QPointF p(w / 2, h / 2);
    return p;
}


bool KisBrush::hasColor() const
{
    return d->hasColor;
}

void KisBrush::setHasColor(bool hasColor)
{
    d->hasColor = hasColor;
}

bool KisBrush::isPiercedApprox() const
{
    QImage image = brushTipImage();

    qreal w = image.width();
    qreal h = image.height();

    qreal xPortion = qMin(0.1, 5.0 / w);
    qreal yPortion = qMin(0.1, 5.0 / h);

    int x0 = std::floor((0.5 - xPortion) * w);
    int x1 = std::ceil((0.5 + xPortion) * w);

    int y0 = std::floor((0.5 - yPortion) * h);
    int y1 = std::ceil((0.5 + yPortion) * h);

    const int maxNumSamples = (x1 - x0 + 1) * (y1 - y0 + 1);
    const int failedPixelsThreshold = 0.1 * maxNumSamples;
    const int thresholdValue = 0.95 * 255;
    int failedPixels = 0;

    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            QRgb pixel = image.pixel(x,y);

            if (qRed(pixel) > thresholdValue) {
                failedPixels++;
            }
        }
    }

    return failedPixels > failedPixelsThreshold;
}

bool KisBrush::canPaintFor(const KisPaintInformation& /*info*/)
{
    return true;
}

void KisBrush::setBrushTipImage(const QImage& image)
{
    //Q_ASSERT(!image.isNull());
    d->brushTipImage = image.convertToFormat(QImage::Format_ARGB32);;

    if (!image.isNull()) {
        if (image.width() > 128 || image.height() > 128) {
            KoResource::setImage(image.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            KoResource::setImage(image);
        }
        setWidth(image.width());
        setHeight(image.height());
    }
}

void KisBrush::setBrushType(enumBrushType type)
{
    d->brushType = type;
}

enumBrushType KisBrush::brushType() const
{
    return d->brushType;
}

void KisBrush::predefinedBrushToXML(const QString &type, QDomElement& e) const
{
    e.setAttribute("type", type);
    e.setAttribute("filename", shortFilename());
    e.setAttribute("spacing", QString::number(spacing()));
    e.setAttribute("useAutoSpacing", QString::number(autoSpacingActive()));
    e.setAttribute("autoSpacingCoeff", QString::number(autoSpacingCoeff()));
    e.setAttribute("angle", QString::number(angle()));
    e.setAttribute("scale", QString::number(scale()));
}

void KisBrush::toXML(QDomDocument& /*document*/ , QDomElement& element) const
{
    element.setAttribute("BrushVersion", "2");
}

KisBrushSP KisBrush::fromXML(const QDomElement& element, bool forceCopy)
{
    KisBrushSP brush = KisBrushRegistry::instance()->getOrCreateBrush(element, forceCopy);
    if (brush && element.attribute("BrushVersion", "1") == "1") {
        brush->setScale(brush->scale() * 2.0);
    }
    return brush;
}

QSizeF KisBrush::characteristicSize(double scaleX, double scaleY, double rotation) const
{
    Q_UNUSED(scaleY);

    qreal angle = normalizeAngle(rotation + d->angle);
    qreal scale = scaleX * d->scale;

    return KisQImagePyramid::characteristicSize(QSize(width(), height()),
                                                scale, angle);
}

qint32 KisBrush::maskWidth(double scale, double angle, qreal subPixelX, qreal subPixelY, const KisPaintInformation& info) const
{
    Q_UNUSED(info);

    angle = normalizeAngle(angle + d->angle);
    scale *= d->scale;

    return KisQImagePyramid::imageSize(QSize(width(), height()),
                                       scale, angle,
                                       subPixelX, subPixelY).width();
}

qint32 KisBrush::maskHeight(double scale, double angle, qreal subPixelX, qreal subPixelY, const KisPaintInformation& info) const
{
    Q_UNUSED(info);

    angle = normalizeAngle(angle + d->angle);
    scale *= d->scale;

    return KisQImagePyramid::imageSize(QSize(width(), height()),
                                       scale, angle,
                                       subPixelX, subPixelY).height();
}

double KisBrush::maskAngle(double angle) const
{
    return normalizeAngle(angle + d->angle);
}

quint32 KisBrush::brushIndex(const KisPaintInformation& info) const
{
    Q_UNUSED(info);
    return 0;
}

void KisBrush::setSpacing(double s)
{
    if (s < 0.02) s = 0.02;
    d->spacing = s;
}

double KisBrush::spacing() const
{
    return d->spacing;
}

void KisBrush::setAutoSpacing(bool active, qreal coeff)
{
    d->autoSpacingCoeff = coeff;
    d->autoSpacingActive = active;
}

bool KisBrush::autoSpacingActive() const
{
    return d->autoSpacingActive;
}

qreal KisBrush::autoSpacingCoeff() const
{
    return d->autoSpacingCoeff;
}

void KisBrush::notifyStrokeStarted()
{
}

void KisBrush::notifyCachedDabPainted(const KisPaintInformation& info)
{
    Q_UNUSED(info);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, double scaleX, double scaleY, double angle, const KisPaintInformation& info , double subPixelX, double subPixelY, qreal softnessFactor) const
{
    generateMaskAndApplyMaskOrCreateDab(dst, 0, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KoColor& color, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    PlainColoringInformation pci(color.data());
    generateMaskAndApplyMaskOrCreateDab(dst, &pci, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KisPaintDeviceSP src, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    PaintDeviceColoringInformation pdci(src, maskWidth(scaleX, angle, subPixelX, subPixelY, info));
    generateMaskAndApplyMaskOrCreateDab(dst, &pdci, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

QImage KisBrush::transformBrushTip(const QImage& image,
    qreal scaleX, qreal scaleY, qreal rotate, 
    qreal subPixelX, qreal subPixelY)
{
    QImage ret(
        image.width()  + (subPixelX == 0.0 ? 0 : 1),
        image.height() + (subPixelY == 0.0 ? 0 : 1),
        QImage::Format_ARGB32);
    ret.fill(0);

    QTransform transform;
    transform.scale(scaleX, scaleY)
             .rotate(rotate)
             .translate(subPixelX, subPixelY);

    /**
     * QPainter has one more bug: when a QTransform is TxTranslate, it
     * does wrong sampling (probably, Nearest Neighbour) even though
     * we tell it directly that we need SmoothPixmapTransform.
     *
     * So here is a workaround: we set a negligible scale to convince
     * Qt we use a non-only-translating transform.
     */
    while (transform.type() == QTransform::TxTranslate) {
        const qreal scale = transform.m11();
        const qreal fakeScale = scale - 10 * std::numeric_limits<qreal>::epsilon();
        transform *= QTransform::fromScale(fakeScale, fakeScale);
    }

    QPainter painter(&ret);
    painter.setTransform(transform);
    painter.setRenderHints(QPainter::SmoothPixmapTransform
                         | QPainter::Antialiasing);
    painter.drawImage(QPointF(), image);
    painter.end();

    return std::move(ret);
}

void KisBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
        ColoringInformation* coloringInformation,
        double scaleX, double scaleY, double angle,
        const KisPaintInformation& info_,
        double subPixelX, double subPixelY, qreal softnessFactor) const
{
    Q_ASSERT(valid());
    Q_UNUSED(info_);
    Q_UNUSED(softnessFactor);

    angle   = normalizeAngle(angle + d->angle);
    scaleX *= d->scale;
    scaleY *= d->scale;

    QImage outputImage = transformBrushTip(brushTipImage(),
        scaleX, scaleY, angle, subPixelX, subPixelY);

    qint32 maskWidth = outputImage.width();
    qint32 maskHeight = outputImage.height();

    dst->setRect(QRect(0, 0, maskWidth, maskHeight));
    dst->initialize();

    quint8* color = 0;

    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    }

    const KoColorSpace *cs = dst->colorSpace();
    qint32 pixelSize = cs->pixelSize();
    quint8 *dabPointer = dst->data();
    quint8 *rowPointer = dabPointer;
    quint8 *alphaArray = new quint8[maskWidth];
    bool hasColor = this->hasColor();

    for (int y = 0; y < maskHeight; y++) {
        const quint8* maskPointer = outputImage.constScanLine(y);
        if (coloringInformation) {
            for (int x = 0; x < maskWidth; x++) {
                if (color) {
                    memcpy(dabPointer, color, pixelSize);
                }
                else {
                    memcpy(dabPointer, coloringInformation->color(), pixelSize);
                    coloringInformation->nextColumn();
                }
                dabPointer += pixelSize;
            }
        }

        const quint8 *src = maskPointer;
        quint8 *dst = alphaArray;
        for (int x = 0; x < maskWidth; x++) {
            const QRgb *c = reinterpret_cast<const QRgb*>(src);
            if (hasColor) {
                *dst = KoColorSpaceMaths<quint8>::multiply(255 - qGray(*c), qAlpha(*c));
            }
            else {
                *dst = KoColorSpaceMaths<quint8>::multiply(255 - *src, qAlpha(*c));
            }
            src += 4;
            dst++;
        }

        cs->applyAlphaU8Mask(rowPointer, alphaArray, maskWidth);
        rowPointer += maskWidth * pixelSize;
        dabPointer = rowPointer;

        if (!color && coloringInformation) {
            coloringInformation->nextRow();
        }
    }

    delete[] alphaArray;
}

KisFixedPaintDeviceSP KisBrush::paintDevice(const KoColorSpace * colorSpace,
        double scale, double ratio, double angle,
        const KisPaintInformation& info,
        double subPixelX, double subPixelY) const
{
    Q_ASSERT(valid());
    Q_UNUSED(info);
    angle  = normalizeAngle(angle + d->angle);
    scale *= d->scale;

    QImage outputImage = transformBrushTip(brushTipImage(),
        scale, scale * ratio, angle, subPixelX, subPixelY);

    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(colorSpace);
    Q_CHECK_PTR(dab);
    dab->convertFromQImage(outputImage, "");

    return dab;
}

void KisBrush::resetBoundary()
{
    delete d->boundary;
    d->boundary = 0;
}

void KisBrush::generateBoundary() const
{
    KisFixedPaintDeviceSP dev;

    if (brushType() == IMAGE || brushType() == PIPE_IMAGE) {
        dev = paintDevice(KoColorSpaceRegistry::instance()->rgb8(),
            1.0 / scale(), 1.0 / scale(), -angle(), KisPaintInformation());
    }
    else {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        dev = new KisFixedPaintDevice(cs);
        mask(dev, KoColor(Qt::black, cs) , 1.0 / scale(), 1.0 / scale(), -angle(), KisPaintInformation());
    }

    d->boundary = new KisBoundary(dev);
    d->boundary->generateBoundary();
}

const KisBoundary* KisBrush::boundary() const
{
    if (!d->boundary)
        generateBoundary();
    return d->boundary;
}

void KisBrush::setScale(qreal _scale)
{
    d->scale = _scale;
}

qreal KisBrush::scale() const
{
    return d->scale;
}

void KisBrush::setAngle(qreal _rotation)
{
    d->angle = _rotation;
}

qreal KisBrush::angle() const
{
    return d->angle;
}

QPainterPath KisBrush::outline() const
{
    return boundary()->path();
}

void KisBrush::lodLimitations(KisPaintopLodLimitations *l) const
{
    if (spacing() > 0.5) {
        l->limitations << KoID("huge-spacing", i18nc("PaintOp instant preview limitation", "Spacing > 0.5, consider disabling Instant Preview"));
    }
}
