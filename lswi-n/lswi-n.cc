// ** now using include/release3.0-chemical  **

// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   See the file COPYING for full copying permissions.                      *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
// This problem simulates the injection of low salinity water into a core 
// saturated with oil with the following a sequence for the PD1 problem 
// (any number of sequences and episodes per sequence are specified
// from the input file):
//
// first: high salinity
// second: low salinity

// In this approach we define the concept of stage, where each stage has a 
// determined number of episodes (defaults to one episode per stage if not
// otherwise specified). The idea behind this is that for each stage 
// a determined number of experimental values may be available and for each
// experimental value we define an episode. When each episode is concluded, 
// the simulation result is compared against the target value and the
// the error is tabulated. These values are output to stderr where they can
// be parsed by shell scripts.
// The sum of rms errors provides an objective function for minimization while 
// exploring the space of values for Brooks Corey type material laws.

/*!
 * \file
 *
 * \brief Test for the 2pnc model used for low-salinity water flooding.
 */

// ### Includes
// Configuration header created by cmake.
#include <config.h>

// Further, we include a standard header file for C++, to get time and date information
#include <ctime>
// and another one for in- and output.
#include <iostream>

// Dumux is based on DUNE, the Distributed and Unified Numerics Environment, 
// which provides several grid managers and linear solvers. So we need some 
// includes from that. This program is targeted for dune-2.7.
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/timer.hh>
#include <dune/grid/io/file/dgfparser/dgfexception.hh>
#include <dune/grid/io/file/vtk.hh>
#include <dune/istl/io.hh>
// In Dumux, a property system is used to specify the model. For this,
// different properties are defined containing type definitions, values 
// and methods. All properties are declared in the file `properties.hh`.
#include <dumux/common/properties.hh>
// The following file contains the parameter class, which manages the 
// definition of input parameters by a default value, the inputfile or 
// the command line.#include <dumux/common/parameters.hh>
// The file `dumuxmessage.hh` contains the class defining the start and
// end message of the simulation.#include <dumux/common/dumuxmessage.hh>
#include <dumux/common/defaultusagemessage.hh>
// The valgrind header provides memory analysis to avoid leaks and incorrect
// calls on non initialized code.
#include <dumux/common/valgrind.hh>
// Header for amg iterative solver. This solver allows usage of MPI parallel
// execution.
#include <dumux/linear/amgbackend.hh>
// Newton solver for nonlinear part of the algorithm.
#include <dumux/nonlinear/newtonsolver.hh>
// Assembler of the matrix which represents the system of equations to be 
// solved, for finite volume schemes (box-scheme, tpfa-approximation,
// mpfa-approximation).
#include <dumux/assembly/fvassembler.hh>
// The containing class in the following file defines the different 
// differentiation methods used to compute the derivatives of the residual. 
#include <dumux/assembly/diffmethod.hh>
#include <dumux/discretization/method.hh>
// We need the following class to simplify the writing of dumux simulation 
// data to VTK format.
#include <dumux/io/vtkoutputmodule.hh>
// The gridmanager constructs a grid from the information in the input or 
// grid file. There is a specification for the different supported grid 
// managers.
#include <dumux/io/grid/gridmanager.hh>
// The following header is required to access solution values to feed to 
// the Brooks Corey Modified Variable (BCMV) material law.
#include <dumux/io/loadsolution.hh>
// Convenience debug/warning/trace preprocesor macros:
// Used mainly for debugging. TRACE is extremely verbose and off 
// by default. DBG is on by default. WARN is an execution warning (not
// used too often). To turn off, use -DNODEBUG and -DNOWARN in CXXFLAGS
// or from CMakeLists.txt specifications.
// TRACE:
#undef TRACE
#define TRACE(...)   { (void)0; }
// DBG:
#undef DBG
#ifndef NODEBUG
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
#else
# define DBG(...)   { (void)0; }
#endif
// WARN:
# undef WARN
#ifndef NOWARN
# define WARN(...)  {fprintf(stderr, "warning> "); fprintf(stderr, __VA_ARGS__);}
#else
# define WARN(...)   { (void)0; }
#endif


