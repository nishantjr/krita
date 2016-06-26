/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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

#include <brushengine/kis_paintop_settings.h>

#include <QImage>
#include <QColor>
#include <QPointer>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>
#include <KoViewConverter.h>

#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paintop_registry.h"
#include <brushengine/kis_paint_information.h>
#include "kis_paintop_config_widget.h"
#include <brushengine/kis_paintop_preset.h>
#include <time.h>
#include<kis_types.h>

struct Q_DECL_HIDDEN KisPaintOpSettings::Private {
    Private() : disableDirtyNotifications(false) {}

    QPointer<KisPaintOpConfigWidget> settingsWidget;
    QString modelName;
    KisPaintOpPresetWSP preset;


    bool disableDirtyNotifications;

    class DirtyNotificationsLocker {
    public:
        DirtyNotificationsLocker(KisPaintOpSettings::Private *d)
            : m_d(d),
              m_oldNotificationsState(d->disableDirtyNotifications)
        {
            m_d->disableDirtyNotifications = true;
        }

        ~DirtyNotificationsLocker() {
            m_d->disableDirtyNotifications = m_oldNotificationsState;
        }

    private:
        KisPaintOpSettings::Private *m_d;
        bool m_oldNotificationsState;
        Q_DISABLE_COPY(DirtyNotificationsLocker)
    };
};


KisPaintOpSettings::KisPaintOpSettings()
    : d(new Private)
{
    d->preset = 0;
}

KisPaintOpSettings::~KisPaintOpSettings()
{
}

void KisPaintOpSettings::setOptionsWidget(KisPaintOpConfigWidget* widget)
{
    d->settingsWidget = widget;
}
void KisPaintOpSettings::setPreset(KisPaintOpPresetWSP preset)
{
    d->preset = preset;
}
KisPaintOpPresetWSP KisPaintOpSettings::preset() const
{
    return d->preset;
}

bool KisPaintOpSettings::mousePressEvent(const KisPaintInformation &pos, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    Q_UNUSED(pos);
    Q_UNUSED(modifiers);
    Q_UNUSED(currentNode);
    setRandomOffset();
    return true; // ignore the event by default
}
void KisPaintOpSettings::setRandomOffset()
{
    srand(time(0));
    bool isRandomOffsetX = KisPropertiesConfiguration::getBool("Texture/Pattern/isRandomOffsetX");
    bool isRandomOffsetY = KisPropertiesConfiguration::getBool("Texture/Pattern/isRandomOffsetY");
    int offsetX = KisPropertiesConfiguration::getInt("Texture/Pattern/OffsetX");
    int offsetY = KisPropertiesConfiguration::getInt("Texture/Pattern/OffsetY");
    if (KisPropertiesConfiguration::getBool("Texture/Pattern/Enabled")) {
        if (isRandomOffsetX) {
            offsetX = rand() % KisPropertiesConfiguration::getInt("Texture/Pattern/MaximumOffsetX");

            KisPropertiesConfiguration::setProperty("Texture/Pattern/OffsetX", offsetX);
            offsetX = KisPropertiesConfiguration::getInt("Texture/Pattern/OffsetX");

        }

        if (isRandomOffsetY) {
            offsetY = rand() % KisPropertiesConfiguration::getInt("Texture/Pattern/MaximumOffsetY");
            KisPropertiesConfiguration::setProperty("Texture/Pattern/OffsetY", offsetY);
            offsetY = KisPropertiesConfiguration::getInt("Texture/Pattern/OffsetY");
        }
    }
}


KisPaintOpSettingsSP KisPaintOpSettings::clone() const
{
    QString paintopID = getString("paintop");
    if (paintopID.isEmpty())
        return 0;

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(KoID(paintopID, ""));
    QMapIterator<QString, QVariant> i(getProperties());
    while (i.hasNext()) {
        i.next();
        settings->setProperty(i.key(), QVariant(i.value()));
    }
    settings->setPreset(this->preset());
    return settings;
}

void KisPaintOpSettings::activate()
{
}

void KisPaintOpSettings::changePaintOpSize(qreal x, qreal y)
{
    if (!d->settingsWidget.isNull()) {
        d->settingsWidget.data()->changePaintOpSize(x, y);
        d->settingsWidget.data()->writeConfiguration(this);
    }
}


QSizeF KisPaintOpSettings::paintOpSize() const
{
    if (!d->settingsWidget.isNull()) {
        return d->settingsWidget.data()->paintOpSize();
    }
    return QSizeF(1.0, 1.0);
}

void KisPaintOpSettings::setPaintOpOpacity(qreal value)
{
    setProperty("OpacityValue", value);
}

void KisPaintOpSettings::setPaintOpFlow(qreal value)
{
    setProperty("FlowValue", value);
}

