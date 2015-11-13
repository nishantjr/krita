/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "RgbF16ColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"

#include "compositeops/RgbCompositeOps.h"

RgbF16ColorSpace::RgbF16ColorSpace(const QString &name, KoColorProfile *p) :
        LcmsColorSpace<KoRgbF16Traits>(colorSpaceId(), name, TYPE_RGBA_HALF_FLT, cmsSigRgbData, p)
{
    addChannel(new KoChannelInfo(i18n("Red")  , 0 * sizeof(half), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, QColor(255, 0, 0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1 * sizeof(half), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, QColor(0, 255, 0)));
    addChannel(new KoChannelInfo(i18n("Blue") , 2 * sizeof(half), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, QColor(0, 0, 255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3 * sizeof(half), 3, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT16, 2));

    init();

    addStandardCompositeOps<KoRgbF16Traits>(this);

    addCompositeOp(new RgbCompositeOpIn<KoRgbF16Traits>(this));
    addCompositeOp(new RgbCompositeOpOut<KoRgbF16Traits>(this));
    addCompositeOp(new RgbCompositeOpBumpmap<KoRgbF16Traits>(this));
}

bool RgbF16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16)
        return true;
    else
        return false;
}

KoColorSpace* RgbF16ColorSpace::clone() const
{
    return new RgbF16ColorSpace(name(), profile()->clone());
}

void RgbF16ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoRgbF16Traits::Pixel* p = reinterpret_cast<const KoRgbF16Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("RGB");
    labElt.setAttribute("r", KoColorSpaceMaths< KoRgbF16Traits::channels_type, qreal>::scaleToA(p->red));
    labElt.setAttribute("g", KoColorSpaceMaths< KoRgbF16Traits::channels_type, qreal>::scaleToA(p->green));
    labElt.setAttribute("b", KoColorSpaceMaths< KoRgbF16Traits::channels_type, qreal>::scaleToA(p->blue));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void RgbF16ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoRgbF16Traits::Pixel* p = reinterpret_cast<KoRgbF16Traits::Pixel*>(pixel);
    p->red = KoColorSpaceMaths< qreal, KoRgbF16Traits::channels_type >::scaleToA(elt.attribute("r").toDouble());
    p->green = KoColorSpaceMaths< qreal, KoRgbF16Traits::channels_type >::scaleToA(elt.attribute("g").toDouble());
    p->blue = KoColorSpaceMaths< qreal, KoRgbF16Traits::channels_type >::scaleToA(elt.attribute("b").toDouble());
    p->alpha = 1.0;
}

void RgbF16ColorSpace::toHSY(QVector <double> channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> colorants(9);
    if (profile()->hasColorants()){
        colorants = profile()->getColorantsxyY();
    } else {
        //TODO: Change this to rec 709//
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    if (colorants[2]<0 || colorants[5]<0 || colorants[8]<0) {
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    
    RGBToHSY(channelValues[0],channelValues[1],channelValues[2], hue, sat, luma, colorants[2], colorants[5], colorants[8]);
}

QVector <double> RgbF16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    QVector <double> colorants(9);
    if (profile()->hasColorants()){
        colorants = profile()->getColorantsxyY();
    } else {
        //TODO: Change this to rec 709//
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    if (colorants[2]<0 || colorants[5]<0 || colorants[8]<0) {
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    HSYToRGB(*hue, *sat, *luma, &channelValues[0],&channelValues[1],&channelValues[2], colorants[2], colorants[5], colorants[8]);
    channelValues[3]=1.0;
    return channelValues;
}

void RgbF16ColorSpace::toYCbCr(QVector <double> channelValues, qreal *y, qreal *cb, qreal *cr) const
{
    QVector <double> colorants(9);
    if (profile()->hasColorants()){
        colorants = profile()->getColorantsxyY();
    } else {
        //TODO: Change this to rec 709//
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    if (colorants[2]<0 || colorants[5]<0 || colorants[8]<0) {
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    
    RGBToYCbCr(channelValues[0],channelValues[1],channelValues[2], y, cb, cr, colorants[2], colorants[5], colorants[8]);
}

QVector <double> RgbF16ColorSpace::fromYCbCr(qreal *y, qreal *cb, qreal *cr) const
{
    QVector <double> channelValues(4);
    QVector <double> colorants(9);
    if (profile()->hasColorants()){
        colorants = profile()->getColorantsxyY();
    } else {
        //TODO: Change this to rec 709//
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    if (colorants[2]<0 || colorants[5]<0 || colorants[8]<0) {
        colorants.fill(1.0);
        colorants[2] = 0.30;
        colorants[5] = 0.59;
        colorants[8] = 0.11;
    }
    YCbCrToRGB(*y, *cb, *cr, &channelValues[0],&channelValues[1],&channelValues[2], colorants[2], colorants[5], colorants[8]);
    channelValues[3]=1.0;
    return channelValues;
}