// GUI for screen capture and movie

#include "vizmo2.h"
//#include "main_win.h"
/** 
* This class deifines the animation GUI for vizmo2
*
*/



#ifdef WIN32
#pragma warning(disable : 4786)
#endif

///////////////////////////////////////////////////////////////////////////////// QT Headhers
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qdialog.h>
#include <qtoolbar.h>

class QToolBar;
class QAction;
class MovieSaveDialog;
class QLineEdit;
class QString;
class QFileDialog;
class QLabel;
class QToolButton;

#include <vector>
#include <string>
using namespace std;

class gliPickBox;

class VizmoScreenShotGUI : public QToolBar
{
    Q_OBJECT
        
public:
    VizmoScreenShotGUI(QMainWindow *,char * );
    ~VizmoScreenShotGUI();
    void reset(); //reset every thing   

public slots:
    void takeSnapshot();
    void takeBoxSnapshot(); 
    void takeMoviePictures();
    void takeCampusSnapshot(char *,char *);

signals:
    void getScreenSize(int *w,int *h);
    void getBoxDimensions(int *xOffet,int *yOffset,int *w,int *h);
    void togleSelectionSignal();
    void simulateMouseUp();
    void callUpdate();
    void goToFrame(int);    
    
private:
    void CreateActions();
    bool CreateGUI();
    void addFilters(QFileDialog * );
    
private:    
    
    QToolButton * animation;
    QToolButton * takeBoxPicture;
    QToolButton * takePicture;

    bool boxPicture;
    MovieSaveDialog *mDialog;
};


class MovieSaveDialog : public QDialog
{
    Q_OBJECT
        
public:
    MovieSaveDialog(QWidget *parent, const char *name, WFlags f=0);
    void updateAttributes();
    
private slots:
    
    void saveImages();
    void showFileDialog();
    
private:
    void storeAttributes();

    QLineEdit *startFrame;
    QLineEdit *endFrame;
    QLineEdit *stepSize;
    QLabel *fnameLabel;
    
public:
    
    int startIntFrame;
    int endIntFrame;
    int stepIntSize;
    QString sFileName;
	QString sFileExt;
};
