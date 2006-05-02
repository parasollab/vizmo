// Point.h: interface for the Point class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_Point_H_)
#define _Point_H_

#include <iostream>
#include <iterator>
#include <math.h>
#include "Vector.h"

namespace mathtool
{

    /*3D Point.*/
    template<class T, int D=3>
    class Point 
    {
    public:

        typedef vector<T> VT;

        ///////////////////////////////////////////////////////////////////////////////////
        //
        //  Constructors
        //
        ///////////////////////////////////////////////////////////////////////////////////

        Point(const T& x=0,const T& y=0,const T& z=0, const T& w=0):v(D,0)
        { 
            set(x,y,z,w); 
        }
        Point( const Point & other ) { *this = other; }

        ///////////////////////////////////////////////////////////////////////////////////
        //
        //  Operators
        //
        ///////////////////////////////////////////////////////////////////////////////////
        const Point & operator=( const Point & other ) {
            v=other.v;
            return *this;
        }

        bool operator==( const Point & other ) const {
            for( int i=0;i<D;i++ )
                if( fabs(other.v[i]-v[i])>SMALLNUMBER ) return false;
            return true;
        }

        bool almost_equ( const Point & other ) const {
            for( int i=0;i<D;i++ )
                if( fabs(other.v[i]-v[i])>1e-1 ) return false;
            return true;
        }

        bool operator!=( const Point & other ) const {
            return !(*this==other);
        }

        Vector<T,D> operator-(const Point & other) const {
            Vector<T,D> v1(v); 
            Vector<T,D> v2(other.v);
            return v1-v2;
        }

        Point operator+(const Vector<T,D> & vect) const {
            Point newp;
            for( int i=0;i<D;i++ )
                newp.v[i]=v[i]+vect[i];
            return newp;
        }

        ///////////////////////////////////////////////////////////////////////////////////
        //
        //  Access
        //
        ///////////////////////////////////////////////////////////////////////////////////
        
        void set(const T& x=0,const T& y=0,const T& z=0, const T& w=0) {
            if( D>0 ) v[0]=x; if( D>1 ) v[1]=y;
            if( D>2 ) v[2]=z; if( D>3 ) v[3]=w;
            for(int i=4;i<D;i++) v[i]=0;
        }

        void set(const T other[D]) {
            copy(&other[0],&other[D],v.begin());
        }

        void get(T other[D]) const { copy(v.begin(),v.end(),other); }
        void get(VT & other) const { other=v; }
        
        T& operator[]( int i ){ return v[i]; }
        const T& operator[]( int i ) const{ return v[i]; }

    private:
        VT v;
    };

    template<class T, int D>
    ostream & operator<<(ostream & out, const Point<T,D> & point) {
        //copy(point.v.begin(),point.v.end(),ostream_iterator<T>(out,", "));
    for( int d=0;d<D;d++ )
        out<<point[d]<<" ";
        return out;
    }

    template<class T, int D>
    istream & operator>>(istream & in, Point<T,D> & point) {
        double x,y,z;
        in>>x>>y>>z;
        point.set(x,y,z);

        return in;
    }

    /* Typedef common used vector type */
    typedef Point<double,3> Point3d;

}//end off nprmlib namespace

#endif // !defined(_Point_H_)
