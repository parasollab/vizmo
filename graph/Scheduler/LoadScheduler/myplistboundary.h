
template<class T>
class MypListBoundary
{
private:
	typename std::list<T>::iterator b;
	typename std::list<T>::iterator l; //last
	int bdry_size;
 	int ID;
public:

	int GetID()	{	return ID;	}
 	void SetID(int _id)	{	ID= _id; }

	void SetBoundary(typename std::list<T>::iterator _begin,typename std::list<T>::iterator _end,int sz)
	{
		b = _begin;
		l = --_end;

		bdry_size = sz;
	}

	void Print()
	{	cout<<"\tSize = "<<bdry_size<<" ID = "<<ID<<"    Contents:> ";
		typename std::list<T>::iterator e = l;
		e++;

		for(typename std::list<T>::iterator it=b; it!=e; ++it)
		{
			cout<<*it<<"  ";
		}
		cout<<endl;


	}



	int size()      {       return bdry_size;       };

	typename std::list<T>::iterator begin()	{ return b; }
	typename std::list<T>::iterator end()	{
							typename std::list<T>::iterator e = l;
							e++;
							return e;
	                                        }

//	void SetEnd(typename std::list<T>::iterator newend )	{ l=newend; }





};



