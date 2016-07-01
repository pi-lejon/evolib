#ifndef GP_HPP
#define GP_HPP
#include <vector>
#include <list>
#include <iostream>
#include "assert.h"
#include "tree.hpp"
#include "operator.hpp"
#include "cloneable.hpp"
#include <numeric>


struct __GPExpression__ {
    virtual float evaluate() = 0 ; // { return 0.f ; }
};

static const std::string ops[] = { "+" , "-" , "/" , "*" } ;
static const int vars[] = { 0,30000 } ;


template< typename Primitives >
struct GPTree : public __Tree__<Primitives>, public __GPExpression__ {

    typedef typename __Tree__<Primitives>::key_t key_t ;

    GPTree()
        :__Tree__<Primitives>() {
    }

    GPTree(__Tree__<Primitives> const & t)
        :__Tree__<Primitives>(t) {
    }


/*    float eval(std::shared_ptr<Node<Primitives>> r ) {

        if ( r ) {
      //      std::cout << "let us go " << "\"" << r->data()<< "\""  << std::endl ;
            if( r->data() == ops[0] ) { // +
                return eval(r->nth_child(0)) + eval(r->nth_child(1)) ;
            }
            else if( r->data() == ops[1] ) { // -
                return eval(r->nth_child(0)) - eval(r->nth_child(1)) ;
            }
            else if( r->data() == ops[2] ) { // /
                return eval(r->nth_child(0)) / eval(r->nth_child(1)) ;
            }
            else if( r->data() == ops[3] ) { // *
                return eval(r->nth_child(0)) * eval(r->nth_child(1)) ;
            } else {
                return std::stof(r->data());
            }
        } else {
            std::cout << "bad point" << std::endl ;
            return std::numeric_limits<float>::max() ;
        }
    }*/

    float evaluate() {
       // std::cout << " launch eval " << this->mRoot << std::endl ;
        float exp = 12.5; //eval(this->mRoot) ;
        //std::cout << "eval : " << exp << std::endl;
        float dst = ( ( std::abs(2252568.f - exp) * std::abs(2252568.f - exp)  * std::abs(2252568.f - exp) ) * this->size() )  ;
        //std::cout << "Note = " << dst << std::endl ;
        return dst ;
    }

    void random() {

        int maxNodes = 15 ;
        int nodes = rand() % maxNodes + 1 ;

        std::deque<typename __Tree__<Primitives>::key_t> todo ;
        todo.push_back( set_root(ops[rand()%4]) ) ;

        while ( nodes > 0 && todo.size() != 0 ) {
            int n = ( rand() % 2 ) * 2 ;
            auto nd = todo.front() ;
            if( n == 0 ) {
                auto t = get(nd);
                int val = (rand()%vars[1])+vars[0] ;
                t->set(std::to_string(val));
            } else for ( int i = 0 ; i < 2 ; i++ ) {
                auto t = append(nd,ops[rand()%4],i) ;
                todo.push_back(t);
                nodes-- ;
            }
            todo.pop_front();
        }
        while( todo.size() != 0 ) {
            auto nd = todo.front() ;
            get(nd)->set(std::to_string((rand()%vars[1])+vars[0]));
            todo.pop_front();
        }
    }

    key_t rdm_key() {
        auto k = keys() ;
        return k[rand()%k.size()] ;
    }
};

