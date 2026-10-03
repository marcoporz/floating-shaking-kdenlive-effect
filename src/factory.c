/*
 * Standalone MLT module exposing the "floating" filter (gentle, automatic
 * drift/hover motion for a clip or PNG, no keyframes needed).
 *
 * Original filter, not derived from any upstream MLT code.
 * License: LGPL-2.1-or-later, same as MLT.
 */
#include <framework/mlt.h>
#include <limits.h>
#include <stdio.h>

extern mlt_filter filter_floating_init(mlt_profile profile,
                                       mlt_service_type type,
                                       const char *id,
                                       char *arg);

static mlt_properties metadata(mlt_service_type type, const char *id, void *data)
{
    char file[PATH_MAX];
    snprintf(file, PATH_MAX, "%s/floating/%s", mlt_environment("MLT_DATA"), (char *) data);
    return mlt_properties_parse_yaml(file);
}

MLT_REPOSITORY
{
    MLT_REGISTER(mlt_service_filter_type, "floating", filter_floating_init);
    MLT_REGISTER_METADATA(mlt_service_filter_type, "floating", metadata, "filter_floating.yml");
}
