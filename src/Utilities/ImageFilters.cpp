#include "ImageFilters.h"

//Change the string filter to an extension
string
FilterToExt(const string& _filter){
  typedef QStringList::iterator SIT;
  for(SIT sit = imageExtensions.begin(); sit != imageExtensions.end(); ++sit)
    if(_filter.find(sit->toStdString()) != string::npos)
      return "." + sit->toStdString();
  return "";
}

//determine if this filename has an extension
bool
HasExtension(const string& _filename){
  typedef QStringList::iterator SIT;
  for(SIT sit = imageExtensions.begin(); sit != imageExtensions.end(); ++sit){
    size_t pos = _filename.rfind(".");
    if(pos != string::npos && _filename.substr(pos+1) == sit->toStdString())
      return true;
  }
  return false;
}

//create a QString for a filename from _filename and _filter determined from a
//QFileDialog
QString
GrabFilename(const QString& _filename, const QString& _filter){
  string filename = _filename.toStdString();
  string filter = _filter.toStdString();
  if(HasExtension(filename))
    return filename.c_str();
  else
    return (filename + FilterToExt(filter)).c_str();
}

