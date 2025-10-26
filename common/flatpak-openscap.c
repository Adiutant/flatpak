#include "flatpak-openscap.h"

#include <openscap/oscap.h>
#include <openscap/oval_results.h>
#include <openscap/oval_agent_api.h>
#include <openscap/oval_session.h>

#include <stdio.h>

struct _FlatpakOpenscapContext
{
  struct oval_session *session;
  GObject parent_instance;
};

G_DEFINE_TYPE (FlatpakOpenscapContext, flatpak_openscap_context, G_TYPE_OBJECT)

static void
flatpak_openscap_context_finalize (GObject *object)
{
  oscap_cleanup ();

  G_OBJECT_CLASS (flatpak_openscap_context_parent_class)->finalize (object);
}

static void
flatpak_openscap_context_class_init (FlatpakOpenscapContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = flatpak_openscap_context_finalize;
}

static void
flatpak_openscap_context_init (FlatpakOpenscapContext *self)
{
}

FlatpakOpenscapContext *flatpak_openscap_context_new(const char *scap_file, GError **error) {
    g_autoptr(FlatpakOpenscapContext) context = NULL;
    oscap_init();
    context = g_object_new(FLATPAK_TYPE_OPENSCAP_CONTEXT, NULL);
    const char *version = oscap_get_version();
    g_printerr("OpenSCAP version: %s\n", version);
    if (context == NULL) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NODEV, "Failed to create OpenSCAP context");
        return NULL;
    }
    g_printerr("OpenSCAP file: %s\n", scap_file);
    context->session = oval_session_new(scap_file);
    if (context->session == NULL) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NODEV, "Failed to create OpenSCAP oval session");
        return NULL;
    }
    return g_steal_pointer (&context);
}
static void cleanup(FlatpakOpenscapContext *context) {
    if (context->session!= NULL) {
        oval_session_free(context->session);
        printf("[OK] Сессия OVAL освобождена.\n");
    }
    oscap_cleanup();
    printf("[OK] Ресурсы библиотеки OpenSCAP освобождены.\n");
}

gboolean flatpak_openscap_context_run_scap(FlatpakOpenscapContext *context, const char *target_dir, GError **error) {
    if (setenv("OSCAP_PROBE_ROOT", target_dir,1) != 0) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_IO, "Failed to set OSCAP_PROBE_ROOT environment variable");
        cleanup(context);
        return FALSE;
    }
    printf("OSCAP_PROBE_ROOT установлена в: %s\n", target_dir);
    if(oval_session_load(context->session) != 0 ) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_IO, "Failed to load OVAL file");
        cleanup(context);
        return FALSE; 
    }
    printf("OVAL session loaded successfully\n");
    if (oval_session_evaluate(context->session, NULL, NULL) != 0) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_IO, "Failed to evaluate OVALs");
        cleanup(context);
        return FALSE; 
    }
    printf("OVAL evaluated successfully\n");
    const char *results_file = "oval-results.xml";
    const char *report_file = "oval-report.html";

    oval_session_set_results_export(context->session, results_file);
    oval_session_set_report_export(context->session, report_file);

    printf("Экспорт результатов...\n");
    if (oval_session_export(context->session)!= 0) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_IO, "Failed to export OVAL results");
        cleanup(context);
        return FALSE;
    }
    printf("[OK] Результаты успешно сохранены:\n");
    printf("  - Машиночитаемый отчет: %s\n", results_file);
    printf("  - Человекочитаемый отчет: %s\n", report_file);
    cleanup(context);
    return TRUE;

}