template< typename T >
GPTree<T> mutate(const GPTree<T> & c) {
    GPTree<T> v  = c ;
    auto t = v.rdm_key() ;
    auto n = v.get(t) ;
    if ( n->is_terminal() ) {
        int mut = rand() % 5 ;
        if (  mut > 0 ) {
//            std::cout << "terminal switch " << std::endl ;
             n->set(std::to_string((rand()%vars[1])+vars[0])) ;
             assert(n->is_terminal()) ;
        } else if ( mut == 0){
//            std::cout << "terminal to unterm " << std::endl ;
            v.append(t,std::to_string((rand()%vars[1])+vars[0]),0) ;
            v.append(t,std::to_string((rand()%vars[1])+vars[0]),1) ;
            n->set(ops[rand()%4]) ;
            assert(!n->is_terminal()) ;
        }

    } else {
        int mut = rand() % 5 ;
        if (  mut == 0 ) {
//            std::cout << " change operators " << std::endl ;
            n->set(ops[rand()%4]) ;
        } else if ( mut > 0 ){
//            std::cout << "delete branch" << std::endl ;
            n->set_nth(0,nullptr) ;
            n->set_nth(1,nullptr) ;
            n->set(std::to_string((rand()%vars[1])+vars[0])) ;
            assert(n->is_terminal()) ;

        }
    }
    return v ;
}

template< typename T >
std::vector<__Tree__<T>> one_point_crossover ( const __Tree__<T>& c1 , const __Tree__<T>& c2 ) {

    std::vector<typename __Tree__<T>::key_t> k1 = c1.keys() ;
    std::vector<typename __Tree__<T>::key_t> k2 = c2.keys() ;


    auto pt1 = k1[rand()%k1.size()] ; // key of C1
    auto pt2 = k2[rand()%k2.size()] ; //key of C2

   assert(pt1.rank() < 2 && pt1.rank() >= 0) ;
   assert(pt2.rank() < 2 && pt2.rank() >= 0 ) ;

    std::vector<__Tree__<T>> g(2) ;
    g[0] = c1 ;
    g[1] = c2 ;

    auto of = g[0].cut(pt1) ;
    auto off = g[1].cut(pt2) ;

    g[0].add(pt1.father(),off,pt1.rank()) ;
    g[1].add(pt2.father(),of,pt2.rank()) ;

    return g ;

}

template< typename T >
std::vector<GPTree<T>> one_point_crossover ( const GPTree<T>& c1 , const GPTree<T>& c2 ) {

    std::vector<typename GPTree<T>::key_t> k1 = c1.keys() ;
    std::vector<typename GPTree<T>::key_t> k2 = c2.keys() ;


    auto pt1 = k1[rand()%k1.size()] ; // key of C1
    auto pt2 = k2[rand()%k2.size()] ; //key of C2

   assert(pt1.rank() < 2 && pt1.rank() >= 0) ;
   assert(pt2.rank() < 2 && pt2.rank() >= 0 ) ;

    std::vector<GPTree<T>> g(2) ;
    g[0] = c1 ;
    g[1] = c2 ;

    auto of = g[0].cut(pt1) ;
    auto off = g[1].cut(pt2) ;

    g[0].add(pt1.father(),off,pt1.rank()) ;
    g[1].add(pt2.father(),of,pt2.rank()) ;

    return g ;

}

// sequence of instruction
template< typename Instructions >
struct GPSequence  : public __GPExpression__ {
    GPSequence()
        :mInstructions() {}

    GPSequence(const GPSequence & c)
        :mInstructions(c.mInstructions) {
    }


    std::vector<Instructions> mInstructions ;

    float evaluate() {
        return 0 ;
    }

    bool operator==(const GPSequence& other ) {
        return ( mInstructions == other.mInstructions ) ;
    }
};

template< typename Vertex, typename Edge >
struct GPValuedGraph : public __GPExpression__ {

    ValuedGraph<Vertex,Edge> mGraph ;

    void random() {
        int numV = 1 + rand() % 5 ;
        int numE = 1 + rand() % 15 ;

        for ( int i = 0 ; i < numV ; i++ )
            mGraph.add_vertex(std::to_string(i)) ;

        for ( int i = 0 ; i < numE ; i ++ )
            mGraph.add_edge(rand()%numV,rand()%numV,3) ;
    }

