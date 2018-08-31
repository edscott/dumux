#!/usr/bin/python3

######################### libraries
import re
import os
import subprocess
import re
import signal
import shutil
import time
import glob
import sys

def version():
    return "2"

def color(which):
    c={"b-black": "\x1b[01;30m", \
        "black": "\x1b[0;30m", \
        "red": "\x1b[0;31m", \
        "b-red": "\x1b[01;31m", \
        "x-red": "\x1b[31;01m", \
        "green": "\x1b[0;32m", \
        "b-green": "\x1b[01;32m", \
        "x-green": "\x1b[32;01m", \
        "yellow": "\x1b[0;33m", \
        "b-yellow": "\x1b[01;33m", \
        "x-yellow": "\x1b[33;01m", \
        "blue": "\x1b[0;34m", \
        "b-blue": "\x1b[01;34m", \
        "x-blue": "\x1b[34;01m", \
        "magenta": "\x1b[0;35m", \
        "b-magenta": "\x1b[01;35m", \
        "x-magenta": "\x1b[35;01m", \
        "cyan": "\x1b[0;36m", \
        "b-cyan": "\x1b[01;36m", \
        "x-cyan": "\x1b[36;01m", \
        "white": "\x1b[0;37m", \
        "b-white": "\x1b[01;37m", \
        "x-white": "\x1b[37;01m", \
        "bold": "\x1b[01m", \
        "default": "\x1b[0m", \
        "italic": "\x1b[04m", \
        "b-italic": "\x1b[04;01m"}
    return c[which]


def help(argv, available_directives, defaults):
    wd=os.getcwd()+"/projects"
    helptxt= color("b-blue")+"*** Usage:\n  "+argv[0]+version()+" [options]"+ \
        color("green")+"\n\
# arguments:\n\
#   --help  -> print this text\n\
#   --projects={output directory} -> output directory for executable and runtime \n\
#   --only-compile -> remove build-cmake and compile, do not copy anything \
(default: off) \n\
#   --no-clean -> do not remove input file from target run dir (default: clean)\n\
#   --query -> process input file in interactive mode from keyboard (default: \
no-query)\n\
#   --verbose -> print stderr output from configure step to terminal (default: \
no-verbose)\n\
#   --source={problem} -> solve {problem} with defaults, unattended \n\
#   --search={search directory} -> search for {problem} directory (default ./)\n\
#   --projects={projects directory} -> output directory for unattended {problem}\n\
#               (default: "+wd+")\n\
#   --overwrite -> Force creation of new project (default: query before overwrite)\n\
#   --run -> Short circuit to run problem (default: off)\n\
#   --input=n -> With multiple input files, use the one indexed by {n}\n\
#   --plot -> Follow simulation with gnuplot output\n\
#   --MPI=n -> mpirun --np n, where {n} is an integer (implies -DAMG)"+ \
    color("default")
    print(helptxt)
    print_bblue("# Code particular directives:")
    for k in available_directives.keys():
        print_bblue("#   --D"+k)
        text = "       "+color("green")+available_directives[k]+color("default")
        for d in defaults:
            if  k == d:
                text = text + color("b-red") + " (default)" + color("default")
        print(text)
    sys.exit(1)
########################### functions
def get_specific_problem(argv):
    env={}
    env.update(os.environ)
    current_dir=os.path.abspath("./")
    PROJECTS=current_dir+"/projects"
    problem_directory=None
    sd=None
# Specification of projects directory from command line
    for arg in argv:
        if ("--projects=" in arg[0:len("--projects=")]):
            PROJECTS = os.path.abspath(arg[len("--projects="):])
            info("Projects directory set to "+arg)
            try:
                os.stat(PROJECTS)
            except:
                try:
                    os.makedirs(PROJECTS)
                except:
                    error("Cannot create projects directory "+color("b-magenta")+ \
                    PROJECTS)

# Specification of search directory from command line
    for arg in argv:
        if ("--search=" in arg[0:len("--search=")]):
            sd=arg[len("--search="):]
            
    if sd == None:
        sd = os.getcwd()
        info("Search directory set to " + sd +color("b-red")+" (override with --search=)")
    else:
        try:
            os.stat(sd)
        except:
            error("Cannot locate search directory: "+sd)                   
        info("Search directory set to "+sd)

