#ifndef TREE_HPP
#define TREE_HPP

#include <vector>
#include <iostream>
#include <deque>
#include <algorithm>
#include <memory>

template<typename U>
struct is_pointer { static const bool value = false; };

template<typename U>
struct is_pointer<std::shared_ptr<U>> { static const bool value = true; };


template< typename T >
struct Node {

    typedef typename std::vector< std::shared_ptr<Node<T>>>::iterator citerator ;

    Node(T dat):mData(dat),mChilds(0,std::shared_ptr<Node<T>>()) {}

    Node(Node const& n):mData(n.mData),mChilds(n.mChilds.size(),std::shared_ptr<Node<T>>()) {
        for ( int i = 0 ; i < n.mChilds.size() ; i++ ) {
            if( n.mChilds[i] ) {
                if( is_pointer<T>::value ) {
                    mData.reset(n.mData->clone()) ;
                }
                mChilds[i].reset(new Node<T>(*(n.mChilds[i]))) ;
            }
        }
    }

    Node(Node&& other)
        :mData(other.mData),mChilds(other.mChilds.size(),std::shared_ptr<Node<T>>())
    {
        for ( int i = 0 ; i < other.mChilds.size() ; i++ ) {
            if( other.mChilds[i] ) {
                if( is_pointer<T>::value ) {
                    mData = other.mData ;
                    other.mData.reset() ;
                }
                mChilds[i].reset(new Node<T>(*(other.mChilds[i]))) ;
                other.mChilds[i].reset() ;
            }
        }
    }

    inline int size() { return mChilds.size() ; }
    inline T data() { return mData ; }
    inline void set(T t) { mData = t ; }
    inline citerator child_begin() { return mChilds.begin(); }
    inline citerator child_end() { return mChilds.end(); }
    inline std::shared_ptr<Node<T>> nth_child(int a) { return mChilds[a]; }
    inline bool is_terminal() {
        return !std::any_of(mChilds.begin(),mChilds.end(),[](std::shared_ptr<Node<T>> t){ return t ; } ) ;
    }

    void set_nth(int a , std::shared_ptr<Node<T>> ch) {
        if( a < size() ) {
            mChilds[a] = ch ;
        } else {
            mChilds.resize(a+1,std::shared_ptr<Node<T>>()) ;
            mChilds[a] = ch ;
        }
    }

    Node<T>& operator=(Node<T> t) {
        std::swap(t.mData,this->mData);
        std::swap(t.mChilds,this->mChilds);
        return *this ;
    }

protected :

    T mData ;
    std::vector< std::shared_ptr<Node<T>> > mChilds ;

};

template< typename T >
struct __Tree__ {

    typedef Node<T> Node_t ;

    typedef struct node_position {
        std::vector<int> position ;
        inline int rank() { return position.back(); }

        inline node_position father() {
            auto pos = *this ;
            if(pos.position.size()>1)pos.position.pop_back();
            return pos ;
        }

        bool operator==(const node_position& nd) {
            return ( position == nd.position ) ;
        }
    } key_t ;

    std::shared_ptr<Node<T>> mRoot ;

    __Tree__ ()
        :mRoot(nullptr) {
    }

    __Tree__ (std::shared_ptr<Node<T>> ptr)
        :mRoot(ptr) {
    }

    __Tree__ ( __Tree__<T> const& t)
        :mRoot( ( !t.mRoot )?nullptr:(new Node_t(*(t.mRoot))) ) {
    }

    key_t root() const {
        key_t t ;
        t.position.push_back(0) ;
        return t ;
    }

    virtual key_t set_root(T data) {
        mRoot.reset(new Node_t(data)) ;
        return root() ;
    }

    void set( key_t const & it , T dat) {
        std::shared_ptr<Node<T>> r ;
        if( it.position.size() > 0 ) {
            r = mRoot ;
            for ( int i = 1 ; i < it.position.size() ; i++ ) {
                r = r->nth_child(it.position[i]) ;
                if ( !r ) {
                    return ;
                }
            }
        }
        return r->set(dat) ;
    }

