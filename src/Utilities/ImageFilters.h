#ifndef IMAGEFILTERS_H_
#define IMAGEFILTERS_H_

#include <string>

using namespace std;

#include <QString>
#include <QStringList>

//File filters and extensions for allowable image formats.
//Determined from Write support of QImages.
inline QStringList ImageFilters() {
  QStringList filters;

  filters += "Windows Bitmap (*.bmp)";
  filters += "Joint Photographic Experts Group (*.jpg)";
  filters += "Portable Network Graphics (*.png)";
  filters += "Portable Pixmap (*.ppm)";
  filters += "Tagged Image File Format (*.tiff)";
  filters += "X11 Pixmap (*.xpm)";

  return filters;
}

inline QStringList ImageExtensions() {
  QStringList extensions;

  extensions += "bmp";
  extensions += "jpg";
  extensions += "png";
  extensions += "ppm";
  extensions += "tiff";
  extensions += "xpm";

  return extensions;
}

static QStringList imageFilters = ImageFilters();
static QStringList imageExtensions = ImageExtensions();

//Change the string filter to an extension
string FilterToExt(const string& _filter);

//determine if this filename has an extension
bool HasExtension(const string& _filename);

//create a QString for a filename from _filename and _filter determined from a
//QFileDialog
QString GrabFilename(const QString& _filename, const QString& _filter);

#endif
