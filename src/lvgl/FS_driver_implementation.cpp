/**
 **************************************************
 *
 * @file        FS_driver_implementation.cpp
 * @brief       LVGL filesystem driver implementation for the SD card
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#include "FS_driver_implementation.h"
#include "Inkplate-LVGL.h"
#ifndef ARDUINO_INKPLATE2

/**
 * @brief       LVGL 'open' callback. Opens a file on the SD card.
 *
 *              Strips a leading '/' from the path (SdFat expects a relative
 *              path from the volume root). Retries up to three times with
 *              progressive delays to handle SD cold-start SPI failures.
 *
 * @param       lv_fs_drv_t *drv
 *              Pointer to the registered LVGL filesystem driver.
 *
 * @param       const char *path
 *              File path as supplied by LVGL (leading drive letter already
 *              stripped, e.g. "/cat.jpg").
 *
 * @param       lv_fs_mode_t mode
 *              LV_FS_MODE_RD for read, LV_FS_MODE_WR for write/create.
 *
 * @return      Pointer to a heap-allocated SdFile on success, NULL on failure.
 */
static void * sd_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {

  Inkplate *inkplate = static_cast<Inkplate *>(drv->user_data);
  SdFat &sd = inkplate->getSdFat();

  if (*path == '/') path++;

  SdFile *f = new SdFile();
  oflag_t flags = (mode == LV_FS_MODE_WR) ? (O_WRITE | O_CREAT | O_TRUNC) : O_READ;
  bool ok = f->open(&sd, path, flags);

  static const uint16_t retryDelaysMs[] = {10, 50, 100};
  for (int i = 0; !ok && i < 3; i++) {
    vTaskDelay(pdMS_TO_TICKS(retryDelaysMs[i]));
    ok = f->open(&sd, path, flags);
  }

  if (!ok) {
    delete f;
    return NULL;
  }

  f->seekSet(0);
  return f;
}

/**
 * @brief       LVGL 'close' callback. Closes and frees a file handle.
 *
 * @param       lv_fs_drv_t *drv
 *              Pointer to the registered LVGL filesystem driver.
 *
 * @param       void *file_p
 *              SdFile pointer previously returned by sd_open().
 *
 * @return      LV_FS_RES_OK always.
 */
static lv_fs_res_t sd_close(lv_fs_drv_t *drv, void *file_p) {
  SdFile *f = static_cast<SdFile*>(file_p);
  f->close();
  delete f;
  return LV_FS_RES_OK;
}

/**
 * @brief       LVGL 'read' callback. Reads bytes from an open file.
 *
 * @param       lv_fs_drv_t *drv
 *              Pointer to the registered LVGL filesystem driver.
 *
 * @param       void *file_p
 *              SdFile pointer previously returned by sd_open().
 *
 * @param       void *buf
 *              Destination buffer for the read data.
 *
 * @param       uint32_t btr
 *              Number of bytes to read.
 *
 * @param       uint32_t *br
 *              Set to the number of bytes actually read on success.
 *
 * @return      LV_FS_RES_OK on success, LV_FS_RES_UNKNOWN on read error.
 */
static lv_fs_res_t sd_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br) {
  SdFile *f = static_cast<SdFile*>(file_p);
  int32_t res = f->read(buf, btr);

  if (res < 0) {
    *br = 0;
    return LV_FS_RES_UNKNOWN;
  }

  *br = (uint32_t)res;
  return LV_FS_RES_OK;
}

/**
 * @brief       LVGL 'seek' callback. Moves the file read/write position.
 *
 * @param       lv_fs_drv_t *drv
 *              Pointer to the registered LVGL filesystem driver.
 *
 * @param       void *file_p
 *              SdFile pointer previously returned by sd_open().
 *
 * @param       uint32_t pos
 *              Byte offset interpreted relative to whence.
 *
 * @param       lv_fs_whence_t whence
 *              LV_FS_SEEK_SET (from start), LV_FS_SEEK_CUR (from current
 *              position), or LV_FS_SEEK_END (from end).
 *
 * @return      LV_FS_RES_OK on success, LV_FS_RES_UNKNOWN on failure.
 */
static lv_fs_res_t sd_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence) {
  SdFile *f = static_cast<SdFile*>(file_p);

  switch (whence) {
    case LV_FS_SEEK_SET:
      return f->seekSet(pos) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
    case LV_FS_SEEK_CUR:
      return f->seekCur((int32_t)pos) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
    case LV_FS_SEEK_END:
      return f->seekEnd((int32_t)pos) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;
    default:
      return LV_FS_RES_UNKNOWN;
  }
}

/**
 * @brief       LVGL 'tell' callback. Returns the current file position.
 *
 * @param       lv_fs_drv_t *drv
 *              Pointer to the registered LVGL filesystem driver.
 *
 * @param       void *file_p
 *              SdFile pointer previously returned by sd_open().
 *
 * @param       uint32_t *pos_p
 *              Set to the current byte position within the file.
 *
 * @return      LV_FS_RES_OK always.
 */
static lv_fs_res_t sd_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
  SdFile *f = static_cast<SdFile*>(file_p);
  *pos_p = f->curPosition();
  return LV_FS_RES_OK;
}

/**
 * @brief       Registers the LVGL 'S:' filesystem driver backed by SdFat.
 *
 *              Called automatically from Inkplate::begin(). Subsequent calls
 *              with a null pointer (e.g. from user sketches) are silently
 *              ignored. A static guard prevents double-registration, which
 *              would corrupt LVGL's internal driver linked list.
 *
 * @param       Inkplate *inkplate
 *              Pointer to the Inkplate instance whose SdFat volume will be
 *              used for all SD file operations. Pass nullptr to skip.
 */
void lv_fs_init_sd(Inkplate *inkplate) {
  // Ignore calls without a valid Inkplate pointer (e.g., redundant calls
  // from user sketches after begin() has already registered the driver).
  if (inkplate == nullptr) return;

  // Guard against double registration: lv_fs_drv_init() zeroes the static
  // struct in place, which would corrupt the linked-list node inserted by
  // the first call while it is still live in LVGL's driver list.
  static bool registered = false;
  if (registered) return;
  registered = true;

  static lv_fs_drv_t drv;
  lv_fs_drv_init(&drv);

  drv.letter     = 'S';
  drv.user_data  = inkplate;   // explicit volume reference, avoids global cwv()
  drv.open_cb    = sd_open;
  drv.close_cb   = sd_close;
  drv.read_cb    = sd_read;
  drv.seek_cb    = sd_seek;
  drv.tell_cb    = sd_tell;
  drv.cache_size = 32000;

  lv_fs_drv_register(&drv);
}
#endif