# Specification of specific source (and problem_directory) for unattended run
    for arg in argv:
            
        if ("--source=" in arg[0:len("--source=")]):
            debug(arg+"specified, going into non interactive mode...")
            glob_pattern=os.path.abspath(sd)+'/**'

            specific=arg[len("--source="):]
            #p = re.compile('.cc\b')
            if re.search("\.cc$", specific):
                debug("* Searching for source: "+ specific)
            else:
                specific = specific + ".cc"
                debug("* Searching for source: "+ specific + "(added .cc)")

            debug("specific="+specific+" glob_pattern="+glob_pattern)
            p = re.compile(specific+"$")
            globlist = glob.glob(glob_pattern, recursive=True)
            found=0
            for f in globlist:
                #print(f)
                if (p.match(os.path.basename(f))):
                    if os.path.islink(f):
                        info("ignoring source file symlink: "+f)
                    else:
                        found +=1
                        debug("found: "+f+".")
                        problem_directory = os.path.dirname(f)
                        problem_src = os.path.basename(f)
                    
            if (found > 1):
                error("Cannot continue unattended, more than one file found")
            if (found < 1):
                error("Cannot continue, file not found: "+specific)
            env['DEFAULTS']="yes"   
            info("Unattended run for "+os.path.abspath(problem_directory)+"/"+ \
            problem_src)
            info("Unattended projects at "+PROJECTS)
            return (os.path.abspath(problem_directory), problem_src, PROJECTS, sd)

    return (None,None,PROJECTS, sd)
def print_green(message):
    print(color("green")+message+color("default"))
    return
def print_bgreen(message):
    print(color("b-green")+message+color("default"))
    return
def print_blue(message):
    print(color("blue")+message+color("default"))
    return
def print_bblue(message):
    print(color("b-blue")+message+color("default"))
    return
def print_bold(message):
    print(color("bold")+message+color("default"))
    return
def warn(message):
    print(color("b-red")+"*** Warning: "+color("b-blue")+message+color("default"))
    #time.sleep(1)
    return
def error(message):
    print(color("b-red")+"*** Error: "+message+color("default"))
    sys.exit(1)
    return
def info(message):
    print(color("b-red")+"* "+color("b-blue")+message+color("default"))
    return
def enter_debug():
    print(color("yellow"))
    return
def exit_debug():
    print(color("default"))
    return
def debug(message):
    print(color("yellow")+"DBG> "+message+color("default"))
    return

def get_overwrite(argv):
    for arg in argv:
        if ("--overwrite" in arg[0:len("--overwrite")]):
            warn("Forcing creation of new project...")
            return "yes"            
    for arg in argv:
        if ("--run" in arg[0:len("--run")]):
            warn("Short circuit to run...")
            return "no"  
    return ""

def input_index_f(argv):
    for arg in argv:
        if ("--input=" in arg[0:len("--input=")]):
            print("input from "+arg)
            index_s = arg[len("--input="):len("--input=")+4]
            print("Input index has been specified to "+ index_s)
            return index_s
    return None




def choose_problem(source_dir):
    info("Now searching "+source_dir+" for all .cc files... (CTRL-C to cancel)")

    glob_pattern=source_dir+"/**"
    # Find problems:
    # Find cc files within src directory (excluding PROJECTS)
    globlist = glob.glob(glob_pattern, recursive=True)
    p = re.compile('.*cc$')
    j=0
    problems=[]
    directory=[]
    for f in globlist:
        if os.path.islink(f):
            continue
        if (p.match(f)):
#            print("globbed: "+f)
            g=f.split('/')
            problems.append(g[-1])
#            print("problem="+problems[j])
            directory.append("")
            for i in range(0, len(g)-1):
                directory[j] += g[i]
                directory[j] += "/"
#            print("directory="+directory[j])
            j+=1

    p = re.compile('.*input$')
    input_choice = 0
    multiple_input = 0
    input_list = []
    input_index = -1
    i=0
    info("Available problems:")
    for filename in problems:
        print_bold(directory[i]+filename+" ["+str(i)+"]")
        input_list.append(filename)
        i=i+1
    input_index_s=input(color("b-red")+"> "+color("default")+color("green")+ \
    "Select problem to configure [0]: "+color("default"))
    if (input_index_s == ""):
        input_index = 0
    else:
        input_index = int(input_index_s)
    problem_src = input_list[input_index]
    problem_directory = directory[input_index]

    info("Selected problem: "+problem_directory+problem_src)

    return (problem_directory, problem_src)

