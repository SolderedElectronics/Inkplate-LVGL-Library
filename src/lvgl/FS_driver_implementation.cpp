#include "FS_driver_implementation.h"
#include "Inkplate-LVGL.h"
#ifndef ARDUINO_INKPLATE2

static void * sd_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
  SdFile *f = new SdFile();
  oflag_t flags = (mode == LV_FS_MODE_WR) ? (O_WRITE | O_CREAT | O_TRUNC) : O_READ;
  bool ok = f->open(path, flags);
  
  if (!ok) { 
    delete f; 
    return NULL; 
  }
  
  // LVGL 9.4 expects file pointer to be at position 0
  f->seekSet(0);
  return f;
}

static lv_fs_res_t sd_close(lv_fs_drv_t *drv, void *file_p) {
  SdFile *f = static_cast<SdFile*>(file_p);
  f->close();
  delete f;
  return LV_FS_RES_OK;
}

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

static lv_fs_res_t sd_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
  SdFile *f = static_cast<SdFile*>(file_p);
  *pos_p = f->curPosition();
  return LV_FS_RES_OK;
}

// make sure sd_size is declared above and implemented
void lv_fs_init_sd() {
  static lv_fs_drv_t drv;              // <-- MUST be static or global for LVGL v9
  lv_fs_drv_init(&drv);

  drv.letter = 'S';
  drv.open_cb  = sd_open;
  drv.close_cb = sd_close;
  drv.read_cb  = sd_read;
  drv.seek_cb  = sd_seek;
  drv.tell_cb  = sd_tell;
  drv.cache_size = 0;              // optional but helpful for file reads

  lv_fs_drv_register(&drv);
}
#endif