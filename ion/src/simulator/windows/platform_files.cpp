#include "../shared/platform.h"
#include <windows.h>
#include <stdio.h>

namespace Ion {
namespace Simulator {
namespace Platform {

static OPENFILENAME * getOFN(const char * extension) {
  static OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(ofn)); // Zero-out ofn

  static char path[PATH_MAX];
  path[0] = 0; // Reset the path

  static char filter[32];
  if (snprintf(filter, sizeof(filter), "%s%c*.%s%c%c", extension, 0, extension, 0, 0) < 0) {
    /* Note: We cannot use litteral \0 in the format string, otherwise snprintf
     * will think the format string is finished... */
    return nullptr;
  }

  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = path;
  ofn.nMaxFile = sizeof(path);
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.lpstrDefExt = extension;

  return &ofn;
}

const char * filePathForReading(const char * extension) {
  OPENFILENAME * ofn = getOFN(extension);
  if (ofn != nullptr && GetOpenFileName(ofn)) {
    return ofn->lpstrFile;
  }
  return nullptr;
}

const char * filePathForWriting(const char * extension) {
  OPENFILENAME * ofn = getOFN(extension);
  if (ofn != nullptr && GetSaveFileName(ofn)) {
    return ofn->lpstrFile;
  }
  return nullptr;
}

  // Warning: not sure that works
const char * filePathInTempDir(const char * filename) {
  static char path[64];
  int written = 0;
  written = GetTempPath(sizeof(path), path);
  if(written > sizeof(path) || written == 0) {
    return nullptr;
  }
  assert(written + strlen(filename) < sizeof(path));
  strcpy(&path[written], filename);
  return path;
}

}
}
}