    void delete_orphans() {
        std::cout << "Delete orphans" << std::endl ;
        int n = mGraph.mEdges.size() ;
        if ( n > 1 ) {
            std::vector<int> orphans ;
            for ( int i = 1 ; i < n ; i++ )
                orphans.push_back(i);

            for ( int i = 0 ; i < n ; i++ ) {
                for ( int j = 0 ; j < mGraph.mEdges[i].size() ; j++ ) {
                    orphans.erase(std::find(orphans.begin(),orphans.end(),mGraph.mEdges[i][j].first)) ;
                }
            }

            std::cout << "start deletion of " << orphans.size() << std::endl ;

            for ( int i = 0 ; i < orphans.size() ; i++ ) {
                mGraph.mVertices.erase( mGraph.mVertices.begin() + orphans[i] ) ;
            }
        }
        std::cout << "end of deletion " << std::endl;
    }

    float evaluate() { return 0.f ; }
};

/*
// Operators on these data structure ( crossover & mutation )
template< typename Mutator >
struct MutationFactory {
    std::vector<std::pair<float,Mutator>> mMutators ;
    void mutate() {

    }

    MutationFactory& operator()(float prob,Mutator m) {
        //
    }

     *  use :
     *  MutationFactory mf ;
     *  mf(0.05f,DELETE_NODE_MUTATION)
     *    (0.25f,ADD_EDGE_MUTATION )
     *    (0.10f,FLIP_EDGE_MUTATION ) ;
     *

};

*/

/*
enum SBGPStructures { GPGRAPH,
                      GPTREE,
                      GPSEQUENCE
};


enum CrossoverOperators { ONE_POINT_HOMOGENEOUS_CROSSOVER = 0x01,
                          ONE_POINT_HETEROGENEOUS_CROSSOVER = 0x10,
                          TWO_POINT_HOMOGENEOUS_CROSSOVER = 0x100,
                          TWO_POINT_HETEROGENEOUS_CROSSOVER = 0x1000 } ;

enum MutationOperators { DELETE_NODE_MUTATION = 0x01,
                         ADD_EDGE_MUTATION = 0x10,
                         FLIP_EDGE_MUTATION = 0x100,
                         DELETE_EDGE_MUTATION = 0x1000 };

struct OperatorsTable {

  //  std::map<CrossoverOperators,std::function<void()>> operators ;


    void registerr(int a) {
        if( a  &  ONE_POINT_HOMOGENEOUS_CROSSOVER ) {
            std::cout << "one point homogeneous" << std::endl;
        }
        if( a  &  ONE_POINT_HETEROGENEOUS_CROSSOVER ) {
            std::cout << "one point heterogeneous" << std::endl;
        }
        if( a  &  TWO_POINT_HOMOGENEOUS_CROSSOVER ) {
            std::cout << "two point homogeneous" << std::endl;
        }
    }
};
*/
//std::vector<__GPExpression__> crossover( __GPExpression__& g1,__GPExpression__& g2, CrossoverOperators op ) {
//    return OperatorsMap[op](g1,g2) ;
//}

template<typename T, typename U>
GPValuedGraph<T,U> mutate ( const GPValuedGraph<T,U> & gen ) {
    return gen ;
}