    std::shared_ptr<Node<T>> get(key_t const & it) const {
        std::shared_ptr<Node<T>> r ;
        if( it.position.size() > 0 ) {
            r = mRoot ;
            for ( int i = 1 ; i < it.position.size() ; i++ ) {
                r = r->nth_child(it.position[i]) ;
                if ( !r ) {
                    return std::shared_ptr<Node<T>>() ;
                }
            }
        }
        return r ;
    }

    virtual key_t append( key_t const & n, T data, int pos ) {
        auto t = get(n) ;
        if ( t ) {
            auto niou = std::make_shared<Node<T>>(data) ;
            t->set_nth(pos,niou) ;
            key_t k(n);
            k.position.push_back(pos) ;
            return  k;
        } else {
            std::cout << "Bad append of " << data << std::endl ;
        }
        return key_t() ;
    }

    virtual key_t insert_in(key_t const & n,T data, int pos, int pos2 = 0 ) {
        auto node = get(n);
        if( node ) {
            auto tmp = node->nth_child(pos) ;
            auto niou = std::make_shared<Node<T>>(data) ;
            node->set_nth(pos,niou);
            niou->set_nth(pos2,tmp);
            key_t r(n) ;
            r.position.push_back(pos);
            return r;
        }else {
            std::cout << "Bad insert of " << data << std::endl ;
        }
        return key_t() ;
    }

    virtual __Tree__ cut( key_t const &  n ) {
        if ( get(n) != get(root()) ) {
            auto son = get(n) ;
            auto fat = n.father() ;
            auto father = get(fat) ;
            if ( father && son ) {
                for ( auto it = 0 ; it < father->size() ; it++ ) {
                    if( father->nth_child(it) == son ) {
                        father->set_nth(it,nullptr) ;
                        return __Tree__(son) ;
                    }
                }
            }
            else {
                std::cout << "Bad cut " << n.position.size() << std::endl ;
            }
        } else {
            auto son = mRoot ;
            mRoot = std::shared_ptr<Node<T>>() ;
            return __Tree__(son) ;
        }
    }

    virtual void add(key_t const & n , __Tree__& ad, int pos = 0) {
       auto father = get(n) ;
       if ( father && father != get(root()) ) {
           father->set_nth(pos,ad.mRoot) ;
        }else {
           if(father == get(root())) {
               mRoot = ad.mRoot ;
           }
       }
        return ;
    }

    virtual __Tree__ replace(key_t const & n , __Tree__& ad) {
        auto son = get(n) ;
        auto fat = n.father() ;
        auto father = get(fat) ;
        if ( father ) {
            for ( auto it = 0 ; it < father->size() ; it++ ) {
                if( father->nth_child(it) == son ) {
                    father->set_nth(it,ad.mRoot) ;
                    return __Tree__(son) ;
                }
            }
        }
        else {
           std::cerr << "Bad replace of " <<  std::endl ;
        }
        return __Tree__() ;
    }

    virtual __Tree__ subtree(key_t const & n ) {
        auto son = get(n) ;
        if( son ) {
            return __Tree__(son) ;
        }else {
            std::cerr << "Bad subtree " << std::endl ;
        }
       return __Tree__() ;
    }

    __Tree__<T>& operator=(__Tree__<T> t) {
        std::swap(t.mRoot,this->mRoot);
        return *this ;
    }

    T operator[](key_t const & n) {
        return get(n)->data() ;
    }

    int size() const {
        return keys().size() ;
    }

    std::vector<key_t> keys() {

        std::shared_ptr<Node<T>> r ;
        std::deque<std::shared_ptr<Node<T>>> todo ;
        std::deque<std::shared_ptr<Node<T>>> nxt_todo ;

        std::vector<key_t> keys ;
        std::deque<key_t>  todokeys ;
        std::deque<key_t> nxt_keys ;
        if ( mRoot != nullptr ) {
            key_t deepness  = root() ;
            todo.push_back(mRoot) ;
            keys.push_back(deepness);
            todokeys.push_back(deepness);
            while( todo.size() != 0 ) {
                while( todo.size() != 0 ) {
                    r = todo.front() ;
                    deepness = todokeys.front() ;
                    for ( int i = 0 ; i < r->size() ; i++ ) {
                        if ( r->nth_child(i) ) {
                            nxt_todo.push_back(r->nth_child(i));
                            auto t = deepness ;
                            t.position.push_back(i);
                            nxt_keys.push_back(t);
                            keys.push_back(t);
                        }
                    }
                    todo.pop_front() ;
                    todokeys.pop_front() ;
                }
                todo.swap(nxt_todo);
                todokeys.swap(nxt_keys) ;
                nxt_todo.clear();
                nxt_keys.clear();
            }
        }
        return keys ;
    }

