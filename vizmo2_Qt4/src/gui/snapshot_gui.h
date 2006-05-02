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
#include <q3mainwindow.h>
#include <qnamespace.h>
#include <qdialog.h>
#include <q3toolbar.h>
//Added by qt3to4:
#include <QLabel>

class Q3ToolBar;
class QAction;
class MovieSaveDialog;
class QLineEdit;
class QString;
class Q3FileDialog;
class QLabel;
class QToolButton;

#include <vector>
#include <string>
using namespace std;

class gliPickBox;

class VizmoScreenShotGUI : public Q3ToolBar
{
    Q_OBJECT
        
public:
    VizmoScreenShotGUI(Q3MainWindow *,char * );
    ~VizmoScreenShotGUI();
    void reset(); //reset every thing   

public slots:
    void takeSnapshot();
    void takeBoxSnapshot(); 
    void takeMoviePictures();

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
    void addFilters(Q3FileDialog * );
    
private:    
    
    QToolButton * animation;
    QToolButton * takeBoxPicture;
    //QToolButton * takePicture;

    bool boxPicture;
    MovieSaveDialog *mDialog;
    
    // QToolButton * takePicture set as public because I couldn't compile,
    //this compiler didn't complain before with this instruction!
public:

    QToolButton * takePicture;

};


class MovieSaveDialog : public QDialog
{
    Q_OBJECT
        
public:
    MovieSaveDialog(QWidget *parent, const char *name, Qt::WFlags f=0);
    //MovieSaveDialog(QWidget *parent, Qt::WFlags f=0);
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
	int frame_digit;
};
