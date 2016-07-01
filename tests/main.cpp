#include "grammar.hpp"

int main ( int argc , char** argv ) {
	
	evo::InstructionFactory<double> f ;
	f	(new Unterminal<double,2>("ADD",[](double a,double b){ return a + b ; }))
		(new Unterminal<double,2>("SUB",[](double a,double b){ return a - b ; }))
		(new Unterminal<double,2>("MULT",[](double a,double b){ return a * b ; }))
		(new Unterminal<double,2>("DIV",[](double a,double b){ return a / b ; }))
		(new Unterminal<double,1>("LOG",[](double a) { return log(a); }))
		(new Terminal<double>("VAR_X",&x)
		(new Terminal<double>("VAR_Y",&y)
		(new Terminal<double>("CTE",0,1) ;

	/**
	 *	auto tree = f.getRandomTree() ;
	 *	tree.eval(x,y) ;
	 *	tree.eval(x,y) ;
	 *
	 *
	 *
	 *
	 *
	 *
	 */
	

	std::cout << "All test passed" << std::endl ;
	return 0 ;
}
