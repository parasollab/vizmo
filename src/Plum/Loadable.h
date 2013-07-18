///////////////////////////////////////////////////////////////////////////////////////////
// Loadable is an interface file for all model loader

#ifndef LOADABLE_H_
#define LOADABLE_H_

#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

//#define MAX_LINE_LENGTH 2000

namespace plum{

  class Loadable {
    public:
      Loadable() : m_filename("") {m_color[0] = m_color[1] = m_color[2] = -1;}
      virtual ~Loadable(){}

      string GetFilename() const {return m_filename;}
      void SetFilename(const string& _filename) {m_filename=_filename;}

      /**
       * Parse given file.
       */
      virtual bool ParseFile()=0;

      /**
       * Check if given file name is valid
       */
      bool CheckCurrentStatus() const {
        //Check if file exist
        ifstream fin(m_filename.c_str());
        if(!fin.good()) {
          cerr<<"File ("<< m_filename <<") not found";
          return false;
        }
        return true;
      }

    protected:

      /**
       * discard all commented lines util the next uncommented line is found
       */
      static void GoToNext(istream& _in) {
        string line;
        while(!_in.eof()) {
          char c;
          while(isspace(_in.peek()))
            _in.get(c);

          c = _in.peek();
          if(!IsCommentLine(c))
            return;
          else
            getline(_in, line);
        }
      }

      /**
       * is given char a start of command line?
       * comments denoted with # character
       */
      static bool IsCommentLine(char _c) {return _c == '#';}

      /**
       * GetPathName from given filename
       */
      string GetPathName(const string& _filename) {
        size_t pos = _filename.rfind('/');
        if(pos == string::npos)
          return "";
        return _filename.substr(0, pos+1);
      }

      /**
       * get color from Env file
       */
      void GetColor(istream& _in){
        string line;
        while(!_in.eof()){
          char c;
          while(isspace(_in.peek()))
            _in.get(c);

          c = _in.peek();
          if(!IsCommentLine(c))
            return;
          else {
            getline(_in, line);
            //colors begin with VIZMO_COLOR
            if(line[7] == 'C'){
              size_t loc = line.find(" ");
              string sub = line.substr(loc+1);
              istringstream iss(sub);
              if(!(iss >> m_color[0] >> m_color[1] >>  m_color[2]))
                cerr << "Warning: Error loading vizmo color." << endl;
            }
          }
        }
      }

      template <class T>
        T ReadField(istream& _is, string _error) {
          char c;
          string line;
          T element;
          while(_is) {
            c = _is.peek();
            if(c == '#') {
              getline(_is, line);
            }
            else if(!isspace(c)) {
              if (!(_is >> element)) {
                cerr << "Error in Reading Field::" << _error << endl;
                exit(1);
              }
              else
                break;
            }
            else
              _is.get(c);
          }
          if(_is.eof()){
            cerr << "Error end of file reached in Reading Field::" << _error << endl;
            exit(1);
          }
          return element;
        };

      string ReadFieldString(istream& _is, string _error, bool _toUpper = true){
        string s = ReadField<string>(_is, _error);
        if(_toUpper)
          transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
      };

      string m_filename; //filename to be read
      float m_color[3]; //color read from env file
  };
}

#endif
