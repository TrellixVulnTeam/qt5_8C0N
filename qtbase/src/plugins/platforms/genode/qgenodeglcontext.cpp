/*
 * \brief  QGenodeGLContext
 * \author Christian Prochaska
 * \date   2013-11-18
 */

/*
 * Copyright (C) 2013-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/log.h>

/* EGL includes */
#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>

/* Qt includes */
#include <QtEglSupport/private/qeglconvenience_p.h>
#include <QDebug>

/* local includes */
#include "qgenodeplatformwindow.h"
#include "qgenodeglcontext.h"

static const bool qnglc_verbose = false;

QT_BEGIN_NAMESPACE

QGenodeGLContext::QGenodeGLContext(QOpenGLContext *context)
    : QPlatformOpenGLContext()
{
	if (qnglc_verbose)
		Genode::log(__func__, "called");

	if (!eglBindAPI(EGL_OPENGL_API))
		qFatal("eglBindAPI() failed");

    _egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (_egl_display == EGL_NO_DISPLAY)
		qFatal("eglGetDisplay() failed");

	int major = -1;
	int minor = -1;
	if (!eglInitialize(_egl_display, &major, &minor))
		qFatal("eglInitialize() failed");

	if (qnglc_verbose)
		Genode::log("eglInitialize() returned major: ", major, ", minor: ", minor);

    _egl_config = q_configFromGLFormat(_egl_display, context->format(), false, EGL_PBUFFER_BIT);
    if (_egl_config == 0)
        qFatal("Could not find a matching EGL config");

	_format = q_glFormatFromConfig(_egl_display, _egl_config);

	_egl_context = eglCreateContext(_egl_display, _egl_config, EGL_NO_CONTEXT, 0);
    if (_egl_context == EGL_NO_CONTEXT)
        qFatal("eglCreateContext() failed");
}


bool QGenodeGLContext::makeCurrent(QPlatformSurface *surface)
{
	if (qnglc_verbose)
		Genode::log(__func__, " called");

	doneCurrent();

	QGenodePlatformWindow *w = static_cast<QGenodePlatformWindow*>(surface);

	Genode_egl_window egl_window = { w->geometry().width(),
		                             w->geometry().height(),
		                             w->framebuffer() };

	if (qnglc_verbose)
		Genode::log(__func__, ": w->framebuffer()=", w->framebuffer());

	if (w->egl_surface() != EGL_NO_SURFACE)
		if (!eglDestroySurface(_egl_display, w->egl_surface()))
			qFatal("eglDestroySurface() failed");

	EGLSurface egl_surface =
		eglCreatePixmapSurface(_egl_display, _egl_config, &egl_window, 0);

	if (egl_surface == EGL_NO_SURFACE)
		qFatal("eglCreateiWindowSurface() failed");

	w->egl_surface(egl_surface);

	if (!eglMakeCurrent(_egl_display, w->egl_surface(), w->egl_surface(), _egl_context))
		qFatal("eglMakeCurrent() failed");

	return true;
}


void QGenodeGLContext::doneCurrent()
{
	if (qnglc_verbose)
		Genode::log(__func__, " called");

	if (!eglMakeCurrent(_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
		qFatal("eglMakeCurrent() failed");
}


void QGenodeGLContext::swapBuffers(QPlatformSurface *surface)
{
	if (qnglc_verbose)
		Genode::log(__func__, " called");

	QGenodePlatformWindow *w = static_cast<QGenodePlatformWindow*>(surface);

	if (!eglSwapBuffers(_egl_display, w->egl_surface()))
		qFatal("eglSwapBuffers() failed");

	w->refresh(0, 0, w->geometry().width(), w->geometry().height());
}


QFunctionPointer QGenodeGLContext::getProcAddress(const char *procName)
{
	if (qnglc_verbose)
		Genode::log("procName=", Genode::Cstring(procName), " , "
		            "pointer=", eglGetProcAddress(procName));

	return static_cast<QFunctionPointer>(eglGetProcAddress(procName));
}


QSurfaceFormat QGenodeGLContext::format() const
{
    return _format;
}

QT_END_NAMESPACE

