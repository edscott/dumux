# dumux
Docker files and python scripts for working with DuMux

dune/ contains dockerfiles to build dune-2.4, dune-2.5, dune-2.6.
To build with pdelab support, next in line are dune-pdelab-2.4, 
dune-pdelab-2.5 and dune-pdelab-2.6.

dumux/ contains files to build containers for dumux-2.12-2.4, dumux-2.12 
and dumux-3.0

bin/ contains start docker script and python problem script to simplify configuration,
compilation and execution fo dumux problems.

problem-GM.py and problem-LSWF are used to define particular default and optional compilation
directives to be used by the Python problem.py script.
