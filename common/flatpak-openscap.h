#ifndef _FLATPAK_OPENSCAP_H_
#define _FLATPAK_OPENSCAP_H_

#include <gio/gio.h>

#define FLATPAK_TYPE_OPENSCAP_CONTEXT (flatpak_openscap_context_get_type ())
G_DECLARE_FINAL_TYPE (FlatpakOpenscapContext, flatpak_openscap_context, FLATPAK, OPENSCAP_CONTEXT, GObject)

G_BEGIN_DECLS

FlatpakOpenscapContext *flatpak_openscap_context_new(const char *oval_file, GError **error);

gboolean flatpak_openscap_context_run_scap(FlatpakOpenscapContext *context, const char *target_dir, GError **error);

G_END_DECLS

#endif // _FLATPAK_OPENSCAP_H_