template < typename T , typename U>
std::vector<GPValuedGraph<T,U>> one_point_crossover ( const GPValuedGraph<T,U>& c1 , const GPValuedGraph<T,U>& c2 ) {
    std::vector<GPValuedGraph<T,U>> offsprings ;
    offsprings.push_back(c1);
    offsprings.push_back(c2);
    if( c1.mGraph.vrt_size() > 1 && c2.mGraph.vrt_size() > 1 ) {

        GPValuedGraph<T,U> s1,s2 ;

        // choose crossover point
        int fpt =  1 + rand() % (c1.mGraph.vrt_size()) ;
        int fptt =  1 + ( ( ( float ) rand()/ ( float ) RAND_MAX )* c2.mGraph.vrt_size() ) ;

        // s1[0;fpt] <<-- c1[0;fpt]
        s1.mGraph.mVertices.insert(s1.mGraph.mVertices.end(),c1.mGraph.mVertices.begin(),c1.mGraph.mVertices.begin()+fpt) ;
        s1.mGraph.mEdges.insert(s1.mGraph.mEdges.end(),c1.mGraph.mEdges.begin(),c1.mGraph.mEdges.begin()+fpt) ;

        // s2[0;fptt] <<-- c2[0;fptt]
        s2.mGraph.mVertices.insert(s2.mGraph.mVertices.end(),c2.mGraph.mVertices.begin(),c2.mGraph.mVertices.begin()+fptt) ;
        s2.mGraph.mEdges.insert(s2.mGraph.mEdges.end(),c2.mGraph.mEdges.begin(),c2.mGraph.mEdges.begin()+fptt) ;

        // s1[fpt;N] <<-- c2[fptt;M]
        s1.mGraph.mVertices.insert(s1.mGraph.mVertices.end(),c2.mGraph.mVertices.begin()+fptt,c2.mGraph.mVertices.end()) ;
        s1.mGraph.mEdges.insert(s1.mGraph.mEdges.end(),c2.mGraph.mEdges.begin()+fptt,c2.mGraph.mEdges.end()) ;

        // s2[fpt;N] <<-- c1[fpt;M]
        s2.mGraph.mVertices.insert(s2.mGraph.mVertices.end(),c1.mGraph.mVertices.begin()+fpt,c1.mGraph.mVertices.end()) ;
        s2.mGraph.mEdges.insert(s2.mGraph.mEdges.end(),c1.mGraph.mEdges.begin()+fpt,c1.mGraph.mEdges.end()) ;

        assert(s1.mGraph.mVertices.size()==s1.mGraph.mEdges.size());
        assert(s2.mGraph.mVertices.size()==s2.mGraph.mEdges.size());

        // check edge integrity
        int size = s1.mGraph.mVertices.size() ;
        for ( int i = 0 ; i < size ; i++ ) {
            int t = 0 ;
            for ( int j = s1.mGraph.mEdges[i].size() - 1 ; j >= 0 ; j-- ) {
                if( s1.mGraph.mEdges[i][j].first >= size )
                    std::swap(s1.mGraph.mEdges[i][j],s1.mGraph.mEdges[i][s1.mGraph.mEdges[i].size() - 1 - t++]) ;
            }
            s1.mGraph.mEdges[i].resize(s1.mGraph.mEdges[i].size()-t);
        }

        size = s2.mGraph.mVertices.size() ;
        for ( int i = 0 ; i < size ; i++ ) {
            int t = 0 ;
            for ( int j = s2.mGraph.mEdges[i].size() - 1 ; j >= 0 ; j-- ) {
                if( s2.mGraph.mEdges[i][j].first >= size )
                    std::swap(s2.mGraph.mEdges[i][j],s2.mGraph.mEdges[i][s2.mGraph.mEdges[i].size() - 1 - t++]) ;
            }
            s2.mGraph.mEdges[i].resize(s2.mGraph.mEdges[i].size()-t);
        }

        assert(c1.mGraph.vrt_size() > 0 ) ;
        assert(c2.mGraph.vrt_size() > 0 ) ;

        offsprings.push_back(s1);
        offsprings.push_back(s2);
    }
    return offsprings ;
}

template< typename T>
std::vector<GPSequence<T>> one_point_crossover (const GPSequence<T>& s1 , const GPSequence<T>& s2) {
    std::vector<GPSequence<T>> offsprings(2) ;
    offsprings.push_back(s1);
    offsprings.push_back(s2);
    auto ww = one_point_heterogeneous_crossover(s1.mInstructions,s2.mInstructions) ;
    offsprings[0].mInstructions = ww[0] ;
    offsprings[1].mInstructions = ww[1] ;

    return offsprings ;
}

#endif // GP_HPP
