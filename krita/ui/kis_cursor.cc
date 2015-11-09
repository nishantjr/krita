/*
 *  kis_cursor.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2013 David Revoy <info@davidrevoy.com>
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

#include "kis_cursor.h"


#include <QBitmap>
#include <QCursor>
#include <QImage>
#include <QPainter>
#include <QtGlobal>
#include <qmath.h>
#include <QDebug>

#include <KoResourcePaths.h>


KisCursor::KisCursor() {}

/*
 * Predefined Qt cursors
 */
QCursor KisCursor::arrowCursor()
{
    return Qt::ArrowCursor;
}

QCursor KisCursor::upArrowCursor()
{
    return Qt::UpArrowCursor;
}

QCursor KisCursor::crossCursor()
{
    return load("cursor-cross.xpm");
}

QCursor KisCursor::roundCursor()
{
    return load("cursor-round.xpm");
}

QCursor KisCursor::waitCursor()
{
    return Qt::WaitCursor;
}

QCursor KisCursor::ibeamCursor()
{
    return Qt::IBeamCursor;
}

QCursor KisCursor::sizeVerCursor()
{
    return Qt::SizeVerCursor;
}

QCursor KisCursor::sizeHorCursor()
{
    return Qt::SizeHorCursor;
}

QCursor KisCursor::sizeBDiagCursor()
{
    return Qt::SizeBDiagCursor;
}

QCursor KisCursor::sizeFDiagCursor()
{
    return Qt::SizeFDiagCursor;
}

QCursor KisCursor::sizeAllCursor()
{
    return Qt::SizeAllCursor;
}

QCursor KisCursor::blankCursor()
{
    return Qt::BlankCursor;
}

QCursor KisCursor::splitVCursor()
{
    return Qt::SplitVCursor;
}

QCursor KisCursor::splitHCursor()
{
    return Qt::SplitHCursor;
}

QCursor KisCursor::pointingHandCursor()
{
    return Qt::PointingHandCursor;
}


/*
 * Existing custom KimageShop cursors. Use the 'load' function for all new cursors.
 */

QCursor KisCursor::pickerCursor()
{
    return pickerLayerForegroundCursor();
}


QCursor KisCursor::pickerPlusCursor()
{
    static const unsigned char pickerplus_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x80, 0x1e,
    0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0xb0, 0x1f, 0x00, 0x00, 0xe8, 0x0f,
    0x00, 0x00, 0xd0, 0x07, 0x00, 0x00, 0xa8, 0x03, 0x00, 0x00, 0x64, 0x03,
    0x00, 0x00, 0x72, 0x01, 0x00, 0x00, 0xb9, 0x00, 0x00, 0x80, 0x1c, 0x00,
    0x00, 0x40, 0x0e, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x90, 0x03, 0x00,
    0x00, 0xc8, 0x01, 0x01, 0x40, 0xe4, 0x00, 0x01, 0x40, 0x74, 0xc0, 0x07,
    0x40, 0x3c, 0x00, 0x01, 0x40, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00
    };

    QBitmap picker_bitmap = bitmapFromData(QSize(32, 32), pickerplus_bits);
    QBitmap picker_mask = picker_bitmap.createHeuristicMask(false);

    return QCursor(picker_bitmap, picker_mask, 6, 25);
}


QCursor KisCursor::pickerMinusCursor()
{
    static const unsigned char pickerminus_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x80, 0x1e,
    0x00, 0x00, 0x40, 0x1f, 0x00, 0x00, 0xb0, 0x1f, 0x00, 0x00, 0xe8, 0x0f,
    0x00, 0x00, 0xd0, 0x07, 0x00, 0x00, 0xa8, 0x03, 0x00, 0x00, 0x64, 0x03,
    0x00, 0x00, 0x72, 0x01, 0x00, 0x00, 0xb9, 0x00, 0x00, 0x80, 0x1c, 0x00,
    0x00, 0x40, 0x0e, 0x00, 0x00, 0x20, 0x07, 0x00, 0x00, 0x90, 0x03, 0x00,
    0x00, 0xc8, 0x01, 0x00, 0x40, 0xe4, 0x00, 0x00, 0x40, 0x74, 0xc0, 0x07,
    0x40, 0x3c, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00
    };

    QBitmap picker_bitmap = bitmapFromData(QSize(32, 32), pickerminus_bits);
    QBitmap picker_mask = picker_bitmap.createHeuristicMask(false);

    return QCursor(picker_bitmap, picker_mask, 6, 25);
}

