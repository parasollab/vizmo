#ifndef _hash_exceptions_h_
#define _hash_exceptions_h_

class KeyHashFcnMismatch
{
public:


	int hash_key;
	int gid;

	KeyHashFcnMismatch(int x,int y)
	{
	
		hash_key=x;
		gid=x;
	}

};

#endif
