#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixdata.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "baselayer.h"
#include "build.h"

#ifndef LINKED_GTK
# include "dynamicgtk.h"
#endif

int gtkenabled = 0;

static GdkPixbuf *appicon = NULL;

int gtkbuild_msgbox(char *name, char *msg)
{
	GtkWidget *dialog;

	if (!gtkenabled) return -1;

	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK,
			msg);
	gtk_window_set_title(GTK_WINDOW(dialog), name);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	return 1;
}

int gtkbuild_ynbox(char *name, char *msg)
{
	int r;
	GtkWidget *dialog;

	if (!gtkenabled) return -1;

	dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_YES_NO,
			msg);
	gtk_window_set_title(GTK_WINDOW(dialog), name);
	r = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	if (r == GTK_RESPONSE_YES) return 1;
	return 0;
}

#ifdef RENDERTYPESDL
#include "sdlayer.h"
extern struct sdlappicon sdlappicon;
#endif
void gtkbuild_init(int *argc, char ***argv)
{
#ifndef LINKED_GTK
	gtkenabled = dynamicgtk_init();
	if (gtkenabled < 0) {
		gtkenabled = 0;
		return;
	}
#endif
	gtkenabled = gtk_init_check(argc, argv);
	if (!gtkenabled) return;
#ifdef RENDERTYPESDL
	appicon = gdk_pixbuf_new_from_data((const guchar *)sdlappicon.pixels,
			GDK_COLORSPACE_RGB, TRUE, 8, sdlappicon.width, sdlappicon.height,
			sdlappicon.width*4, NULL, NULL);
#endif
	if (appicon) gtk_window_set_default_icon(appicon);
}

void gtkbuild_exit(int r)
{
	if (gtkenabled) {
		if (appicon) g_object_unref((gpointer)appicon);
	}
#ifndef LINKED_GTK
	dynamicgtk_uninit();
#endif
}
