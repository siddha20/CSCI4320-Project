import os
import sys


if len(sys.argv) != 2 and len(sys.argv) != 1:
    print("Usage: python run.py <delete>")
    exit()


delete_files = True if len(sys.argv) == 2 and sys.argv[1] == 'delete' else False
project_directory = os.popen('cd .. ; pwd').read().replace(" ", "\\ ").replace("\n", "")
exec_directory = os.path.join(project_directory, 'build/bin')
data_directory = os.path.join(project_directory, 'aimos/data')
output_directory = os.path.join(project_directory, 'aimos/output')
slurm_directory = os.path.join(project_directory, 'aimos/slurm')
batch_directory = os.path.join(project_directory, 'aimos/batch')


batch_args = {
    'gpus': 0,
    'time_limit': 10,
    'partition': "",
    'script_path': ""
}

def file_exists(filename):
    return os.popen(f'[ -f {filename} ] && echo 1 || echo 0').read().replace("\n", "") == '1'


os.system(f'mkdir -p {output_directory}')

os.system(f'mkdir -p {slurm_directory}')
os.system(f'rm -r {slurm_directory}')
os.system(f'mkdir {slurm_directory}')

os.system(f'mkdir -p {batch_directory}')
os.system(f'rm -r {batch_directory}')
os.system(f'mkdir {batch_directory}')

os.system(f'mkdir -p {data_directory}')
# os.system(f'rm -r {data_directory}')
# os.system(f'mkdir {data_directory}')

def make_slurm_script(filename, ranks, exec_path, exec_args, exec_output):
    if (file_exists(filename)):
        os.system(f'echo "taskset -c 0-159:4 mpirun -N {ranks} --oversubscribe {exec_path} {exec_args} > {exec_output}"'
                  f'>> {filename}')
    else:
        os.system(f'touch {filename}')
        os.system(f'echo "#!/bin/bash -x\n'
                  f'module load spectrum-mpi\n'
                  f'taskset -c 0-159:4 mpirun -N {ranks} --oversubscribe {exec_path} {exec_args} > {exec_output}"'
                  f'> {filename}')

def make_slurm_script_not_aimos(filename, ranks, exec_path, exec_args, exec_output):
    if (file_exists(filename)):
        os.system(f'echo "mpirun -N {ranks} --oversubscribe {exec_path} {exec_args} > {exec_output}"'
                  f'>> {filename}')
    else:
        os.system(f'touch {filename}')
        os.system(f'echo "#!/bin/bash -x\n'
                  f'mpirun -N {ranks} --oversubscribe {exec_path} {exec_args} > {exec_output}"'
                  f'> {filename}')

node_and_rank_counts = [
    #(1, 1),   # 1 rank
    #(1, 2),   # 2 ranks
    (1, 4),   # 4 ranks
    (1, 8),   # 8 ranks 
    (1, 16),  # 16 ranks
    (1, 32),  # 32 ranks
    #(2, 24),  # 48 ranks
    (2, 32),  # 64 ranks
    #(3, 32),  # 96 ranks
    (4, 32),  # 128 ranks
    #(5, 32),  # 160 ranks
    (6, 32),  # 192 ranks
    #(7, 32),  # 224 ranks
    (8, 32)  # 256 ranks
    #(9, 32),  # 288 ranks
    #(10, 32), # 320 ranks
    #(11, 32), # 352 ranks
    #(12, 32), # 384 ranks
    #(13, 32), # 416 ranks
    #(14, 32), # 448 ranks
    #(15, 32), # 480 ranks
    #(16, 32)  # 512 ranks
]

candidate_and_vote_counts = [
    (50000, 20),
    (75000, 20),
    (100000, 20),
    (125000, 20),
    (150000, 20),
    (175000, 20),
    (200000, 20),
    (225000, 20),
    (250000, 20),
    (275000, 20),
    (300000, 20),
    (325000, 20),
    (350000, 20),
    (375000, 20),
    (400000, 20),
    (425000, 20),
    (450000, 20),
    (475000, 20),
    (500000, 20),
    (525000, 20),
    (550000, 20),
    (575000, 20),
    (600000, 20),
    (625000, 20),
    (650000, 20),
    (675000, 20),
    (700000, 20),
    (725000, 20),
    (750000, 20),
    (775000, 20),
    (800000, 20),
    (825000, 20),
    (850000, 20),
    (875000, 20),
    (900000, 20),
    (925000, 20),
    (950000, 20),
    (975000, 20),
    (1000000, 20)
]



