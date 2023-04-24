import os


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
os.system(f'rm -r {data_directory}')
os.system(f'mkdir {data_directory}')

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
    (1, 1),   # 1 rank
    (1, 2),   # 2 ranks
    (1, 4),   # 4 ranks
    (1, 8),   # 8 ranks 
    (1, 16),  # 16 ranks
    (1, 32),  # 32 ranks
    (2, 24),  # 48 ranks
    (2, 32),  # 64 ranks
    (3, 32),  # 96 ranks
    (4, 32),  # 128 ranks
    (5, 32),  # 160 ranks
    (6, 32),  # 192 ranks
    (7, 32),  # 224 ranks
    (8, 32),  # 256 ranks
    (9, 32),  # 288 ranks
    (10, 32), # 320 ranks
    (11, 32), # 352 ranks
    (12, 32), # 384 ranks
    (13, 32), # 416 ranks
    (14, 32), # 448 ranks
    (15, 32), # 480 ranks
    (16, 32)  # 512 ranks
]

candidate_and_vote_counts = [
    # (10000, 20),
    # (1000000, 20),
    # (10000000, 20),
    # (50000000, 20),
    (100000000, 20),
    (500000000, 20),
    (1000000000, 20),
    (5000000000, 20)
    # (1000, 500),
    # (1000, 5000),
    # (1000, 50000)
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
        exec_args = f'{data_file} {candidate_count} {vote_count} {dist} {dist_arg} DELETE'
        exec_output = os.path.join(vote_gen_output_dir ,vote_gen_filename)
        make_slurm_script(slurm_filename, ranks, exec_path, exec_args, exec_output)

os.system(f'chmod -R 777 .')