QCursor KisCursor::pickLayerCursor()
{
    return load("precise-pick-layer-icon.xpm", 7, 23);
}

QCursor KisCursor::penCursor()
{
    static const unsigned char pen_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3a, 0x00, 0x00, 0x7d,
        0x00, 0x80, 0x7e, 0x00, 0x40, 0x7f, 0x00, 0xa0, 0x3f, 0x00, 0xd0, 0x1f,
        0x00, 0xe8, 0x0f, 0x00, 0xf4, 0x07, 0x00, 0xfa, 0x03, 0x00, 0xfd, 0x01,
        0x80, 0xfe, 0x00, 0x40, 0x7f, 0x00, 0xa0, 0x3f, 0x00, 0xf0, 0x1f, 0x00,
        0xd0, 0x0f, 0x00, 0x88, 0x07, 0x00, 0x88, 0x03, 0x00, 0xe4, 0x01, 0x00,
        0x7c, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap pen_bitmap = bitmapFromData(QSize(24, 24), pen_bits);
    QBitmap pen_mask = pen_bitmap.createHeuristicMask(false);

    return QCursor(pen_bitmap, pen_mask, 1, 22);
}

QCursor KisCursor::brushCursor()
{
    static const unsigned char brush_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x68, 0x00,
        0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0xfd, 0x00,
        0x00, 0x80, 0x7e, 0x00, 0x00, 0x40, 0x3f, 0x00, 0x00, 0xa0, 0x1f, 0x00,
        0x00, 0xd0, 0x0f, 0x00, 0x00, 0xe8, 0x07, 0x00, 0x00, 0xf4, 0x03, 0x00,
        0x00, 0xe4, 0x01, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x80, 0x41, 0x00, 0x00,
        0x40, 0x32, 0x00, 0x00, 0xa0, 0x0f, 0x00, 0x00, 0xd0, 0x0f, 0x00, 0x00,
        0xd0, 0x0f, 0x00, 0x00, 0xe8, 0x07, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00,
        0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap brush_bitmap = bitmapFromData(QSize(25, 23), brush_bits);
    QBitmap brush_mask = brush_bitmap.createHeuristicMask(false);

    return QCursor(brush_bitmap, brush_mask, 1, 21);
}

QCursor KisCursor::airbrushCursor()
{
    static const unsigned char airbrush_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x68, 0x00, 0x00, 0x74,
        0x00, 0x00, 0x7a, 0xf0, 0x00, 0x3d, 0x08, 0x81, 0x1e, 0xe8, 0x41, 0x0f,
        0xe8, 0xa1, 0x07, 0xe8, 0xd1, 0x03, 0xe8, 0xe9, 0x01, 0xe8, 0xf5, 0x00,
        0xe8, 0x7b, 0x00, 0xf0, 0x33, 0x00, 0xf0, 0x23, 0x1f, 0xa0, 0x9f, 0x3f,
        0xd0, 0xff, 0x31, 0xe8, 0xf7, 0x30, 0xf4, 0x03, 0x18, 0xfc, 0x01, 0x0c,
        0xf8, 0x00, 0x06, 0x76, 0x00, 0x03, 0x36, 0x00, 0x03, 0x00, 0x00, 0x00
    };

    QBitmap airbrush_bitmap = bitmapFromData(QSize(24, 24), airbrush_bits);
    QBitmap airbrush_mask = airbrush_bitmap.createHeuristicMask(false);

    return QCursor(airbrush_bitmap, airbrush_mask, 1, 22);
}

