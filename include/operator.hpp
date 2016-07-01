#ifndef OPERATOR_HPP
#define OPERATOR_HPP
#include <functional>
#include <vector>
#include <assert.h>


template< typename T>
std::vector< std::vector<T> > one_point_homogeneous_crossover( const std::vector<T>& u , const std::vector<T>& v ) {
    std::vector< std::vector<T> > vv (2) ;

    if ( u.size() > 1 && v.size() > 1) {
        int pt = rand() % ( v.size() - 1 ) ;
        // vv0[0,pt] <- u[0,pt]
        vv[0].insert(vv[0].end(),u.begin(),u.begin()+pt) ;
        // vv0[pt,N] <- v[pt,N]
        vv[0].insert(vv[0].end(),v.begin()+pt,v.end()) ;
        // vv1[0,pt] <- v[0,pt]
        vv[1].insert(vv[1].end(),v.begin(),v.begin()+pt) ;
        // vv0[pt,N] <- v[pt,N]
        vv[1].insert(vv[1].end(),u.begin()+pt,u.end()) ;
    } else {
        vv[0] = v ;
        vv[1] = u ;
    }
    assert(u.size() == v.size()) ;
    return vv ;
}

template< typename T>
std::vector< std::vector<T> > one_point_heterogeneous_crossover( const std::vector<T>& u , const std::vector<T>& v ) {
    std::vector< std::vector<T> > vv (2) ;
    if ( u.size() > 1 && v.size() > 1 ) {
        int pt = rand() % (u.size() -1) ;
        int ptt = rand() % (v.size() -1)  ;
        // vv0[0,pt] <- u[0,pt]
        vv[0].insert(vv[0].end(),u.begin(),u.begin()+pt) ;
        // vv1[0,ptt] <- v[0,ptt]
        vv[1].insert(vv[1].end(),v.begin(),v.begin()+ptt) ;
        // vv0[pt,N] <- v[ptt,N]
        vv[0].insert(vv[0].end(),v.begin()+ptt,v.end()) ;
        // vv1[ptt,N] <- u[pt,N]
        vv[1].insert(vv[1].end(),u.begin()+pt,u.end()) ;
    } else {
        vv[0] = v ;
        vv[1] = u ;
    }
    return vv ;
}


template < typename Genome >
struct OperatorFactory {

    typedef Genome gen_t ;
    typedef int key_t ;
    typedef std::function<Genome(const Genome&)> cross_t ;
    typedef std::function<Genome(const Genome&,const Genome&)> mut_t ;

    std::vector<Genome> crossover ( const Genome & g1 ,const Genome & g2 ) { }
    Genome mutate ( const Genome & g1 ) { }

    std::vector< std::pair<float,std::function<Genome(const Genome &)> > > mMutators ;
    std::vector< std::pair<float,std::function<std::vector<Genome>(const Genome & , const Genome &)> > > mCrossers ;

    template< typename T >
    OperatorFactory<T> operator()( float , cross_t ) {
        return *this ;
    }

    template< typename T >
    OperatorFactory<T> operator()( float , mut_t ) {
        return *this ;
    }
};




#endif // OPERATOR_HPP
