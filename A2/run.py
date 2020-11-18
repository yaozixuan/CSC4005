import subprocess
import time
import os

program = str(input("Which program do you want to run?\n\tMPI\t\tPthread\t\tBoth\n> "))
times = int(input("How many times do you want to run?\n> "))
num_workers = str(input("How many processes/threads do you want to run?\n> "))

load_command = "module load openmpi-3.1.2-gcc-8.2.0-qgxgzyn"
MPI_run_command = "mpiexec -oversubscribe -np " + num_workers + " MS_MPI"
Pthread_run_command = os.path.join(".", "MS_Pthread") + " " + num_workers
    
if(program == "MPI"):
    MPI_file_name = str(input("The MPI program that you want to execute (with extension):\n> ") or "MPI_mandelbrot_set.c") 
    MPI_compile_command = "mpicc -o MS_MPI " + MPI_file_name + " -lX11"
    subprocess.call(load_command, shell=True)
    subprocess.call(MPI_compile_command, shell=True)
    for i in range(times):
        print("Experiment " + str(i + 1))
        print(MPI_run_command)
        subprocess.call(MPI_run_command, shell=True)
        time.sleep(1)
elif (program == "Pthread"):
    Pthread_file_name = str(input("The Pthread program that you want to execute (with extension):\n> ") or "Pthread_mandelbrot_set.c") 
    Pthread_compile_command = "gcc -o MS_Pthread " + Pthread_file_name + " -lpthread -lX11"
    subprocess.call(Pthread_compile_command, shell=True)
    for i in range(times):
        print("Experiment " + str(i + 1))
        print(Pthread_run_command)
        subprocess.call(Pthread_run_command, shell=True)
        time.sleep(1)
elif (program == "Both"):
    MPI_file_name = str(input("The MPI program that you want to execute (with extension):\n> ") or "MPI_mandelbrot_set.c") 
    Pthread_file_name = str(input("The Pthread program that you want to execute (with extension):\n> ") or "Pthread_mandelbrot_set.c") 
    MPI_compile_command = "mpicc -o MS_MPI " + MPI_file_name + " -lX11"
    Pthread_compile_command = "gcc -o MS_Pthread " + Pthread_file_name + " -lpthread -lX11"
    subprocess.call(load_command, shell=True)
    subprocess.call(MPI_compile_command, shell=True)
    subprocess.call(Pthread_compile_command, shell=True)
    for i in range(times):
        print("Experiment " + str(i + 1))
        print(MPI_run_command)
        subprocess.call(MPI_run_command, shell=True)
        time.sleep(0.1)
        print(Pthread_run_command)
        subprocess.call(Pthread_run_command, shell=True)
        time.sleep(0.1)