QCursor KisCursor::eraserCursor()
{
    static const unsigned char eraser_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x1d, 0x00,
        0x00, 0x80, 0x3e, 0x00, 0x00, 0x40, 0x7f, 0x00, 0x00, 0xa0, 0xff, 0x00,
        0x00, 0xd0, 0xff, 0x00, 0x00, 0xe8, 0x7f, 0x00, 0x00, 0xf4, 0x3f, 0x00,
        0x00, 0xfe, 0x1f, 0x00, 0x00, 0xf9, 0x0f, 0x00, 0x80, 0xf2, 0x07, 0x00,
        0x40, 0xe7, 0x03, 0x00, 0xa0, 0xcf, 0x01, 0x00, 0xd0, 0x9f, 0x00, 0x00,
        0xe8, 0x7f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf2, 0x1f, 0x00, 0x00,
        0xe2, 0x0f, 0x00, 0x00, 0xc4, 0x07, 0x00, 0x00, 0x88, 0x03, 0x00, 0x00,
        0x10, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap eraser_bitmap = bitmapFromData(QSize(25, 24), eraser_bits);
    QBitmap eraser_mask = eraser_bitmap.createHeuristicMask(false);

    return QCursor(eraser_bitmap, eraser_mask, 7, 22);
}

QCursor KisCursor::fillerCursor()
{
    static const unsigned char filler_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x28, 0x00,
        0x00, 0x54, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x85, 0x00, 0x80, 0x0a, 0x01,
        0x40, 0x11, 0x01, 0xe0, 0x00, 0x02, 0x58, 0x01, 0x04, 0x2c, 0x02, 0x04,
        0x44, 0x04, 0x08, 0x0c, 0x08, 0x18, 0x3c, 0x00, 0x14, 0x5c, 0x00, 0x0a,
        0x9c, 0x01, 0x05, 0x1c, 0x82, 0x02, 0x18, 0x4c, 0x01, 0x18, 0xb0, 0x00,
        0x08, 0x60, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap filler_bitmap = bitmapFromData(QSize(22, 22), filler_bits);
    QBitmap filler_mask = filler_bitmap.createHeuristicMask(false);

    return QCursor(filler_bitmap, filler_mask, 3, 20);
}