void KisPaintOpSettings::setPaintOpCompositeOp(const QString &value)
{
    setProperty("CompositeOp", value);
}

qreal KisPaintOpSettings::paintOpOpacity() const
{
    return getDouble("OpacityValue", 1.0);
}

qreal KisPaintOpSettings::paintOpFlow() const
{
    return getDouble("FlowValue", 1.0);
}

QString KisPaintOpSettings::paintOpCompositeOp() const
{
    return getString("CompositeOp", COMPOSITE_OVER);
}

void KisPaintOpSettings::setEraserMode(bool value)
{
    setProperty("EraserMode", value);
}

bool KisPaintOpSettings::eraserMode() const
{
    return getBool("EraserMode", false);
}

QString KisPaintOpSettings::effectivePaintOpCompositeOp() const
{
    return !eraserMode() ? paintOpCompositeOp() : COMPOSITE_ERASE;
}

qreal KisPaintOpSettings::savedEraserSize() const
{
    return getDouble("SavedEraserSize", 0.0);
}

void KisPaintOpSettings::setSavedEraserSize(qreal value)
{
    setProperty("SavedEraserSize", value);
    setPropertyNotSaved("SavedEraserSize");
}

qreal KisPaintOpSettings::savedBrushSize() const
{
    return getDouble("SavedBrushSize", 0.0);
}

void KisPaintOpSettings::setSavedBrushSize(qreal value)
{
    setProperty("SavedBrushSize", value);
    setPropertyNotSaved("SavedBrushSize");
}


QString KisPaintOpSettings::modelName() const
{
    return d->modelName;
}

void KisPaintOpSettings::setModelName(const QString & modelName)
{
    d->modelName = modelName;
}

KisPaintOpConfigWidget* KisPaintOpSettings::optionsWidget() const
{
    if (d->settingsWidget.isNull())
        return 0;

    return d->settingsWidget.data();
}

bool KisPaintOpSettings::isValid() const
{
    return true;
}

bool KisPaintOpSettings::isLoadable()
{
    return isValid();
}

QString KisPaintOpSettings::indirectPaintingCompositeOp() const
{
    return COMPOSITE_ALPHA_DARKEN;
}

QPainterPath KisPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {
        path = ellipseOutline(10, 10, 1.0, 0);
        
        if (mode == CursorTiltOutline) {
            QPainterPath tiltLine;
            QLineF tiltAngle(QPointF(0.0,0.0), QPointF(0.0,3.0));
            tiltAngle.setLength(50.0 * (1 - info.tiltElevation(info, 60.0, 60.0, true)));
            tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))-2.0);
            tiltLine.moveTo(tiltAngle.p1());
            tiltLine.lineTo(tiltAngle.p2());
            tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))+2.0);
            tiltLine.lineTo(tiltAngle.p2());
            tiltLine.lineTo(tiltAngle.p1());
            path.addPath(tiltLine);
        }

        path.translate(info.pos());
    }

    return path;
}

QPainterPath KisPaintOpSettings::ellipseOutline(qreal width, qreal height, qreal scale, qreal rotation)
{
    QPainterPath path;
    QRectF ellipse(0, 0, width * scale, height * scale);
    ellipse.translate(-ellipse.center());
    path.addEllipse(ellipse);

    QTransform m;
    m.reset();
    m.rotate(rotation);
    path = m.map(path);
    return path;
}

void KisPaintOpSettings::setCanvasRotation(qreal angle)
{
    Private::DirtyNotificationsLocker locker(d.data());

    setProperty("runtimeCanvasRotation", angle);
    setPropertyNotSaved("runtimeCanvasRotation");
}

void KisPaintOpSettings::setCanvasMirroring(bool xAxisMirrored, bool yAxisMirrored)
{
    Private::DirtyNotificationsLocker locker(d.data());

    setProperty("runtimeCanvasMirroredX", xAxisMirrored);
    setPropertyNotSaved("runtimeCanvasMirroredX");

    setProperty("runtimeCanvasMirroredY", yAxisMirrored);
    setPropertyNotSaved("runtimeCanvasMirroredY");
}

void KisPaintOpSettings::setProperty(const QString & name, const QVariant & value)
{
    if (value != KisPropertiesConfiguration::getProperty(name) &&
        !d->disableDirtyNotifications && this->preset()) {

        this->preset()->setPresetDirty(true);
    }

    KisPropertiesConfiguration::setProperty(name, value);
    onPropertyChanged();
}


void KisPaintOpSettings::onPropertyChanged()
{

}

bool KisPaintOpSettings::isLodUserAllowed(const KisPropertiesConfiguration *config)
{
    return config->getBool("lodUserAllowed", true);
}

void KisPaintOpSettings::setLodUserAllowed(KisPropertiesConfiguration *config, bool value)
{
    config->setProperty("lodUserAllowed", value);
}