void usage(const char *progName, const std::string &errorMsg)
{
    if (errorMsg.size() > 0) {
        std::string errorMessageOut = "\nUsage: ";
                    errorMessageOut += progName;
                    errorMessageOut += " [options]\n";
                    errorMessageOut += errorMsg;
                    errorMessageOut += "\n\nThe list of mandatory options for this program is:\n"
                                        "\t-ParameterFile Parameter file (Input file) \n";

        std::cout << errorMessageOut
                  << "\n";
    }
}

// Number of particles in water phase. This defaults to 1 if not
// specified from CXXFLAGS or from the CMakeList.txt file.
// This is the item which requires recompilation, as a different
// executable is created for each case with different number of
// particles.
#ifndef NUM_PARTICLES
# define NUM_PARTICLES 1
#endif
// Water + number of particles in water phase = BRINE_N_COMPONENTS.
#define BRINE_N_COMPONENTS (NUM_PARTICLES+1)

// This is a structure to contain the identifier for each particle 
// and the molecular weight. This structure may well be averted if 
// a components template is available for the aforementioned particle,
// but then care must be taken since code considered chemical units
// (gmol) while component files are specified in SI (kgmol).
// The structure is used in "dumux/material/fluidsystems/brine-n.hh"
// and in "lswidata.hh".
typedef struct particle_t {
    std::string idx;
    double molecularWeight;
}particle_t;

// These are two static global variables for simple consecutive 
// calculations used thus to make code easier to read.
double oilRecovery = 0;
double currentHour = 0;
// Local input data template:
#include "lswidata.hh"
// Spatial parameters:
#include "lswispatialparams.hh"
// Problem definition:
#include "lswiproblem.hh"

/*!
 * \brief Provides an interface for customizing error messages associated with
 *        reading in parameters.
 *
 * \param progName  The name of the program, that was tried to be started.
 * \param errorMsg  The error message that was issued by the start function.
 *                  Comprises the thing that went wrong and a general help message.
 */

// ### Beginning of the main function.
//     This function contains the time loop and episode processing.

