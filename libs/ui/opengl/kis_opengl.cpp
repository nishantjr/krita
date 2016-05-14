/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "opengl/kis_opengl.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDesktopServices>
#include <QMessageBox>
#include <QWindow>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_2_1>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>

namespace
{
    bool NeedsFenceWorkaround = false;
    int glVersion = 0;
    QString Renderer;
}

class TestWindow : public QWindow {
public:
    TestWindow(QScreen *screen = 0)
        : QWindow(screen)
    {
        QOpenGLContext *context = 0;
        QSurfaceFormat format;
        QAbstractOpenGLFunctions *f = 0;
        QOpenGLVersionProfile versionProfile;

        // 3.2 Core
        format.setMajorVersion(3);
        format.setMinorVersion(2);
        format.setProfile(QSurfaceFormat::CoreProfile);
        setFormat(format);
        // Create an OpenGL context
        context = new QOpenGLContext;
        context->setFormat(format);
        context->create();
        context->makeCurrent(this);
        versionProfile = QOpenGLVersionProfile(format);
        versionProfile.setVersion(3, 2);
        f = context->versionFunctions(versionProfile);
        version32Core = f;
        delete context;

        // 3.2 Compatibility
        format.setMajorVersion(3);
        format.setMinorVersion(2);
        format.setProfile(QSurfaceFormat::CompatibilityProfile);
        format.setOptions(QSurfaceFormat::DeprecatedFunctions);
        setFormat(format );
        // Create an OpenGL context
        context = new QOpenGLContext;
        context->setFormat( format );
        context->create();
        context->makeCurrent(this);
        versionProfile = QOpenGLVersionProfile(format);
        versionProfile.setVersion(3, 2);
        f = context->versionFunctions(versionProfile);
        version32Compatibility = f;
        delete context;

        // 2.1
        format.setMajorVersion(2);
        format.setMinorVersion(1);
        setFormat(format);
        // Create an OpenGL context
        context = new QOpenGLContext;
        context->setFormat(format);
        context->create();
        context->makeCurrent(this);
        versionProfile = QOpenGLVersionProfile(format);
        versionProfile.setVersion(2, 1);
        f = context->versionFunctions(versionProfile);
        version21 = f;
        delete context;

    }

    bool version32Core;
    bool version32Compatibility;
    bool version21;
};


void KisOpenGL::initialize()
{
//    KisConfig cfg;

    {
        TestWindow w;
        qDebug() << "3.2 core:" << w.version32Core << ", 3.2 compatibility:" << w.version32Compatibility << ", 2.1" << w.version21;

        // And that allows us to use opengl
        if (w.version32Compatibility) {
            glVersion = 303;
        }

    }

    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setOptions(QSurfaceFormat::DeprecatedFunctions);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    // if (cfg.disableDoubleBuffering()) {
    if (false) {
        format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    }
    else {
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    }
    format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

int KisOpenGL::initializeContext(QOpenGLContext* s) {
    KisConfig cfg;
    dbgUI << "OpenGL: Opening new context";

    // Double check we were given the version we requested
    QSurfaceFormat format = s->format();
    glVersion = 100 * format.majorVersion() + format.minorVersion();
    qDebug() << "glVersion";
    QOpenGLFunctions *f = s->functions();

#ifndef GL_RENDERER
#  define GL_RENDERER 0x1F01
#endif
    Renderer = QString((const char*)f->glGetString(GL_RENDERER));

    QFile log(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/krita-opengl.txt");
    dbgUI << "Writing OpenGL log to" << log.fileName();
    log.open(QFile::WriteOnly);
    QString vendor((const char*)f->glGetString(GL_VENDOR));
    log.write(vendor.toLatin1());
    log.write(", ");
    log.write(Renderer.toLatin1());
    log.write(", ");
    QString version((const char*)f->glGetString(GL_VERSION));
    log.write(version.toLatin1());

    // Check if we have a bugged driver that needs fence workaround
    bool isOnX11 = false;
#ifdef HAVE_X11
    isOnX11 = true;
#endif

    if ((isOnX11 && Renderer.startsWith("AMD")) || cfg.forceOpenGLFenceWorkaround()) {
        NeedsFenceWorkaround = true;
    }

    return glVersion;
}

bool KisOpenGL::supportsFenceSync()
{
    return glVersion > 302;
}

bool KisOpenGL::needsFenceWorkaround()
{
    return NeedsFenceWorkaround;
}

QString KisOpenGL::renderer()
{
    return Renderer;
}

bool KisOpenGL::hasOpenGL()
{
    return glVersion > 302;
}