def configure_project(argv, problem_directory, problem_src, default_directives, \
PROJECTS, dirTag, query ):
    env={}
    env.update(os.environ)
    # slicing
    sublength=len(problem_src)-3
    module_name=problem_src[:sublength] + dirTag

    # if unattended, skip module name modifications

    if query:
        info("Project name is the same thing as the dune-module name")
        #module_name=input("Module name ["+module_name+"]:")
        module_name=input(color("green")+"Module name ["+module_name+"]:"+ \
        color("default"))
        if (module_name == ""):
            module_name=problem_src[:sublength] + dirTag
      
        print(color("default"))
    else:
        info("Using default project name: "+module_name)
        time.sleep(1)
    module = module_name
    warn("module_name is "+module_name)
    
    debug("CXXFLAGS=\""+default_directives+"\"")
    env['CXXFLAGS']=default_directives
    env['SRC_DIR']=problem_directory
    env['SRC_FILE']=problem_src[:sublength]
    env['PROJECTS']=PROJECTS
    env['LOCAL_INCLUDE']=os.path.dirname(os.path.abspath(argv[0]))+"/include"
    env['OVERWRITE'] = get_overwrite(argv)
    
#    command=os.path.dirname(argv[0])+"/_project "+module+" all "
#   we look for bash script in the same directory from which this file was imported:
    command=sys.path[0]+"/_project "+module+" all "
    info("--------------------------- configuring dune project...")
#    print("Source directory: "+problem_directory+"\n")
    debug("Detached process: "+command)
#    enter_debug()
    pid=os.fork()
    if pid==0:
        args=["/bin/bash", "-c", command]
        os.execvpe("/bin/bash", args, env) 
        os._exit(123)

    os.waitpid(pid, 0)
#    exit_debug()
    info("--------------------------- dune project configure done.")
    return module

def monitor(rtime_dir, infile, pname):
    env={}
    env.update(os.environ)

    gnuplot=shutil.which("gnuplot")
    updateDelay=5

    if (gnuplot == None):
        warn("gnuplot is not available on this system!")

        return;
    else:
        print("INFO> Following simulation with gnuplot and of ",updateDelay, "seconds...\n")
    gnuplot_dir=rtime_dir+"/gnuplot"
    try:
        os.stat(gnuplot_dir)
    except:
        os.makedirs(gnuplot_dir)
    gnuplot_dat = rtime_dir+"/vtk/gnuplot.dat"

    with open (gnuplot_dir+'/loop.scr', 'w') as f:
        f.write("\
while (1) {\n\
    reset\n\
    load \""+gnuplot_dir+"/multiplot.scr\"\n\
    pause 5\n\
}\n\
")
    with open (gnuplot_dir+'/create_png.scr', 'w') as f:
        f.write("\
    reset\n\
    set terminal pngcairo \n\
    set output \""+gnuplot_dir+"/monitor.png\"\n\
    load \""+gnuplot_dir+"/multiplot.scr\"\n\
    unset output\n\
")
    fixedTitle = infile.replace("_","-")
    with open (gnuplot_dir+'/multiplot.scr', 'w') as f:
        f.write("\
file9 = \""+gnuplot_dat+"\"\n\
set multiplot layout 1,2 title \""+fixedTitle+"\" font \",14\"\n\
unset key \n\
set grid\n\
hours(x) = x/3600\n\
f(x) = 1/x\n\
set title \"Recuperacion acumulada\"\n\
set xlabel \"Horas\"\n\
set ylabel \"Por ciento del volumen inicial\"\n\
plot  file9 using (hours($2)):($7) with linespoints  \n\
set title \"Convergencia\"\n\
set xlabel \"Paso\"\n\
set ylabel \"1/T\"\n\
set logscale y\n\
plot file9 using ($1):(f($3)) with lines\n\
unset logscale y\n\
unset multiplot \n\
")

    debug("gnuplot="+gnuplot)
    debug("gnuplot_dir="+gnuplot_dir)
    pid=os.fork()
    if pid==0:
        warned = 0
        while (not os.path.isfile(gnuplot_dat)):
            if (not warned):
                print("Waiting for "+gnuplot_dat)
                warned = 1
            time.sleep(5) 
        print("Found "+gnuplot_dat)
        args=[gnuplot, gnuplot_dir+'/loop.scr']
        print("Exec "+gnuplot+" "+gnuplot_dir+'/loop.scr')
        os.chdir(gnuplot_dir)
        args=[gnuplot, 'loop.scr']
        os.execvpe(gnuplot, args, env) 
        os._exit(123)
    info("Detached gnuplot pid is "+str(pid))
    return pid