int main(int argc, char** argv) try
{
    DBG("BRINE_N_COMPONENTS = %d particles= %d\n", BRINE_N_COMPONENTS, NUM_PARTICLES);
    using namespace Dumux;

    // Define the type tag for this problem.
    using TypeTag = Properties::TTag::LSWFBoxTypeTag;

    using Scalar = GetPropType<TypeTag, Properties::Scalar>;

    // Initialize MPI, finalize is done automatically on exit.
    // Useful when execution is done via mpirun. Otherwise harmless.
    const auto& mpiHelper = Dune::MPIHelper::instance(argc, argv);

    // Print dumux start message. Only on processor 0 in mpi runs.
    if (mpiHelper.rank() == 0)
        DumuxMessage::print(/*firstCall=*/true);

    // Parse command line arguments and input file.
    // This creates the dune parameter tree which is used to define
    // stages and episodes, among other data.
    Parameters::init(argc, argv, usage);

    // Create and initialize grid (from the given grid file or the input 
    // file definitions).
    // This makes use of 3.0 GridManager template.
    GridManager<GetPropType<TypeTag, Properties::Grid> > gridManager;
    gridManager.init();

    ////////////////////////////////////////////////////////////
    // Run instationary non-linear problem on this grid.
    ////////////////////////////////////////////////////////////

    // Compute on the leaf grid view.
    const auto& leafGridView = gridManager.grid().leafGridView();

    // Create the finite volume grid geometry as a shared pointer
    // and update.
    using GridGeometry = GetPropType<TypeTag, Properties::GridGeometry>;
    auto gridGeometry = std::make_shared<GridGeometry>(leafGridView);
    gridGeometry->update();

    // Get problem definition from the properties namespace
    // defined in the "lswiproblem.hh file".
    using Problem = GetPropType<TypeTag, Properties::Problem>;
    auto problem = std::make_shared<Problem>(gridGeometry);

    // Check if we are about to restart a previously interrupted simulation.
    // Beware that in Dumux 3.0 restart data is in float format, so
    // that double precision is not restored. This may lead to different
    // results or even program termination. If item is not found on 
    // input file, or commented out, value will default to 0
    // (as seen in getParam<type>() call.
    Scalar restartTime = getParam<Scalar>("Restart.Time", 0);
    int restartStep = getParam<int>("Restart.Step", 0);
    
    // Define the solution vector.
    using SolutionVector = GetPropType<TypeTag, Properties::SolutionVector>;
    SolutionVector x(gridGeometry->numDofs());
    // Apply the initial solution or the restart solution.
    if (restartTime > 0)
    {
        using IOFields = GetPropType<TypeTag, Properties::IOFields>;
        using PrimaryVariables = GetPropType<TypeTag, Properties::PrimaryVariables>;
        using ModelTraits = GetPropType<TypeTag, Properties::ModelTraits>;
        using FluidSystem = GetPropType<TypeTag, Properties::FluidSystem>;
        const auto fileName = getParam<std::string>("Restart.File");
        const auto pvName = createPVNameFunction<IOFields, PrimaryVariables, ModelTraits, FluidSystem>();
        loadSolution(x, fileName, pvName, *gridGeometry);
    }
    else {
        problem->applyInitialSolution(x);
    }

    // Initialize the grid variables with the initial solution or
    // the restart solution if restart time specified. If restart
    // time has been specified, then the file containing the restart
    // solution must be specified in the input file with "Restart.File".
    // For this particular example, a "Restart.Recovery" and 
    // "Restart.Step" must also be specified, but would not be
    // necessary in a different problem.
    auto xOld = x;
    // the grid variables
    using GridVariables = GetPropType<TypeTag, Properties::GridVariables>;
    auto gridVariables = std::make_shared<GridVariables>(problem, gridGeometry);
    gridVariables->init(x, xOld);

    // Time loop parameters:
    // DtInitial: size of the initial value for delta_t in march method (seconds)
    // MaxTimeStepSize: limit for size of delta_t to grow in adaptative method.
    // TEnd: End of simulation, in seconds.
    // Default value for MaxTimeStep is 1e100 (no limit). 
    using Scalar = GetPropType<TypeTag, Properties::Scalar>;
    auto dt = getParam<Scalar>("TimeLoop.DtInitial");
    const auto maxDt = getParam<Scalar>("TimeLoop.MaxTimeStepSize");
    const auto tEnd = getParam<Scalar>("TimeLoop.TEnd");
    // const auto maxDivisions = getParam<int>("TimeLoop.MaxTimeStepDivisions");
    //////////////////////////////////////////////////////////////
    // intialize the vtk output module
    using IOFields = GetPropType<TypeTag, Properties::IOFields>;
    VtkOutputModule<GridVariables, SolutionVector> vtkWriter(*gridVariables, x, problem->name());   
    
    using VelocityOutput = GetPropType<TypeTag, Properties::VelocityOutput>;
    vtkWriter.addVelocityOutput(std::make_shared<VelocityOutput>(*gridVariables));
    IOFields::initOutputModule(vtkWriter); //! Add model specific output fields
    vtkWriter.write(0.0);
    ///////////////////////////////////////////////////////////

    // instantiate time loop
    auto timeLoop = std::make_shared<TimeLoop<Scalar>>(restartTime, dt, tEnd);

    // the assembler with time loop for instationary problem
    using Assembler = FVAssembler<TypeTag, DiffMethod::numeric>;
    auto assembler = std::make_shared<Assembler>(problem, gridGeometry, gridVariables, timeLoop);

    // // the linear solver
    // using LinearSolver = AMGBackend<TypeTag>;
    // auto linearSolver =  std::make_shared<LinearSolver>(leafGridView, gridGeometry->dofMapper());
    using LinearSolver = UMFPackBackend;
    auto linearSolver =  std::make_shared<LinearSolver>();

    // the non-linear solver
    using NewtonSolver = NewtonSolver<Assembler, LinearSolver>;

    // NewtonMethod nonLinearSolver(newtonController, assembler, linearSolver);
    NewtonSolver nonLinearSolver(assembler, linearSolver);
            
    // time loop
    int currentEpisodeIndex=0; 
    timeLoop->start(); 
    problem->setTime(timeLoop->timeStepIndex(),timeLoop->time(),timeLoop->timeStepSize());
    TRACE("Problem time index %d, time= %lf,  step size= %lf\n###############################\n", 
        timeLoop->timeStepIndex(),timeLoop->time(),timeLoop->timeStepSize());

    Scalar errorSum = 0;
    Scalar rootMS = 0;

    auto timeLimit = getParam<time_t>("TimeLoop.timeLimit", 0.0);
    time_t start=time(NULL);
    if (timeLimit > 0){ 
        WARN("Program execution time limit set from input file to %lu minutes\n",
                timeLimit);
    }
    if(problem->episodeCount()) {
        timeLoop->setMaxTimeStepSize(problem->getMaxTimeStepSize(0));
    } else {
        timeLoop->setMaxTimeStepSize(problem->getMaxTimeStepSize());
    }
    
    do
    {
        if (timeLimit && time(NULL) > timeLimit){
            DBG("PARSE_T time limit (%ld minutes)for execution reached. Abort.\n", (long)(timeLimit - start)/60);
            exit(1);
        }
        // Set current hour:
        currentHour = timeLoop->time()/3600.0;
        // Are we done with last episode?
        if (timeLoop->time() >=  problem->getUpperTimeStepBoundary(problem->episodeCount() - 1)){
            auto target = problem->getTarget(currentEpisodeIndex);
            DBG("Last episode %d (stage %d) time limit (%lf hours) has been reached.\n", 
                    problem->episodeCount(), 
                    problem->stageCount(), 
                    problem->getUpperTimeStepBoundary(problem->episodeCount() - 1)/3600);
            DBG("episode[%d] recovery/target=%lf/%lf error=%le\n",
                    currentEpisodeIndex, oilRecovery,
                    target, 
                    target>0?
                      fabs(problem->getTarget(currentEpisodeIndex) - oilRecovery):
                          -1.0); 
            if (problem->episodeCount() and problem->getTarget(currentEpisodeIndex)>0) {
                auto error = fabs(problem->getTarget(currentEpisodeIndex) - oilRecovery);
                errorSum += error;
                rootMS += (error*error);
                DBG("PARSE episode=%d  error=%lf avgError=%lf rootMS=%lf time=%ld\n",
                    currentEpisodeIndex, error, errorSum/(currentEpisodeIndex+1),
                    sqrt(rootMS),
                    (long)(time(NULL)-start)/60);
            } 
            break;
        }

        // Determine episode index.
        int episodeIndex = 0;
        for (int idx=0; idx<problem->episodeCount(); idx++){
            if (timeLoop->time() < problem->getUpperTimeStepBoundary(idx) - problem->eps_ 
                    &&
                timeLoop->time() >= problem->getLowerTimeStepBoundary(idx))
            {
                episodeIndex=idx;
                break;
            }
        }
        DBG("Step=%d episode=%d (%s) current=%d, time=%lf, timestep=%lf nexttime=%lf \n",
                timeLoop->timeStepIndex() + restartStep,  
                episodeIndex+1, 
                problem->episodeStageName(episodeIndex).c_str(),
                currentEpisodeIndex+1,
                timeLoop->time(),timeLoop->timeStepSize(),
                timeLoop->time()+timeLoop->timeStepSize() );
       
        if (timeLoop->timeStepIndex()  == 2) {
            DBG("******   dump  ******\n");
            problem->spatialParams().dump();
        }


        if (episodeIndex != currentEpisodeIndex){
            DBG("setting maxTimeStepSize to %le\n", problem->getMaxTimeStepSize(episodeIndex));
            timeLoop->setMaxTimeStepSize(problem->getMaxTimeStepSize(episodeIndex));
            DBG("*** timeLoop: episode switch %d --> %d at %lf s.\n", 
                    currentEpisodeIndex, episodeIndex, timeLoop->time());
            auto target = problem->getTarget(currentEpisodeIndex);
            DBG("episode[%d] recovery/target=%lf/%lf error=%le\n",

                    currentEpisodeIndex, oilRecovery,
                    target, 
                    target>0?
                      fabs(problem->getTarget(currentEpisodeIndex) - oilRecovery):
                          -1.0); 
            if (problem->getTarget(currentEpisodeIndex)>0) {
                auto error = fabs(problem->getTarget(currentEpisodeIndex) - oilRecovery);
                errorSum += error;
                rootMS += (error*error);
                fprintf(stdout, "PARSE episode=%d  error=%lf avgError=%lf rootMS=%lf time=%ld\n",
                    currentEpisodeIndex, error, errorSum/(currentEpisodeIndex+1),
                    sqrt(rootMS),
                    (long)(time(NULL)-start)/60);
/*                fprintf(stdout, "PARSE episode=%d  error=%lf avgError=%lf rootMS=%lf\n",
                    currentEpisodeIndex, error, errorSum/(currentEpisodeIndex+1),
                    sqrt(rootMS));*/
            } 
            currentEpisodeIndex = episodeIndex;
            // set episode form material law parameters    
            DBG("----timeLoop---- Setting episode spatial parameters...\n");
            problem->spatialParams().setEpisode(currentEpisodeIndex);

            // set episode fluidsystem densities and viscosities
            TRACE("timeLoop: Setting episode fluid system parameters...\n");
            
            problem->setDensityViscosity(currentEpisodeIndex);
            // Set initial timestep for episode...
            TRACE("timeLoop: Setting initial timestep size for next episode to %lf...\n",
                    problem->getDtInitial(currentEpisodeIndex));
            timeLoop->setTimeStepSize(problem->getDtInitial(currentEpisodeIndex));    
        }

        // check if timestep does not overshoot episode end
        auto upperTime = (problem->episodeCount())?
            problem->getUpperTimeStepBoundary(currentEpisodeIndex):problem->getTEnd();

        if (timeLoop->time()+timeLoop->timeStepSize() - upperTime > problem->eps_){
            // If initial timestep size overreaches, reset timestep.
            timeLoop->setTimeStepSize(upperTime - timeLoop->time());    
            DBG("timeLoop: *** limiting time step to end of episode:  max step=%le\n", 
                timeLoop->timeStepSize());
        }    

        // set previous solution for storage evaluations
        assembler->setPreviousSolution(xOld);

        // solve the non-linear system with time step control
        nonLinearSolver.solve(x, *timeLoop);

        // make the new solution the old solution
        xOld = x;
        gridVariables->advanceTimeStep();

        // advance to the time loop to the next step
        Scalar lastTimeStepSize = timeLoop->timeStepSize();
        timeLoop->advanceTimeStep();
        TRACE("timeLoop:  time loop now at index=%d time=%lf (step size pending, currently at %lf)\n",
                timeLoop->timeStepIndex(),timeLoop->time(),timeLoop->timeStepSize());

        // write vtk output
        vtkWriter.write(timeLoop->time());
        // report statistics of this time step
        timeLoop->reportTimeStep();


        // This is for the recovery output...
        problem->setTime(timeLoop->timeStepIndex(),timeLoop->time(),lastTimeStepSize);
        // Now we can do the recovery output...
        problem->oilRecOutput(gridVariables->curGridVolVars(), x, currentEpisodeIndex);

        // set new dt as suggested by newton controller
        timeLoop->setTimeStepSize(nonLinearSolver.suggestTimeStepSize(timeLoop->timeStepSize()));
        
    } while (!timeLoop->finished());

    timeLoop->finalize(leafGridView.comm());

    ////////////////////////////////////////////////////////////
    // finalize, print dumux message to say goodbye
    ////////////////////////////////////////////////////////////

    // print dumux end message
    if (mpiHelper.rank() == 0)
    {
        Parameters::print();
        DumuxMessage::print(/*firstCall=*/false);
    }

    return 0;
} // end main
catch (Dumux::ParameterException &e)
{
    std::cerr << std::endl << e << " ---> Abort!" << std::endl;
    return 1;
}
catch (Dune::DGFException & e)
{
    std::cerr << "DGF exception thrown (" << e <<
                 "). Most likely, the DGF file name is wrong "
                 "or the DGF file is corrupted, "
                 "e.g. missing hash at end of file or wrong number (dimensions) of entries."
                 << " ---> Abort!" << std::endl;
    return 2;
}
catch (Dune::Exception &e)
{
    std::cerr << "Dune reported error: " << e << " ---> Abort!" << std::endl;
    return 3;
}
catch (...)
{
    std::cerr << "Unknown exception thrown! ---> Abort!" << std::endl;
    return 4;
}
