#include <string.h>
#include <inttypes.h>
#include "hal/hal_flash.h"
#include "ffs/ffs.h"
#include "ffsutil/ffsutil.h"
#include "bootutil/crc32.h"
#include "bootutil/image.h"
#include "bootutil/bootutil.h"

#define BOOT_PATH_MAIN      "/boot/main"
#define BOOT_PATH_TEST      "/boot/test"
#define BOOT_PATH_STATUS    "/boot/status"

int
boot_crc_is_valid(uint32_t addr, const struct image_header *hdr)
{
    uint32_t crc_off;
    uint32_t crc_len;
    uint32_t crc;

    crc_off = offsetof(struct image_header, ih_crc32) + sizeof hdr->ih_crc32;
    crc_len = hdr->ih_hdr_size - crc_off + hdr->ih_img_size;
    crc = crc32(0, (void *)(addr + crc_off), crc_len);

    return crc == hdr->ih_crc32;
}

static int
boot_vect_read_one(struct image_version *ver, const char *path)
{
    uint32_t len;
    int rc;

    len = sizeof *ver;
    rc = ffsutil_read_file(path, ver, 0, &len);
    if (rc != 0 || len != sizeof *ver) {
        return BOOT_EBADVECT;
    }

    return 0;
}

int
boot_vect_read_test(struct image_version *out_ver)
{
    int rc;

    rc = boot_vect_read_one(out_ver, BOOT_PATH_TEST);
    return rc;
}

int
boot_vect_read_main(struct image_version *out_ver)
{
    int rc;

    rc = boot_vect_read_one(out_ver, BOOT_PATH_MAIN);
    return rc;
}

int
boot_vect_delete_test(void)
{
    int rc;

    rc = ffs_unlink(BOOT_PATH_TEST);
    return rc;
}

int
boot_vect_delete_main(void)
{
    int rc;

    rc = ffs_unlink(BOOT_PATH_MAIN);
    return rc;
}

static int
boot_read_image_header(struct image_header *out_hdr, uint32_t flash_address)
{
    int rc;

    rc = flash_read(out_hdr, flash_address, sizeof *out_hdr);
    if (rc != 0) {
        return BOOT_EFLASH;
    }

    if (out_hdr->ih_magic != IMAGE_MAGIC) {
        return BOOT_EBADIMAGE;
    }

    return 0;
}

void
boot_read_image_headers(struct image_header *out_headers, int *out_num_headers,
                        const uint32_t *addresses, int num_addresses)
{
    int rc;
    int i;

    for (i = 0; i < num_addresses; i++) {
        rc = boot_read_image_header(out_headers + i, addresses[i]);
        if (rc != 0) {
            break;
        }
    }

    *out_num_headers = i;
}

int
boot_read_status(struct boot_status *out_status,
                 struct boot_status_entry *out_entries,
                 int num_sectors)
{
    struct ffs_file *file;
    uint32_t len;
    int rc;

    rc = ffs_open(&file, BOOT_PATH_STATUS, FFS_ACCESS_READ);
    if (rc != 0) {
        rc = BOOT_EBADSTATUS;
        goto done;
    }

    len = sizeof *out_status;
    rc = ffs_read(file, out_status, &len);
    if (rc != 0 || len != sizeof out_status) {
        rc = BOOT_EBADSTATUS;
        goto done;
    }

    len = num_sectors * sizeof *out_entries;
    rc = ffs_read(file, out_entries, &len);
    if (rc != 0 || len != num_sectors * sizeof out_entries) {
        rc = BOOT_EBADSTATUS;
        goto done;
    }

    rc = 0;

done:
    ffs_close(file);
    return rc;
}

int
boot_write_status(const struct boot_status *status,
                  const struct boot_status_entry *entries,
                  int num_sectors)
{
    struct ffs_file *file;
    int rc;

    rc = ffs_open(&file, BOOT_PATH_STATUS,
                  FFS_ACCESS_WRITE | FFS_ACCESS_TRUNCATE);
    if (rc != 0) {
        rc = BOOT_EFILE;
        goto done;
    }

    rc = ffs_write(file, status, sizeof *status);
    if (rc != 0) {
        rc = BOOT_EFILE;
        goto done;
    }

    rc = ffs_write(file, entries, num_sectors * sizeof *entries);
    if (rc != 0) {
        rc = BOOT_EFILE;
        goto done;
    }

    rc = 0;

done:
    ffs_close(file);
    return rc;
}

void
boot_clear_status(void)
{
    ffs_unlink(BOOT_PATH_STATUS);
}