QCursor KisCursor::colorChangerCursor()
{
    static const unsigned char colorChanger_bits[] = {
        0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x10, 0x01, 0x0e, 0x08, 0x02, 0x11,
        0x04, 0x82, 0x20, 0x64, 0x84, 0x20, 0x92, 0x44, 0x46, 0x12, 0x49, 0x5f,
        0x12, 0x31, 0x5f, 0x22, 0x01, 0x5f, 0xc2, 0x00, 0x4e, 0x02, 0x00, 0x40,
        0xc2, 0x00, 0x46, 0xe2, 0x01, 0x4f, 0xe4, 0x19, 0x2f, 0xe4, 0x3d, 0x2f,
        0xe8, 0x3d, 0x17, 0xd0, 0x3c, 0x10, 0x20, 0x38, 0x08, 0x40, 0x00, 0x06,
        0x80, 0x81, 0x01, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap colorChanger_bitmap = bitmapFromData(QSize(24, 23), colorChanger_bits);
    QBitmap colorChanger_mask = colorChanger_bitmap.createHeuristicMask(false);

    return QCursor(colorChanger_bitmap, colorChanger_mask, 12, 10);
}

QCursor KisCursor::zoomSmoothCursor()
{
    return load("zoom_smooth.xpm");
}

QCursor KisCursor::zoomDiscreteCursor()
{
    return load("zoom_discrete.xpm");
}

QCursor KisCursor::rotateCanvasSmoothCursor()
{
    return load("rotate_smooth.xpm");
}

QCursor KisCursor::rotateCanvasDiscreteCursor()
{
    return load("rotate_discrete.xpm");
}

QCursor KisCursor::pickerImageForegroundCursor()
{
    return load("color-picker_image_foreground.xpm", 8, 23);
}

QCursor KisCursor::pickerImageBackgroundCursor()
{
    return load("color-picker_image_background.xpm", 8, 23);
}

QCursor KisCursor::pickerLayerForegroundCursor()
{
    return load("color-picker_layer_foreground.xpm", 8, 23);
}

QCursor KisCursor::pickerLayerBackgroundCursor()
{
    return load("color-picker_layer_background.xpm", 8, 23);
}

QCursor KisCursor::changeExposureCursor()
{
    return load("exposure-cursor-gesture.xpm", 8, 23);
}

QCursor KisCursor::changeGammaCursor()
{
    return load("gamma-cursor-gesture.xpm", 8, 23);
}

QCursor KisCursor::triangleLeftHandedCursor()
{
    return load("cursor-triangle_lefthanded.xpm");
}

QCursor KisCursor::triangleRightHandedCursor()
{
    return load("cursor-triangle_righthanded.xpm");
}

QCursor KisCursor::moveCursor()
{
    static const unsigned char move_bits[] = {
        0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7e, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00,
        0x10, 0x18, 0x08, 0x18, 0x18, 0x18, 0x1c, 0x18, 0x38, 0xfe, 0xff, 0x7f,
        0xfe, 0xff, 0x7f, 0x1c, 0x18, 0x38, 0x18, 0x18, 0x18, 0x10, 0x18, 0x08,
        0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x7e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap move_bitmap = bitmapFromData(QSize(24, 24), move_bits);
    QBitmap move_mask = move_bitmap.createHeuristicMask(false);

    return QCursor(move_bitmap, move_mask, 12, 11);
}

QCursor KisCursor::handCursor()
{
    return Qt::PointingHandCursor;
}

QCursor KisCursor::selectCursor()
{
    static const unsigned char select_bits[] = {
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0xff, 0xff, 0x7f,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00
    };

    QBitmap select_bitmap = bitmapFromData(QSize(23, 23), select_bits);
    QBitmap select_mask = select_bitmap.createHeuristicMask(false);

    return QCursor(select_bitmap, select_mask, 11, 11);
}

QCursor KisCursor::openHandCursor()
{
    return Qt::OpenHandCursor;
}

QCursor KisCursor::closedHandCursor()
{
    return Qt::ClosedHandCursor;
}

QCursor KisCursor::rotateCursor()
{
    return load("rotate_cursor.xpm");
}

QCursor KisCursor::load(const QString & cursorName, int hotspotX, int hotspotY)
{
    QImage cursorImage = QImage(":/" + cursorName);
    if (cursorImage.isNull()) {
        qWarning() << "Could not load cursor from qrc, trying filesystem" << cursorName;

        cursorImage = QImage(KoResourcePaths::findResource("kis_pics", cursorName));
        if (cursorImage.isNull()) {
            qWarning() << "Could not load cursor from filesystem" << cursorName;
            return Qt::ArrowCursor;
        }
    }

#ifdef Q_OS_WIN
    // cursor width must be multiple of 16 on Windows
    int bitmapWidth = qCeil(cursorImage.width() / 16.0) * 16;
    if (hotspotX < 0) {
        hotspotX = cursorImage.width() / 2;
    }

    QBitmap bitmap(bitmapWidth, cursorImage.height());
    QBitmap mask(bitmapWidth, cursorImage.height());

    if (bitmapWidth != cursorImage.width()) {
        bitmap.clear();
        mask.clear();
    }
#else
    QBitmap bitmap(cursorImage.width(), cursorImage.height());
    QBitmap mask(cursorImage.width(), cursorImage.height());
#endif

    QPainter bitmapPainter(&bitmap);
    QPainter maskPainter(&mask);

    for (qint32 x = 0; x < cursorImage.width(); ++x) {
        for (qint32 y = 0; y < cursorImage.height(); ++y) {

            QRgb pixel = cursorImage.pixel(x, y);

            if (qAlpha(pixel) < 128) {
                bitmapPainter.setPen(Qt::color0);
                maskPainter.setPen(Qt::color0);
            } else {
                maskPainter.setPen(Qt::color1);

                if (qGray(pixel) < 128) {
                    bitmapPainter.setPen(Qt::color1);
                } else {
                    bitmapPainter.setPen(Qt::color0);
                }
            }

            bitmapPainter.drawPoint(x, y);
            maskPainter.drawPoint(x, y);
        }
    }

    return QCursor(bitmap, mask, hotspotX, hotspotY);
}

QBitmap KisCursor::bitmapFromData(const QSize& size, const unsigned char* data)
{
    QBitmap result(32, 32);
    result.fill(Qt::color0);
    QPainter painter(&result);
    painter.drawPixmap(0, 0, QBitmap::fromData(size, data));
    return result;
}
