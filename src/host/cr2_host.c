/* SPDX-License-Identifier: GPL-2.0-only */
#include "rebobina/cr2_host.h"
#include "cr2.h"
#include "jpeg_dec.h"

#define RB_CR2_MAX_DIMENSION 12000

int rb_cr2_host_find_preview(const void *data, size_t length,
                            rb_cr2_host_preview *out)
{
    const unsigned char *bytes = (const unsigned char *)data;
    uint32_t off = 0, extent = 0;
    int w = 0, h = 0;
    se_jdec_info jpeg = {0};
    if (!data || !out || length < 16u) return -1;
    /* Soup find_preview analisa TIFF; exigimos antes a assinatura CR2. */
    if (bytes[0] != 'I' || bytes[1] != 'I' ||
        bytes[2] != 0x2a || bytes[3] != 0 ||
        bytes[8] != 'C' || bytes[9] != 'R') return -1;
    if (se_cr2_find_preview(data, length, &off, &extent, &w, &h) != SE_OK)
        return -1;
    /* Sem usar off + extent, que poderia transbordar. */
    if (extent < 4u || (size_t)off > length ||
        (size_t)extent > length - (size_t)off ||
        w <= 0 || h <= 0 || w > RB_CR2_MAX_DIMENSION ||
        h > RB_CR2_MAX_DIMENSION)
        return -1;
    if (se_jpeg_dec_info(bytes + off, (size_t)extent, &jpeg) != SE_OK)
        return -1;
    if (jpeg.width != w || jpeg.height != h || jpeg.ncomp != 3)
        return -1;
    out->offset = off;
    out->bytes = extent;
    out->width = w;
    out->height = h;
    return 0;
}
