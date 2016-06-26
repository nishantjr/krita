/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_dab_cache.h"

#include <KoColor.h>
#include "kis_color_source.h"
#include "kis_paint_device.h"
#include "kis_brush.h"
#include <kis_pressure_mirror_option.h>
#include <kis_pressure_sharpness_option.h>
#include <kis_texture_option.h>
#include <kis_precision_option.h>
#include <kis_fixed_paint_device.h>
#include <brushengine/kis_paintop.h>

#include <kundo2command.h>

struct PrecisionValues {
    qreal angle;
    qreal sizeFrac;
    qreal subPixel;
    qreal softnessFactor;
};

const qreal eps = 1e-6;
static const PrecisionValues precisionLevels[] = {
    {M_PI / 180, 0.05,   1, 0.01},
    {M_PI / 180, 0.01,   1, 0.01},
    {M_PI / 180,    0,   1, 0.01},
    {M_PI / 180,    0, 0.5, 0.01},
    {eps,         0, eps,  eps}
};

struct KisDabCache::Private {

    Private(KisBrushSP brush)
        : brush(brush),
          mirrorOption(0),
          sharpnessOption(0),
          textureOption(0),
          precisionOption(0),
          subPixelPrecisionDisabled(false)
    {}

    KisBrushSP brush;
    KisPaintDeviceSP colorSourceDevice;

    KisPressureMirrorOption *mirrorOption;
    KisPressureSharpnessOption *sharpnessOption;
    KisTextureProperties *textureOption;
    KisPrecisionOption *precisionOption;
    bool subPixelPrecisionDisabled;
};



KisDabCache::KisDabCache(KisBrushSP brush)
    : m_d(new Private(brush))
{
}

KisDabCache::~KisDabCache()
{
    delete m_d;
}

void KisDabCache::setMirrorPostprocessing(KisPressureMirrorOption *option)
{
    m_d->mirrorOption = option;
}

void KisDabCache::setSharpnessPostprocessing(KisPressureSharpnessOption *option)
{
    m_d->sharpnessOption = option;
}

void KisDabCache::setTexturePostprocessing(KisTextureProperties *option)
{
    m_d->textureOption = option;
}

void KisDabCache::setPrecisionOption(KisPrecisionOption *option)
{
    m_d->precisionOption = option;
}

void KisDabCache::disableSubpixelPrecision()
{
    m_d->subPixelPrecisionDisabled = true;
}

KisFixedPaintDeviceSP KisDabCache::fetchDab(const KoColorSpace *cs,
        const KisColorSource *colorSource,
        const QPointF &cursorPoint,
        double scaleX, double scaleY,
        double angle,
        const KisPaintInformation& info,
        qreal softnessFactor,
        QRect *dstDabRect)
{
    return fetchDabCommon(cs, colorSource, KoColor(),
                          cursorPoint,
                          scaleX, scaleY, angle,
                          info,
                          softnessFactor,
                          dstDabRect);
}

KisFixedPaintDeviceSP KisDabCache::fetchDab(const KoColorSpace *cs,
        const KoColor& color,
        const QPointF &cursorPoint,
        double scaleX, double scaleY,
        double angle,
        const KisPaintInformation& info,
        qreal softnessFactor,
        QRect *dstDabRect)
{
    return fetchDabCommon(cs, 0, color,
                          cursorPoint,
                          scaleX, scaleY, angle,
                          info,
                          softnessFactor,
                          dstDabRect);
}

struct KisDabCache::DabPosition {
    DabPosition(const QRect &_rect,
                const QPointF &_subPixel,
                qreal _realAngle)
        : rect(_rect),
          subPixel(_subPixel),
          realAngle(_realAngle) {
    }

    QRect rect;
    QPointF subPixel;
    qreal realAngle;
};

qreal positiveFraction(qreal x) {
    qint32 unused = 0;
    qreal fraction = 0.0;
    KisPaintOp::splitCoordinate(x, &unused, &fraction);

    return fraction;
}

