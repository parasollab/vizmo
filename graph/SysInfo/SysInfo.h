#ifndef SYSINFO_H
#define SYSINFO_H

// local includes

#include "Graph.h"

// global includes

#include <map>
#include <list>

class CommInfo {
    
public:  

    // local variables
    
    double latency;		// Overhead incurred using this channel in microseconds.
    double bandwidth;		// Bandwidth in nanoseconds (seconds per megabit).

    // constructors
    
    CommInfo() { }
    
    CommInfo( double _latency, double _bandwidth ) {
        latency = _latency;
        bandwidth = _bandwidth;
    }

    // destructors
    
    ~CommInfo( ) { }
    
    // set and get methods
    
    void setLatency( double _latency ) { latency = _latency; }
    void setBandwidth( double _bandwidth )  { bandwidth = _bandwidth; }
    
    double getLatency( ) { return latency; }
    double getBandwidth( ) { return bandwidth; }

};

inline bool operator== ( const CommInfo& x, const CommInfo& y ) {
    return ( x.latency == y.latency && x.bandwidth == y.bandwidth);
};

inline istream& operator>> ( istream& s, CommInfo& x ) {
    s >> x.latency >> x.bandwidth;
    return s;
};

inline ostream& operator<< ( ostream& s, const CommInfo& x ) {
    s << x.latency << "," << x.bandwidth;
    return s;
};

class ProcInfo {

public:

    // local variables

    int frequency;		// Processor frequency in MHz.
    int flops;			// Number of floating point operations per cycle.

    // constructors
    
    ProcInfo( ) {
        frequency = 0;
        flops = 0;
    }
    
    ProcInfo( int _frequency, int _flops ) {
        frequency = _frequency;
        flops = _flops;
    }

    // destructors
    
    ~ProcInfo( ) { }
    
    // set and get methods
    
    void setFrequency( int _frequency ) { frequency = _frequency; }
    void setFlops( int _flops ) { flops = _flops; }

    int getFrequency( ) { return frequency; }
    int getFlops( ) { return flops; }
    
};

inline bool operator== ( const ProcInfo& x, const ProcInfo& y ) {
    return ( x.frequency == y.frequency && x.flops == y.flops);
};

inline istream& operator>> ( istream& s, ProcInfo& x ) {
    s >> x.frequency >> x.flops;
    return s;
};

inline ostream& operator<< ( ostream& s, const ProcInfo& x ) {
    s << x.frequency << "," << x.flops;
    return s;
};

template<class VERTEX, class WEIGHT=int>
class SysInfo : public Graph<UG<VERTEX,WEIGHT>,NMG<VERTEX,WEIGHT>,WG<VERTEX,WEIGHT>,VERTEX,WEIGHT> {
    
public:

    VID vid;			// The current vid for inserting new processors.
    int numProcs;		// The total number of processors in the system.
    int numClusters;		// The total number of clusters in the system.
    map<int,vector<VID> > cluster;	// What processors are clustered together?
    
    // constructors
    
    SysInfo( ) {
        // Initialize the number of processors and clusters to be 0.
        numProcs = 0;
        numClusters = 0;
    }
    
    SysInfo( int numNodes, VERTEX& v, WEIGHT& w )  {
        // Initialize the number of processors and clusters to be 0.
        numProcs = 0;
        numClusters = 0;
        // Create the inital cluster of nodes.
        generateCluster( numNodes, v, w );
    } 
    
    // Add a vertex and return it's VID.
    VID addVertex( VERTEX& v ) {
        // Increment the number of processors.
        numProcs++;
        // Add the vertex and increment the number of processors.
        return AddVertex( v, vid++ );
    }

    VID addVertex( VERTEX& v, bool flag ) {
        // If TRUE use the default method
        if ( flag ) return addVertex( v );
        // Else add the vertex without incrementing numProcs
        else return AddVertex( v, vid++ );
    }
    
    // Add an edge to the graph
    void addEdge( VID& v1, VID& v2, WEIGHT& e ) {
        AddEdge( v1, v2, e );
    }

    // Get the number of clusters.
    int getNumClusters( ) {
        return numClusters;
    }
    
    // Get the total number of processors.
    int getNumProcs( ) {
        return numProcs;
    }

    // Get a specific vertex
    bool getVertex( VID v0, VERTEX& v ) {
        return true;
    }

    // Set a specific vertex
    bool setVertex( VID v0, VERTEX& v ) {
        return true;
    }

    // Get a specific edge
    bool getWeight( VID v0, VID v1, WEIGHT& e ) {
        return true;
    }

    // Set a specific processor
    bool setWeight( VID v0, VID v1, WEIGHT& e ) {
        return true;
    }
    
    /*
     Make a completely connected node with identical computation and
     communication capabilities (i.e. an SMP-node).
     */
    VID generateCluster( int n, VERTEX& v, WEIGHT& w ) {
        VERTEX v0;
        VID v1 = addVertex( v0, FALSE );
        for ( int i = 0; i < n; i++ ) {
            VID v2 = addVertex( v );
            addEdge( v1, v2, w );
        }
        // Increment the total number of clusters
        numClusters++;
        // Return the cluster id
        return v1;
    }

    VID generateCluster( vector<VID> &v, WEIGHT& w ) {
        VERTEX v0;
        VID v1 = addVertex( v0, FALSE );
        // Make edges from each proc in the cluster to the dummy
        for ( unsigned int i = 0; i < v.size(); i++ )
            addEdge( v1, v[i], w );
        // Increment the total number of clusters
        numClusters++;
        // Return the cluster id
        return v1;
    }
     
};

inline bool operator== ( const SysInfo<ProcInfo,CommInfo>& x, const SysInfo<ProcInfo,CommInfo>& y ) {
    return true;
};

inline ostream& operator<< ( ostream& s, const SysInfo<ProcInfo,CommInfo>& x ) {
    x.WriteGraph( s );
    return s;
};

#endif // SYSINFO_H
