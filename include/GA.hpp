#ifndef GA_HPP
#define GA_HPP
#include "strategy.hpp"
#include <algorithm>
#include "boost/filesystem.hpp"
#include "json_spirit.h"
#include "parser.h"
#include <iostream>

namespace js = json_spirit ;
/**
 *      To use GA in a generic manner I should give :
 *          - Genome type
 *          - Genome Factory : random generator of genome (object or member function)   ( no default )
 *          - Fitness        : evaluator                                                ( no default )
 *          - Crosser        : crossover factory                                        ( default : standard operators )
 *          - Mutator        : mutation factory                                         ( default : standard operators )
 *          - Executor       : execution strategy (omp, mpi, pthread,ilot) (default : sequential)
 *          - EndCriterion   : end criterion                               (default : convergence )
 *
 **/

template< typename Genome ,             // the genomes that compose the population
          typename GAparameters ,     // the parameters of the GA : mutation rate, crossover rate, elistism, population size, etc.
          typename Strategy = SequentialEvaluation /*,       // fonctors to mutation and crossover operators
          typename SelectionStrategy,   // fonctors to selection operator (wheel, tournament, etc.)
          typename ExecStrategy   */    // execution strategy (parallel, isle, sequential, openMP, MPI, etc.)
          >
struct GeneticAlgorithm {
    std::vector<Genome> mPopulation ;
    std::vector<Genome> mNewPopulation ;
    std::vector<Genome> mMatingPool ;
    std::vector<float> mNotes ;
    Genome mBest ;
    float best_note ;
    int iter ;
    int indice ;

    void init() {
        iter = 0 ;
        for ( int i = 0 ; i < GAparameters::population_size ; i++ ) {
            Genome g ;
            g.random() ;
            mPopulation.push_back(g) ;
            mNotes.push_back(0.f);
        }
    }

    void evaluate() {
        Strategy::evaluate_population(mPopulation,mNotes) ;
        auto best_ind = std::min_element(mNotes.begin(),mNotes.end()) ;
        best_note = *best_ind ;
        indice = best_ind - mNotes.begin() ;
        mBest = Genome(mPopulation[indice]) ;
        std::cout << "Generation " << iter++ << " : " << best_note << std::endl ;
    }

    void build_mating_pool () {
        assert(mPopulation.size() == GAparameters::population_size) ;
        for ( int i = 0 ; i < GAparameters::crossover_rate * GAparameters::population_size * 2 ; i++ ) {
            std::vector<int> indiv ;
            for ( int j = 0 ; j < 7 ; j++ ) {
               indiv.push_back(rand() % mPopulation.size()) ;
            }
            int best = *std::min_element(indiv.begin(),indiv.end(),[this](int& a,int& b){ return mNotes[a] < mNotes[b] ;} ) ;
            mMatingPool.push_back(mPopulation[best]);
        }
    }

    void apply_operators() {
        for ( int i = 0 ; i < mMatingPool.size() - 1 ; i += 2 ) {
            std::vector<Genome> of = crossover(mMatingPool[i],mMatingPool[i+1]) ;
            mNewPopulation.insert(mNewPopulation.end(),of.begin(),of.end());
        }
        mMatingPool.clear();
        for ( int i = 0 ; i < GAparameters::mutation_rate * GAparameters::population_size ; i++ ) {
            Genome of = mutate(mPopulation[rand()%mPopulation.size()]) ;
            mNewPopulation.push_back(of);
        }
    }

    void build_new_population() {
        int num = GAparameters::population_size - mNewPopulation.size() - 1 ;
        for ( int i = 0 ; i < num ; i++ ) {
            mNewPopulation.push_back(mPopulation[rand()%mPopulation.size()]);
        }
        mNewPopulation.push_back(mBest);
        std::swap(mPopulation,mNewPopulation) ;
        mNewPopulation.clear();
        assert(mPopulation.size() == GAparameters::population_size) ;
    }

    void save_best() {
        mBest.save() ;
    }

    void start() {
        init() ;
        do {
            boost::filesystem::path dir("./"+ std::to_string(iter) );
            if ( boost::filesystem::create_directory(dir) ) {
                chdir(dir.string().c_str()) ;
                assert(mPopulation.size() == GAparameters::population_size) ;
                evaluate() ;
                build_mating_pool() ;
                apply_operators() ;
                build_new_population() ;
                save_best();
                chdir("..") ;
            }
        } while (!end_criterion()) ;
    }

    bool end_criterion() {return  iter >= GAparameters::numGenerations || (best_note >= -0.001f && best_note <= 0.001f) ;}

};

template< typename Genome , typename GAparameters >
struct SteadyStateGA {
    std::vector<Genome> mPopulation ;
    std::vector<float> mNotes ;
    std::vector<int> mSeeds ;
    Genome mBest ;
    float best_note ;
    int iter ;
    int indice ;

