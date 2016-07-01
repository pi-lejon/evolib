#ifndef CLONEABLE_HPP
#define CLONEABLE_HPP

template< typename Base , typename Derived>
class Cloneable : public Base {
public:
    using Base::Base ;
    virtual Base* clone() const {
        return new Derived(static_cast<Derived const &>(*this)) ;
     }
};


template < typename T >
class AbstractCloneable : public T {
    public :
    AbstractCloneable() {}
    AbstractCloneable(const T& c):T(c) {}
    virtual ~AbstractCloneable() {}
    virtual AbstractCloneable<T>* clone() const = 0 ;
};



#endif // CLONEABLE_HPP