def get_parameter(line, group):
#  if("Name" in line):
#    return ""
  if "#" in line[0:1]:
    return ""
  retval = ""
  parts=line.split("=")
  a=parts[1] 
# on modification of input parameter, do not lose any trailing comments.
  b=a.split("#")
#  print("len(b)=",len(b))
#       print("code:"+line)
  if (len(b) <2):
    b1 = " "
  else:
    b1 = b[1]
  print(color("b-blue")+"# "+group+" "+color("b-red")+b1+color("default"))
  new_value=input(color("bold")+parts[0]+"["+color("green")+b[0]+color("default")+ \
    color("bold")+"] = "+color("default"))
  if new_value == "":
    retval = ""
  else:
    retval = parts[0]+" = "+new_value+"  #"+b1+"\n"
  return retval

def get_group(line):
    if "[" in line[0:1]:
        a=line.replace("[", " ")
        group=a.replace("]", " ")
        return group
    return None

def set_parameters(path):
    print_green("Setting runtime parameters in file: "+color("b-magenta")+path)
#    print("parsing ",path) 
    f=open(path, 'r')
    content=f.read()
    f.close()

    #print(content)
    lines=content.split("\n")
    new_content=""
    item=1;
    group = "NoGroup"
    for line in lines:
         writ=0
         newline = ""
         newgroup = get_group(line)
         if newgroup != None:
            group = newgroup
         if "#" in line[0:1]:
            print(color("b-magenta")+line+color("default"))
         if ("=" in line):
            newline = get_parameter(line, group)
         if (newline == ""):
            newline = line
         new_content += newline
         if (not "\n" in line and item < len(lines)):
            new_content += "\n"
         # hack to remove last line added by python (array end item)
         item+=1
#    print(new_content)

    output = subprocess.check_output(['cp', '-v', path, path+".bak"])
    f=open(path, 'w')
    f.write(new_content)
    f.close();
    return None

def select_input_file(argv, problem_directory):
    if ("--only-compile" in  argv[1:]):
        return None
    input_list = []
    input_index = -1
    debug("Now reading contents of "+problem_directory)


    listdir=glob.glob(problem_directory+"/*.input")
    if len(listdir)<1:
        error("*** Error: no input files found at "+problem_directory)
        sys.exit(1)
    if len(listdir)==1:
        warn("...using single input file: "+listdir[0])
        return listdir[0]
    info("Multiple input files found. Select one:")
    i=0
    for filename in listdir:
        print_bold(filename+" ["+str(i)+"]")
        input_list.append(filename)
        i=i+1
    input_index_s = input_index_f(argv)
    if (input_index_s == None):
        input_index_s=input(color("green")+"Input file [0]: "+color("default"))
    #print_green("got index_s="+input_index_s)


    if (input_index_s == ""):
        input_index = 0
    else:
        input_index = int(input_index_s)
    info("...user defined input file: "+input_list[input_index])
    return input_list[input_index]

def get_directives(argv, available_directives, defaults):
    dirTag=""
    if (os.environ.get('CXXFLAGS') == None):
        default_directives="";
    else:
        default_directives=os.environ["CXXFLAGS"]
    directive_count=0
    for arg in argv:
#        debug("arg="+arg)
        if "--D" in arg[0:3]:
            found=0
            for d in available_directives:
                if arg[3:99] == d:
                    found=1
                    directive_count = directive_count+1;
            if found==0:
                warn("Specificied directive \""+arg[3:99]+ \
                "\" is not in available_directives list (adding anyway).")
            #    sys.exit(1)
            info("Adding compilation directive: ")
            print_bblue(arg)
            if found:
                print_green("   --> "+available_directives[arg[3:]])
            default_directives = default_directives + " "+arg[1:99]
            dirTag = dirTag + "-" + arg[3:99]
    if directive_count > 0 and directive_count < len(defaults):
            warn("Less than "+str(len(defaults)) + \
            " directives specified: using source code defaults for the rest.")
    warn("Adding default directives:")
    for d in defaults:
        print_bblue("--D"+d)
        print_green("   --> "+available_directives[d])
        default_directives = default_directives + " -D"+d
        dirTag = dirTag + "-" + d