# Create batch shell script for vote gen.
vote_gen_batch_filename = os.path.join(batch_directory, 'batch-vote-gen.sh')
vote_gen_data_directory = os.path.join(data_directory, 'vote-gen')
gpu_count = 1
time_limit = 10
partition = 'el8'
os.system(f'touch {vote_gen_batch_filename}')
os.system(f'mkdir -p {vote_gen_data_directory}')

# Create all the slurm files and append them to the bach shell script.
for node, ranks in node_and_rank_counts:

    for candidate_count, vote_count in candidate_and_vote_counts:

        # vote-gen specific args
        dist = 'UNIFORM'
        dist_arg = ''
        delete_arg = 'DELETE' if delete_files else ''

        # Make directory for vote-gen slurm files
        vote_gen_slurm_dir = os.path.join(slurm_directory, f'vote-gen-{candidate_count}-{vote_count}')
        vote_gen_output_dir = os.path.join(output_directory, f'vote-gen-{candidate_count}-{vote_count}')
        os.system(f'mkdir -p {vote_gen_slurm_dir}')
        os.system(f'mkdir -p {vote_gen_output_dir}')

        # Set some names up
        vote_gen_filename = f'vote-gen-{node}-{ranks}-{candidate_count}-{vote_count}.txt'
        exec_path = os.path.join(exec_directory, 'vote-gen')
        data_file = os.path.join(vote_gen_data_directory, vote_gen_filename)
        slurm_filename = os.path.join(vote_gen_slurm_dir, f'slurm-{node}.sh')
        
        # Add slurm file to batch script
        if(not file_exists(slurm_filename)):
            os.system(f'echo "sbatch -N {node} --gres=gpu:{gpu_count} -t {time_limit} --partition={partition} {slurm_filename}"'
                    f'>> {vote_gen_batch_filename}')

        # Set up executable stuff
        exec_args = f'{data_file} {candidate_count} {vote_count} {dist} {dist_arg} {delete_arg}'
        exec_output = os.path.join(vote_gen_output_dir ,vote_gen_filename)
        make_slurm_script(slurm_filename, ranks, exec_path, exec_args, exec_output)

# Create batch shell script for vote gen.
vote_algo_batch_filename = os.path.join(batch_directory, 'batch-vote-algo.sh')
vote_algo_data_directory = os.path.join(data_directory, 'vote-algo')
gpu_count = 1
time_limit = 10
partition = 'el8'
os.system(f'touch {vote_algo_batch_filename}')
os.system(f'mkdir -p {vote_algo_data_directory}')

# Create all the slurm files and append them to the bach shell script.
for node, ranks in node_and_rank_counts:

    for candidate_count, vote_count in candidate_and_vote_counts:

        # vote-gen specific args
        input_data_file = os.path.join(vote_gen_data_directory, f'vote-gen-{node}-{ranks}-{candidate_count}-{vote_count}.txt')

        if not file_exists(input_data_file):
            print(f'{input_data_file} does not exist!')

        # Make directory for vote-algo slurm files
        vote_algo_slurm_dir = os.path.join(slurm_directory, f'vote-algo-{candidate_count}-{vote_count}')
        vote_algo_output_dir = os.path.join(output_directory, f'vote-algo-{candidate_count}-{vote_count}')
        os.system(f'mkdir -p {vote_algo_slurm_dir}')
        os.system(f'mkdir -p {vote_algo_output_dir}')

        # Set some names up
        vote_algo_filename = f'vote-algo-{node}-{ranks}-{candidate_count}-{vote_count}.txt'
        exec_path = os.path.join(exec_directory, 'vote-algo')
        data_file = os.path.join(vote_algo_data_directory, vote_algo_filename)
        slurm_filename = os.path.join(vote_algo_slurm_dir, f'slurm-{node}.sh')
        
        # Add slurm file to batch script
        if(not file_exists(slurm_filename)):
            os.system(f'echo "sbatch -N {node} --gres=gpu:{gpu_count} -t {time_limit} --partition={partition} {slurm_filename}"'
                    f'>> {vote_algo_batch_filename}')

        # Set up executable stuff
        exec_args = f'{input_data_file}'
        exec_output = os.path.join(vote_algo_output_dir ,vote_algo_filename)
        make_slurm_script(slurm_filename, ranks, exec_path, exec_args, exec_output)

os.system(f'chmod -R 777 .')