inline
KisDabCache::DabPosition
KisDabCache::calculateDabRect(const QPointF &cursorPoint,
                              double scaleX, double scaleY,
                              double angle,
                              const KisPaintInformation& info,
                              const MirrorProperties &mirrorProperties)
{
    qint32 x = 0, y = 0;
    qreal subPixelX = 0.0, subPixelY = 0.0;

    if (mirrorProperties.coordinateSystemFlipped) {
        angle = 2 * M_PI - angle;
    }

    QPointF hotSpot = m_d->brush->hotSpot(scaleX, scaleY, angle, info);
    QPointF pt = cursorPoint - hotSpot;

    if (m_d->sharpnessOption) {
        m_d->sharpnessOption->apply(info, pt, x, y, subPixelX, subPixelY);
    }
    else {
        KisPaintOp::splitCoordinate(pt.x(), &x, &subPixelX);
        KisPaintOp::splitCoordinate(pt.y(), &y, &subPixelY);
    }

    if (m_d->subPixelPrecisionDisabled) {
        subPixelX = 0;
        subPixelY = 0;
    }

    if (qIsNaN(subPixelX)) {
        subPixelX = 0;
    }

    if (qIsNaN(subPixelY)) {
        subPixelY = 0;
    }

    int width = m_d->brush->maskWidth(scaleX, angle, subPixelX, subPixelY, info);
    int height = m_d->brush->maskHeight(scaleY, angle, subPixelX, subPixelY, info);

    if (mirrorProperties.horizontalMirror) {
        subPixelX = positiveFraction(-(cursorPoint.x() + hotSpot.x()));
        width = m_d->brush->maskWidth(scaleX, angle, subPixelX, subPixelY, info);
        x = qRound(cursorPoint.x() + subPixelX + hotSpot.x()) - width;
    }

    if (mirrorProperties.verticalMirror) {
        subPixelY = positiveFraction(-(cursorPoint.y() + hotSpot.y()));
        height = m_d->brush->maskHeight(scaleY, angle, subPixelX, subPixelY, info);
        y = qRound(cursorPoint.y() + subPixelY + hotSpot.y()) - height;
    }

    return DabPosition(QRect(x, y, width, height),
                       QPointF(subPixelX, subPixelY),
                       angle);
}

inline
KisFixedPaintDeviceSP KisDabCache::fetchDabCommon(const KoColorSpace *cs,
        const KisColorSource *colorSource,
        const KoColor& color,
        const QPointF &cursorPoint,
        double scaleX, double scaleY,
        double initialAngle,
        const KisPaintInformation& info,
        qreal softnessFactor,
        QRect *dstDabRect)
{
    Q_ASSERT(dstDabRect);

    MirrorProperties mirrorProperties;
    if (m_d->mirrorOption) {
        mirrorProperties = m_d->mirrorOption->apply(info);
    }

    DabPosition position = calculateDabRect(cursorPoint,
                                            scaleX, scaleY,
                                            initialAngle,
                                            info,
                                            mirrorProperties);

    *dstDabRect = position.rect;

    KoColor paintColor = colorSource && colorSource->isUniformColor() ?
                         colorSource->uniformColor() : color;

    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(cs);

    if (m_d->brush->brushType() == IMAGE || m_d->brush->brushType() == PIPE_IMAGE) {
        dab = m_d->brush->paintDevice(cs, scaleX, position.realAngle, info,
                                           position.subPixel.x(),
                                           position.subPixel.y());
    }
    else {
        if (!m_d->colorSourceDevice || !(*cs == *m_d->colorSourceDevice->colorSpace())) {
            m_d->colorSourceDevice = new KisPaintDevice(cs);
        }
        else {
            m_d->colorSourceDevice->clear();
        }

        QRect maskRect(QPoint(), position.rect.size());
        colorSource->colorize(m_d->colorSourceDevice, maskRect, info.pos().toPoint());
        delete m_d->colorSourceDevice->convertTo(cs);

        m_d->brush->mask(dab, m_d->colorSourceDevice, scaleX, scaleY, position.realAngle,
                         info,
                         position.subPixel.x(), position.subPixel.y(),
                         softnessFactor);
    }

    if (!mirrorProperties.isEmpty()) {
        dab->mirror(mirrorProperties.horizontalMirror,
                         mirrorProperties.verticalMirror);
    }

    postProcessDab(dab, position.rect.topLeft(), info);

    return dab;
}

void KisDabCache::postProcessDab(KisFixedPaintDeviceSP dab,
                                 const QPoint &dabTopLeft,
                                 const KisPaintInformation& info)
{
    if (m_d->sharpnessOption) {
        m_d->sharpnessOption->applyThreshold(dab);
    }

    if (m_d->textureOption) {
        m_d->textureOption->apply(dab, dabTopLeft, info);
    }
}
