// BoundingBoxModel.cpp: implementation of the CBoundingBoxModel class.
//
//////////////////////////////////////////////////////////////////////

#include "BoundingBoxModel.h"
#include <stdlib.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBoundingBoxModel::CBoundingBoxModel()
{
    m_pBBXParser=NULL;
    m_DisplayID=-1;
    m_BBX=NULL;
}

CBoundingBoxModel::~CBoundingBoxModel()
{
    m_pBBXParser=NULL;
    m_BBX=NULL;
}

//////////////////////////////////////////////////////////////////////
// GLModel Methods
//////////////////////////////////////////////////////////////////////

bool CBoundingBoxModel::BuildModels(){
    if( m_pBBXParser==NULL ) return false;
    
    m_BBX=m_pBBXParser->getBBXValue();
    
    GLdouble vertice[]=
    { m_BBX[0], m_BBX[2], m_BBX[4],
    m_BBX[1], m_BBX[2], m_BBX[4],
    m_BBX[1], m_BBX[2], m_BBX[5],
    m_BBX[0], m_BBX[2], m_BBX[5],
    m_BBX[0], m_BBX[3], m_BBX[4],
    m_BBX[1], m_BBX[3], m_BBX[4],
    m_BBX[1], m_BBX[3], m_BBX[5],
    m_BBX[0], m_BBX[3], m_BBX[5]};
    
    //Face index
    GLubyte id1[] = { 3, 2, 1, 0 }; //buttom
    GLubyte id2[] = { 4, 5, 6, 7 }; //top
    GLubyte id3[] = { 2, 6, 5, 1 }; //left
    GLubyte id4[] = { 0, 4, 7, 3 }; //right
    GLubyte id5[] = { 1, 5, 4, 0 }; //back
    GLubyte id6[] = { 7, 6, 2, 3 }; //front
    //line index
    GLubyte lineid[] = 
    { 0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7};
    
    //set properties for this box
    m_DisplayID = glGenLists(1);
    glNewList(m_DisplayID, GL_COMPILE);
    
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 2.0, 1.0 );
    //setup points
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, vertice);
    
    glColor3d( 0.95, 0.95, 0.95 );
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id1 );
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id2 );
    
    glColor3d( 0.85, 0.85, 0.85 );
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id3 );
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id4 );
    
    glColor3d( 0.75, 0.75, 0.75 );
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id5 );
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id6 );
    glDisable( GL_POLYGON_OFFSET_FILL );
    
    //Draw lines
    glLineWidth(1);
    glColor3d( 0.5, 0.5, 0.5 );
    glDrawElements( GL_LINES, 24, GL_UNSIGNED_BYTE, lineid );
    glEndList();

    glDisableClientState(GL_VERTEX_ARRAY);
    
    return true;
}

void CBoundingBoxModel::Draw( GLenum mode ){

    if( mode==GL_RENDER ){
        if(m_RenderMode==CPlumState::MV_INVISIBLE_MODE) return;
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //Draw bbx
        if( m_DisplayID!=-1 ){
            glPolygonMode( GL_FRONT, GL_FILL );
            glDisable(GL_LIGHTING);
            glCallList(m_DisplayID);
            glEnable(GL_LIGHTING);
        }
		glDisable(GL_CULL_FACE);
    }
}

////////////////////////////////////////////////////////
// find out the biggest Z coord. to set up the camera
// 07/2002 Aim'ee
///////////////////////////////////////////////////////

double CBoundingBoxModel:: minVal(double x, double y){
    if (x > y)
        return y;
    else
        return x;
}

double CBoundingBoxModel::maxVal(double x, double y){
    if(x < y)
        return y;
    else
        return x;
}

double CBoundingBoxModel::returnMax(){
    
    double  maxR = 0;
    double maxX, maxY, maxZ;
    maxX = maxY = maxZ =0;
    double minX, minY, minZ;
    minX = minY = minZ = 1000;
    
    maxX = maxVal(maxX, m_BBX[0]);
    maxX = maxVal(maxX, m_BBX[1]);
    
    maxY = maxVal(maxY, m_BBX[2]);
    maxY = maxVal(maxY, m_BBX[3]);
    
    maxZ = maxVal(maxZ , m_BBX[4]);
    maxZ = maxVal(maxZ , m_BBX[5]);
    
    minX = minVal(minX, m_BBX[0]);
    minX = minVal(minX, m_BBX[1]);
    
    minY = minVal(minY, m_BBX[2]);
    minY = minVal(minY, m_BBX[3]);
    
    minZ = minVal(minZ, m_BBX[4]);
    minZ = minVal(minZ, m_BBX[5]);
    
    double x, y, z;
    x = maxX - minX;
    y = maxY - minY;
    z = maxZ - minZ;
    
    maxR = x + y + (z/2) + 30;
    // maxR = maxX + maxY + maxZ + 50;
    
    return maxR;
    
}

list<string> CBoundingBoxModel::GetInfo() const 
{ 
    list<string> info; 
	info.push_back("Bounding Box");
	info.push_back("");
    for( int i=0;i<6;i++ ){
		string name;
		switch(i){
			case 0: name="Min X="; break;
			case 1: name="Max X="; break;
			case 2: name="Min Y="; break;
			case 3: name="Max Y="; break;
			case 4: name="Min Z="; break;
			case 5: name="Max Z="; break;
		}
        ostringstream temp;
        temp<<m_BBX[i];
        info.push_back(name+temp.str());
    }   
    return info;
}