    void init( std::string popfile ) {
        std::ifstream in(popfile) ;
        js::mValue v ;
        js::read(in,v) ;
        js::mObject lPop = v.get_obj() ;
        // if the population is still evaluated skip this loop
        iter = 0 ;
        for ( int i = 0 ; i < lPop.size() ; i++ ) {
            auto indiv = lPop[std::to_string(i)].get_obj()["genome"].get_obj() ;
            Parser p;
            SynbProgram sbp = p.parse(indiv) ;
            Genome g ; g.from_program(sbp) ;
            float fit;
            int seed ;

            fit = lPop[std::to_string(i)].get_obj()["fitness"].get_real() ;



            seed = lPop[std::to_string(i)].get_obj()["seed"].get_int() ;

            mPopulation.push_back(g) ;
            mNotes.push_back(fit);
            mSeeds.push_back(seed);
//            std::cout << "Add indiv " << i << " with fitness " << fit << std::endl ;
        }
        in.close() ;
    }

    void start(float duration , std::string popfile, std::string outpop)
    {
        //std::cout << "Start SSGA " << std::endl ;

        std::time_t end_time, ctime ;
        time(&ctime) ; // heure courante
        end_time = ctime + duration * 3600  ; // heure fin
        init(popfile) ;

        //std::cout << "Initialisation Done - Start evaluation loop at " << ctime << std::endl ;

        while ( ctime < end_time ) {

  //          std::cout << "Need to run " << difftime(end_time,ctime) << std::endl ;

            assert(mPopulation.size() == mNotes.size() ) ;
            Genome p1,p2 ; // parent 1,2 - child 1,2

            std::vector<float> notes ;
            std::vector<int> seeds ;


            // on sélectionne deux parents
            auto i1 = SelectionStrategy::TournamentSelection( 7 , mPopulation , mNotes ) ;
            auto i2 = SelectionStrategy::TournamentSelection( 7 , mPopulation , mNotes ) ;


            // Si les parents ne sont pas évalués, j'évalue :
            p1 = mPopulation[i1] ;
            p2 = mPopulation[i2] ;
            int seed ;

            if( mNotes[i1] == -1 ) {
                p1.evaluate(&seed) ;
                mSeeds[i1] = seed ;
            }

            if ( mNotes[i2] == -1 ) {
                p2.evaluate(&seed) ;
                mSeeds[i2] = seed ;
            }

            notes.push_back(mNotes[i1]);
            seeds.push_back(mSeeds[i1]);

            notes.push_back(mNotes[i2]);
            seeds.push_back(mSeeds[i2]);


            // on crée des enfants depuis ces parents -- rajouter les taux de croisements et mutation
            auto v = crossover(p1,p2) ;

            // on applique une mutation
            v[0] = mutate(v[0]) ;
            v[1] = mutate(v[1]) ;

            // on évalue ces enfants



            float n1 = v[0].evaluate(&seed) ;
            notes.push_back(n1);
            seeds.push_back(seed);

            float n2 = v[1].evaluate(&seed) ;
            notes.push_back(n2);
            seeds.push_back(seed);


            v.push_back(p1) ; v.push_back(p2) ;

//            std::cout << "child evaluation done" << std::endl ;


            // on check les deux meilleurs entre parents et enfants
            bool swap ;
            do {
                swap = false ;
                for ( int i = 0 ; i < 3 ; i++ ) {
                    if ( notes[i] < notes[i+1] ) {
                        std::swap(notes[i],notes[i+1]) ;
                        std::swap(v[i],v[i+1]) ;
                        std::swap(seeds[i],seeds[i+1]) ;
                        swap = true ;
                    }
                }
            } while (swap) ;

//            std::cout << "best found" << std::endl ;

            // On remplace les parents par les deux enfants
            mPopulation[i1] = v[0] ;
            mPopulation[i2] = v[1] ;
            mNotes[i1] = notes[0] ;
            mNotes[i2] = notes[1] ;

//            std::cout << "replacement done" << std::endl ;
            time(&ctime) ;

        }
        // save the pop in output file
//        std::cout << "End of GA -- Save pop file" << std::endl ;
        std::ofstream out (outpop) ;
        js::mObject pop , gen ;
        assert(mPopulation.size() == mNotes.size()) ;
        assert(mNotes.size() == mSeeds.size()) ;
        for ( int i = 0 ; i < mPopulation.size() ; ++i ) {
            gen["genome"] = mPopulation[i].to_program().saveAsSBGP() ;
            gen["fitness"] = mNotes[i] ;
            gen["seed"] = mSeeds[i] ;
            pop[std::to_string(i)] = gen ;
        }
        js::write(pop,out) ;
    }


};


#endif // GA_HPP