#    if (dirTag == ""):
#        warn("No specific compilation directives specified, using defaults:")
#        for d in defaults:
#            print_bblue("--D"+d)
#            print_green("   --> "+available_directives[d])
#            default_directives = default_directives + " -D"+d
#            dirTag = dirTag + "-" + d

    debug("Creating program with the following directives: "+ default_directives)
    debug("dirTag is "+ dirTag)
    print(color("b-magenta")+"Use "+color("b-red")+"--help"+color("default") + \
    color("b-magenta")+" for full instructions and directives"+color("default"))
    pause=1;
    for arg in argv:
        if ("--run" in arg[0:len("--run")]):
            warn("Short circuit to run...")
            pause=0
    if pause==1:
        input(color("b-blue")+"If this is not correct type "+color("b-red")+\
        "CTRL-C"+color("b-blue")+" to abort"+color("default"))
        print("")
    return (dirTag, default_directives)

def make(argv, program_dir, module_name, default_directives):
    env={}
    env.update(os.environ)
    path=os.environ["PATH"]
    path=path+":/opt/dune/bin"
    os.environ["PATH"]=path
    debug("PATH ="+os.environ["PATH"])
    # remove build-cmake/
    pid=os.fork()
    if pid==0:
       command = 'rm -rf '+program_dir+'/build-cmake'
       args=["/bin/bash", "-c", command]
       os.execvpe("/bin/bash", args, env) 
       os._exit(123)
    os.waitpid(pid, 0)
    
    os.chdir(program_dir)

#   configure step     
    info(" --------------------------- configuring templates...")
    pid=os.fork()
    if pid==0:
     if ("--verbose" in argv[1:]):
       command = "dunecontrol --only=" +module_name+ " configure"+" >/dev/null"
     else:  
       command = "dunecontrol --only=" +module_name+ " configure"+ \
       " >/dev/null 2>>/dev/null"
     args=["/bin/bash", "-c", command]
     os.execvpe("/bin/bash", args, env) 
     os._exit(123)

    os.waitpid(pid, 0)
    info(" --------------------------- configure done, now at make step...")

    pid=os.fork();
    if pid==0:
     args=["dunecontrol", "--only="+module_name, "make"]
     os.execvpe("dunecontrol", args, env) 
     os._exit(123)

    os.waitpid(pid, 0)
    info(" --------------------------- make done.")

def mpi_command(argv, c):
    if ("-DAMG" in argv[1:]):
        # Use MPI with AMG
        for arg in argv[1:]:
            if "MPI" in arg[1:3]:
                n = arg[4:]
                return "mpirun --np "+n+" "+c
    else:
        warn("MPI execution is only supported with -DAMG directive. Default is -DUMF")
    return c

def run_in_background(command):
    info("Detached process: "+command)
    pid1=os.fork();
    if pid1==0:
        args=["/bin/bash", "-c", command]
        os.execvp(args[0], args) 
        os._exit(123)
    return pid1

def tail_logfile(program_name):
    command = "/usr/bin/tail -f "+ "../"+program_name+".log |grep \"Time step\""
    info(" "+command)
    pid2=os.fork();
    if pid2:
        return pid2 
    #subprocess.run(["/bin/bash", "-c", command]) 
    #return    
    args=["/bin/bash", "-c", "/usr/bin/tail -f "+ "../"+program_name+ \
    ".log |grep \"Time step\""]
    #args=["/usr/bin/tail","-f","../"+program_name+".log"]
    os.execvp(args[0], args) 
    os._exit(123)
    return

