/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwasmintegration.h"
#include "qwasmeventtranslator.h"
#include "qwasmeventdispatcher.h"
#include "qwasmcompositor.h"
#include "qwasmopenglcontext.h"
#include "qwasmtheme.h"
#include "qwasmclipboard.h"

#include "qwasmwindow.h"
#ifndef QT_NO_OPENGL
# include "qwasmbackingstore.h"
#endif
#include "qwasmfontdatabase.h"
#if defined(Q_OS_UNIX)
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#endif
#include <qpa/qplatformwindow.h>
#include <QtGui/qscreen.h>
#include <qpa/qwindowsysteminterface.h>
#include <QtCore/qcoreapplication.h>

#include <emscripten/bind.h>

// this is where EGL headers are pulled in, make sure it is last
#include "qwasmscreen.h"

using namespace emscripten;
QT_BEGIN_NAMESPACE

void browserBeforeUnload(emscripten::val)
{
    QWasmIntegration::QWasmBrowserExit();
}

EMSCRIPTEN_BINDINGS(my_module)
{
    function("browserBeforeUnload", &browserBeforeUnload);
}

QWasmIntegration *QWasmIntegration::s_instance;

QWasmIntegration::QWasmIntegration()
    : m_fontDb(nullptr),
      m_compositor(new QWasmCompositor),
      m_screen(new QWasmScreen(m_compositor)),
      m_eventDispatcher(nullptr),
      m_clipboard(new QWasmClipboard)
{

    globalHtml5Integration = this;
    s_instance = this;

    emscripten::val defaultCanvasId = emscripten::val::global("canvas");
    canvasIds.append(QString::fromStdString(defaultCanvasId["id"].as<std::string>()));
    m_screen->setCanvas(canvasIds.at(0));

    globalHtml5Integration = this;

    screen()->updateQScreenAndCanvasRenderSize(m_screen->m_canvasId);
    screenAdded(m_screen);

    m_eventTranslator = new QWasmEventTranslator;

    emscripten::val::global("window").set("onbeforeunload", val::module_property("browserBeforeUnload"));

}

QWasmIntegration::~QWasmIntegration()
{
    delete m_compositor;
    destroyScreen(m_screen);
    delete m_fontDb;
    delete m_eventTranslator;
    s_instance = nullptr;
}

void QWasmIntegration::QWasmBrowserExit()
{
    QCoreApplication *app = QCoreApplication::instance();
    app->quit();
}

bool QWasmIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    case RasterGLSurface: return false; // to enable this you need to fix qopenglwidget and quickwidget for wasm
    case MultipleWindows: return true;
    case WindowManagement: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QWasmIntegration::createPlatformWindow(QWindow *window) const
{
    return new QWasmWindow(window, m_compositor, m_backingStores.value(window));
}

QPlatformBackingStore *QWasmIntegration::createPlatformBackingStore(QWindow *window) const
{
#ifndef QT_NO_OPENGL
    QWasmBackingStore *backingStore = new QWasmBackingStore(m_compositor, window);
    m_backingStores.insert(window, backingStore);
    return backingStore;
#else
    return nullptr;
#endif
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QWasmIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    return new QWasmOpenGLContext(context->format());
}
#endif

QPlatformFontDatabase *QWasmIntegration::fontDatabase() const
{
    if (m_fontDb == nullptr)
        m_fontDb = new QWasmFontDatabase;

    return m_fontDb;
}

QAbstractEventDispatcher *QWasmIntegration::createEventDispatcher() const
{
    return new QWasmEventDispatcher;
}

QVariant QWasmIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
    return QPlatformIntegration::styleHint(hint);
}

QStringList QWasmIntegration::themeNames() const
{
    return QStringList() << QLatin1String("webassembly");
}

QPlatformTheme *QWasmIntegration::createPlatformTheme(const QString &name) const
{
    if (name == QLatin1String("webassembly"))
        return new QWasmTheme;
    return QPlatformIntegration::createPlatformTheme(name);
}

QPlatformClipboard* QWasmIntegration::clipboard() const
{
    if (!m_clipboard)
        m_clipboard = new QWasmClipboard;
    return m_clipboard;
}

QT_END_NAMESPACE