    bool operator==(const __Tree__& other ) {
        return false ;
    }

};


    /*
    template< typename OStream , typename T >
    OStream& operator<< ( OStream& os , Node<T>& t ) {
        os << (int)&t << "[label=\"" << t.data() << "\"]" << std::endl ;
        for ( int i = 0 ; i < t.size() ; i++ ) {
            if( t.nth_child(i) != nullptr ) {
                os << (int)&t << " -> " << (int)t.nth_child(i).get() << std::endl ;
                os << *(t.nth_child(i)) ;
            }
        }
    }
    */


/*
template< typename OStream , typename T >
OStream& operator<< ( OStream& os , __Tree__<T>& t ) {
    os << std::string("digraph g { ") << std::endl ;
//    os <<  *(t.get(t.root())) ;
    os << std::string("}") ;
}
*/



/**
 *  GRAPH STRUCTURE
 */

template < typename Vertex, typename Edge >
struct ValuedGraph {
    std::vector<Vertex> mVertices ;
    std::vector<std::vector<std::pair<int,Edge>>> mEdges ;

    ValuedGraph()
        :mVertices(),mEdges() { }

    ValuedGraph(ValuedGraph const & g)
        :mVertices(g.mVertices),mEdges(g.mEdges) { }

    ValuedGraph<Vertex,Edge>& operator=(ValuedGraph<Vertex,Edge> v) {
        std::swap(mVertices,v.mVertices) ;
        std::swap(mEdges,v.mEdges) ;
        return *this ;
    }

    typedef int key_t ;

    inline int vrt_size() {
        return mVertices.size() ;
    }

    inline int ed_size() {
        int num = 0 ;
        for ( int i = 0 ; i < mEdges.size() ; i++ ) {
            num += mEdges[i].size() ;
        }
        return num ;
    }

    key_t add_vertex(Vertex ed) {
        //std::cout << "add vertex " << ed << std::endl ;
        //auto id = std::find(mVertices.begin(),mVertices.end(),ed) ;
        //if( id == mVertices.end() ) {
            mVertices.push_back(ed);
            mEdges.push_back( std::vector<std::pair<int,Edge>>() ) ;
            return ( mVertices.size() - 1 ) ;
        //}
        //return ( id - mVertices.begin() ) * mVertices.size() /  ( mVertices.end() - mVertices.begin() ) ;
    }

    std::vector<key_t> add_edge(Vertex ed, Vertex edd,Edge t) {
        std::vector<key_t> keys(2) ;
        keys[0] = add_vertex(ed) ;
        keys[1] = add_vertex(edd) ;
        if( std::find_if(mEdges[keys[0]].begin(),mEdges[keys[0]].end(),
                         [&keys](std::pair<int,Edge>& p) { return p.first == keys[1] ; } ) == mEdges[keys[0]].end() )
            mEdges[keys[0]].push_back(std::make_pair(keys[1],t));
        return keys ;
    }

    void add_edge(int a,int b, Edge t) {
        if( a < mEdges.size() && b < mEdges.size() ) {

            if( std::find_if(mEdges[a].begin(),mEdges[a].end(),
                             [&b](std::pair<int,Edge>& p) { return p.first == b ; } ) == mEdges[a].end() ) {
                mEdges[a].push_back(std::make_pair(b,t));
            }
        }
    }

};





template < typename OStream , typename T, typename U>
OStream& operator << (OStream& os , ValuedGraph<T,U>& g ) {
    os << std::string("digraph g { ") << std::endl ;
    for ( int it = 0 ; it < g.mEdges.size() ; it++ ) {
        for ( int n = 0 ; n < g.mEdges[it].size() ; n++ ) {
            os << std::to_string(it) << std::string("->") << std::to_string(g.mEdges[it][n].first) << ";" << std::endl ;
        }
    }
    os << std::string("}") << std::endl  ;





    return os ;
}



#endif // TREE_HPP
