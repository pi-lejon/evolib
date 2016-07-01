#ifndef GRAMMAR_HPP
#define GRAMMAR_HPP
#include <functional>
#include <random>
#include <memory>
#include <random>
#include "cloneable.hpp"

namespace evo {

template< typename Out , typename ...In >
class __GPInstruction__ {
    virtual __GPInstruction__* clone() const = 0 ;
    virtual Out eval(In...) const = 0 ;
};

template < typename T >
class Terminal : public Cloneable<__GPInstruction__<T>,Terminal<T>> {
    Terminal<T>()
    : Cloneable<__GPInstruction__<T>,Terminal<T>>()
    {}

    Terminal<T>(T& t)
        : Cloneable<__GPInstruction__<T>,Terminal<T>>(),
        _term(t)
    {}

    virtual T eval() const { return _term; }
private:
    T _term ;
};

template < typename T* >
class Terminal : public Cloneable<__GPInstruction__<T>,Terminal<T*>> {
    Terminal<T*>()
    : Cloneable<__GPInstruction__<T>,Terminal<T*>>()
    {}

    Terminal<T*>(T* t)
        : Cloneable<__GPInstruction__<T>,Terminal<T*>>(),
        _term(t)
    {}

    virtual T eval() const { return *_term; }
private:
    T* _term ;
};


template< typename Out , typename... In >
class Unterminal :  public Cloneable<__GPInstruction__<Out>,Unterminal<Out,In...>> {

    Unterminal<Out,In...>()
      : Cloneable<__GPInstruction__<Out>,Unterminal<Out,In...>>(),
        _callback([](In... i){}) {}

    Unterminal<Out,In...>(std::function<Out(In...)> cl)
      : Cloneable<__GPInstruction__<Out>,Unterminal<Out,In...>>(),
        _callback(cl) {}

    virtual Out eval(In... in) const { return _callback(in...) ; }
private:
    std::function<Out(In...)> _callback ;
};

template < typename T >
using GPInstr_ptr = std::shared_ptr<__GPInstruction__<T>> ;

template < typename Out , typename ... In>
class InstructionFactory {

    InstructionFactory<Out,In...>& operator()(Unterminal<Out,In...>* ut) {
        _unterm.push_back(GPInstr_ptr<Out,In...>(ut)) ;
        _unterm_rnd = std::uniform_int_distribution<int>(0,_unterm.size()-1);
    }

    InstructionFactory<Out,In...>& operator()(Terminal<Out>* t) {
        _term.push_back(GPInstr_ptr<Out,In...>(t)) ;
        _term_rnd = std::uniform_int_distribution<int>(0,_term.size()-1);
		}

		GPInstr_ptr<Out,In...> getRandomTerminal() {
		return _term[_term_rnd(_generator)] ;
    }

    GPInstr_ptr<Out,In...> getRandomUnterm() {
        return _unterm[_unterm_rnd(_generator)] ;
    }

private:
    std::vector<GPInstr_ptr<Out,In...>> _unterm ;
    std::vector<GPInstr_ptr<Out,In...>> _term ;
    std::uniform_int_distribution<int> _unterm_rnd ;
    std::uniform_int_distribution<int> _term_rnd ;
    std::mt19937 _generator ;
};

}// NAMESPACE EVO

#endif // GRAMMAR_HPP