#################################  main program
def main(argv, available_directives, defaults):
    if ("--help" in  argv[1:]):
        help(argv, available_directives, defaults)
    ########################### environment
    env={}
    env.update(os.environ)
    path=os.environ["PATH"]
    path=path+":/opt/dune/bin"
    os.environ["PATH"]=path

    (dirTag, default_directives)=get_directives(argv, available_directives, defaults)
    (problem_directory, problem_src, PROJECTS, sd) = get_specific_problem(argv)

    query = 0
    if problem_src == None:
        env['DEFAULTS'] = "no"
        (problem_directory, problem_src) = choose_problem(sd)
        query = 1
    else:
        env['DEFAULTS'] = "yes"

    module = configure_project(argv, problem_directory, problem_src, \
        default_directives, PROJECTS, dirTag, query)
    program_dir = PROJECTS + "/" + module
    sublength=len(problem_src)-3
    program_name = problem_src[:sublength]
    executable_dir=program_dir+"/"+"/build-cmake/src"
    if env['DEFAULTS'] == "yes":
        input_file = problem_directory+"/"+program_name+".input"
        try:
            os.stat(input_file)
            info("Using default input file: "+input_file)
        except:
            input_file = select_input_file(argv, problem_directory)
    else:
        input_file = select_input_file(argv, problem_directory)

    if ("--only-compile" in  argv[1:]):
        input_file="None"
    else:
        if ("--query" in argv[1:]):
            set_parameters(input_file)
        infile = os.path.basename(input_file)
        gs = infile.replace(".input", "")
        default_output_dir = program_dir+"/"+ gs +".output"
        ngs = gs.replace("_","-")
        gs = ngs.replace("_","-")
        gnuplot_subtitle = gs + dirTag  

    debug("program_dir: "+program_dir)
    debug("module name: "+module)
    debug("program name: "+program_name)
    debug("problem_directory: "+problem_directory)
    debug("problem_src: "+problem_src)
    debug("executable_dir: "+problem_src)
    debug("input_file: "+input_file)

    debug("default_directives: "+default_directives)
    debug("dirTag: "+dirTag)

    if ("--only-compile" in  argv[1:]):
        run_time_dir="/tmp/test"
    else:
        if env['DEFAULTS'] == "yes":
            run_time_dir = ""
        else:    
            run_time_dir=input(color("green")+"Output directory ["+ \
            default_output_dir+"]:"+color("default"))
        if (run_time_dir == ""):
            run_time_dir=default_output_dir

        
    if ("--no-clean" in argv[1:]):
        debug("Not cleaning "+run_time_dir+" ...")
    else:
        debug("Cleaning "+run_time_dir+ "...")
        output = subprocess.check_output(['rm', '-rf',run_time_dir])

    if (not "--only-compile" in argv[1:]):
        vtk_dir=run_time_dir+"/vtk"
        try:
            os.stat(vtk_dir)
        except:
            os.makedirs(vtk_dir)


    if (not os.path.isfile(executable_dir+'/'+program_name) or "--only-compile" \
        in argv[1:]):
        make(argv, program_dir, module, default_directives)
        if ( "--only-compile" in argv[1:]):
            info("--only-compile complete.")
            sys.exit()

    # Now prepare to run the program
    # copy input file if it does not exist in runtime directory...
    if (not os.path.isfile(run_time_dir+'/'+os.path.basename(input_file))):
        output = subprocess.check_output(['cp', '-v', input_file, run_time_dir+ \
        '/'+os.path.basename(input_file)])
        debug(output.decode("ascii"))

    # And run the program...
    os.chdir(vtk_dir)
    info("Running from "+vtk_dir+"...")
    # We will output stdout to a new log file 
    if os.path.isfile("../"+program_name+".log"):
        os.remove("../"+program_name+".log")
    open("../"+program_name+".log", 'w').close

    command = executable_dir+"/"+program_name+ " -ParameterFile ../"+ \
        os.path.basename(input_file)+" > ../"+program_name+".log"
    for arg in argv:
        if ("--gdb" in arg):
            env={}
            env.update(os.environ)
            command = "gdb -cd="+ vtk_dir + " " + executable_dir+"/"+ \
            program_name + " -ex \"set args -ParameterFile ../" + \
            os.path.basename(input_file)+"\""
            print("Command = \""+command+"\"")
            args=["/bin/bash", "-c", command]
            os.execvpe("/bin/bash", args, env) 
            os._exit(123)

    # Modify with mpirun if specified on command line
    command = mpi_command(argv, command)

    pid_run=run_in_background(command)

    if ("--plot" in argv[1:]):
        pid_gnuplot=monitor(run_time_dir, gnuplot_subtitle, program_name)
    else:
        pid_gnuplot=0;

    pid_tail=tail_logfile(program_name)
    os.waitpid(pid_run, 0)

    if (pid_gnuplot != 0):
        gnuplot=shutil.which("gnuplot")
        if (gnuplot != None):
          while (1):
            answer=input(color("b-red")+"Finish process? [Y]/N: "+color("default"))
            if (answer == "Y" or answer == "y" or answer == "N" or answer == "n"):
                if (answer == "Y" or answer == "y"):
                    os.kill(pid_gnuplot,signal.SIGKILL)
                if (answer == "N" or answer == "n"):
                    info("Please note that process "+str(pid_gnuplot)+ \
                    " is still active.\n")
                print("killing tail subprocess: "+str(pid_tail))
                os.killpg(os.getpgid(pid_tail), signal.SIGKILL)
                os.waitpid(pid_tail, 0)
                sys.exit()
            print_green("Please answer Y or N\n")
    info("Run complete.\n")
    
    print("killing tail subprocess: "+str(pid_tail))
    os.killpg(os.getpgid(pid_tail), signal.SIGKILL)
    os.waitpid(pid_tail, 0)
    return None


