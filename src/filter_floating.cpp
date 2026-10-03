/*
 * filter_floating.cpp -- gentle procedural floating/drift motion
 *
 * Nudges the incoming frame by a small, slowly-oscillating position and
 * rotation offset, built from a couple of summed sine waves at different
 * frequencies so the motion feels organic rather than a perfect metronome.
 * No keyframes required: amplitude and speed are set once and the effect
 * runs for the whole clip.
 *
 * This is an original filter written for this repository; it is not
 * derived from any upstream MLT code (see CREDITS.md).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "common.h"
#include <framework/mlt.h>
#include <math.h>
#include <QImage>
#include <QPainter>
#include <QTransform>

namespace {

constexpr double TWO_PI = 6.283185307179586;

struct FloatOffset
{
    double dx, dy, rotation;
};

// Sum of two sine waves per axis (different frequency ratios) so the path
// doesn't look like a perfect circle/ellipse. `seed` shifts the phases so
// several clips using this effect don't drift in lockstep with each other.
FloatOffset compute_offset(double t, double amp_x, double amp_y, double amp_rotation,
                           double speed, int seed)
{
    double ph1 = seed * 0.73;
    double ph2 = seed * 1.91 + 1.7;
    double ph3 = seed * 1.37 + 0.4;
    double ph4 = seed * 2.21 + 2.9;
    double ph5 = seed * 0.59 + 1.1;

    FloatOffset o;
    o.dx = amp_x
           * (0.7 * sin(TWO_PI * speed * t + ph1) + 0.3 * sin(TWO_PI * speed * 2.3 * t + ph2));
    o.dy = amp_y
           * (0.7 * sin(TWO_PI * speed * 0.8 * t + ph3)
              + 0.3 * sin(TWO_PI * speed * 1.9 * t + ph4));
    o.rotation = amp_rotation * sin(TWO_PI * speed * 0.6 * t + ph5);
    return o;
}

} // namespace

/** Get the image.
*/
static int filter_get_image(mlt_frame frame,
                            uint8_t **image,
                            mlt_image_format *format,
                            int *width,
                            int *height,
                            int writable)
{
    mlt_filter filter = (mlt_filter) mlt_frame_pop_service(frame);
    mlt_properties properties = MLT_FILTER_PROPERTIES(filter);

    mlt_service_lock(MLT_FILTER_SERVICE(filter));
    mlt_profile profile = mlt_service_profile(MLT_FILTER_SERVICE(filter));
    mlt_position position = mlt_filter_get_position(filter, frame);
    mlt_service_unlock(MLT_FILTER_SERVICE(filter));

    double fps = mlt_profile_fps(profile);
    double t = fps > 0.0 ? (double) position / fps : 0.0;

    double amp_x = mlt_properties_exists(properties, "amp_x")
                       ? mlt_properties_anim_get_double(properties, "amp_x", position, 0)
                       : 15.0;
    double amp_y = mlt_properties_exists(properties, "amp_y")
                       ? mlt_properties_anim_get_double(properties, "amp_y", position, 0)
                       : 10.0;
    double amp_rotation
        = mlt_properties_exists(properties, "amp_rotation")
              ? mlt_properties_anim_get_double(properties, "amp_rotation", position, 0)
              : 1.5;
    double speed = mlt_properties_exists(properties, "speed")
                       ? mlt_properties_get_double(properties, "speed")
                       : 0.2;
    int seed = mlt_properties_exists(properties, "seed") ? mlt_properties_get_int(properties, "seed")
                                                          : 0;
    double pivot_x = mlt_properties_exists(properties, "pivot_x")
                          ? mlt_properties_get_double(properties, "pivot_x")
                          : 0.5;
    double pivot_y = mlt_properties_exists(properties, "pivot_y")
                          ? mlt_properties_get_double(properties, "pivot_y")
                          : 0.5;
    if (speed < 0.0)
        speed = 0.0;

    *format = choose_image_format(*format);
    uint8_t *src_image = NULL;
    int b_width = 0, b_height = 0;
    int error = mlt_frame_get_image(frame, &src_image, format, &b_width, &b_height, 0);
    if (error || !src_image) {
        *width = b_width;
        *height = b_height;
        *image = src_image;
        return error;
    }

    *width = b_width;
    *height = b_height;

    if (*format != mlt_image_rgba) {
        // Unsupported format: pass through unchanged rather than failing.
        *image = src_image;
        return 0;
    }

    FloatOffset offset = compute_offset(t, amp_x, amp_y, amp_rotation, speed, seed);

    QImage sourceImage;
    convert_mlt_to_qimage(src_image, &sourceImage, *width, *height, *format);

    struct mlt_image_s dest_image_desc;
    mlt_image_set_values(&dest_image_desc, NULL, *format, *width, *height);
    int image_size = mlt_image_calculate_size(&dest_image_desc);
    uint8_t *dest_image = (uint8_t *) mlt_pool_alloc(image_size);

    QImage destImage;
    convert_mlt_to_qimage(dest_image, &destImage, *width, *height, *format);
    destImage.fill(0);

    double pivotX = *width * pivot_x;
    double pivotY = *height * pivot_y;

    QTransform t_transform;
    t_transform.translate(pivotX + offset.dx, pivotY + offset.dy);
    if (offset.rotation != 0.0)
        t_transform.rotate(offset.rotation);
    t_transform.translate(-pivotX, -pivotY);

    QPainter painter(&destImage);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setTransform(t_transform);
    painter.drawImage(0, 0, sourceImage);
    painter.end();

    convert_qimage_to_mlt(&destImage, dest_image, *width, *height);

    *image = dest_image;
    mlt_frame_set_image(frame, *image, image_size, mlt_pool_release);
    return 0;
}

/** Filter processing.
*/
static mlt_frame filter_process(mlt_filter filter, mlt_frame frame)
{
    mlt_frame_push_service(frame, filter);
    mlt_frame_push_get_image(frame, filter_get_image);

    return frame;
}

/** Constructor for the filter.
*/
extern "C" {

mlt_filter filter_floating_init(mlt_profile profile, mlt_service_type type, const char *id, char *arg)
{
    mlt_filter filter = mlt_filter_new();

    if (filter && createQApplicationIfNeeded(MLT_FILTER_SERVICE(filter))) {
        filter->process = filter_process;
        mlt_properties properties = MLT_FILTER_PROPERTIES(filter);
        mlt_properties_set_double(properties, "amp_x", 15.0);
        mlt_properties_set_double(properties, "amp_y", 10.0);
        mlt_properties_set_double(properties, "amp_rotation", 1.5);
        mlt_properties_set_double(properties, "speed", 0.2);
        mlt_properties_set_int(properties, "seed", 0);
        mlt_properties_set_double(properties, "pivot_x", 0.5);
        mlt_properties_set_double(properties, "pivot_y", 0.5);
    } else {
        mlt_log_error(MLT_FILTER_SERVICE(filter), "Filter floating failed\n");

        if (filter) {
            mlt_filter_close(filter);
        }

        filter = NULL;
    }
    return filter;
}
}
