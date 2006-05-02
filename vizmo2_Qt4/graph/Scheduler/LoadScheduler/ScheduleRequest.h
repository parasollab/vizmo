class ScheduleRequest
{
private:

	int targetpid;	//The target processor to which the data must be sent
	double fraction; //Fraction of local data that must be sent to target processor
    int originlevel;//Level at which this scheduling request originated from
	int level;	//Level to which this scheduling request must be sent out.... It should be satisfied by processors in this level


public:

	ScheduleRequest(int t,double d,int o,int L)
	{
		targetpid = t;
		fraction = d;
		originlevel = o;
		level = L;

	}
 	void define_type(stapl::typer &t)
	{
		t.local(targetpid);
		t.local(fraction);
		t.local(originlevel);
		t.local(level);

  	}

	int getTargetPid(){	return targetpid; }
	double getFraction(){	return fraction; }
	int getOriginLevel(){   return originlevel; }
	int getLevel(){	return level; }



	void Print()
	{
		cout<<"In Thread "<<stapl::get_thread_id()<<" SchReq:Tgt = "<<targetpid<<"\tFrac="<<fraction<<"\tOrLvl= "<<originlevel<<"\tLevel= "<<level<<endl;
	}

};



class TaskAvailabilityToken
{
private:

        int replyingprocessor;
	int level;
        int originlevel;
	double fraction;
	double weight;
public:

        TaskAvailabilityToken(int r,int L,int o,double f,double w)
	{
	        replyingprocessor = r;
		level = L;
	        originlevel = o;
		fraction = f;
		weight = w;
	}


        void define_type(stapl::typer &t)
	{
	        t.local(replyingprocessor);
		t.local(level);
                t.local(originlevel);
		t.local(fraction);
		t.local(weight);
	}

	int getReplyingProcessor(){       return replyingprocessor; }
	int getLevel()          {       return level;   }
	int getOriginLevel() {       return originlevel;        }
	double getFraction() {          return fraction;        }
	double getWeight()   {          return weight;      }


	void Print()
	{
	        cout<<"In Thread "<<stapl::get_thread_id()<<" [TskAvlTkn]  RplyProc="<<replyingprocessor<<"\tLevel="<<level<<"\toriginlevel= "<<originlevel<<"\tfraction="<<fraction<<"\tWeight ="<<weight<<endl;
	}

};
