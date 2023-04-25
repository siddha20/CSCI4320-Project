import os
import sys
import matplotlib.pyplot as plt

'''
for i in <dir to aimos stats> ; do  python3 analyze.py $i ; done
for i in ../aimos-stats/* ; do  python3 analyze.py $i ; done
'''

# print(plt.style.available)
plt.style.use(['ggplot'])
plt.rcParams['figure.figsize'] = [6, 3]


CLOCK_FREQ = 512000000

if len(sys.argv) != 2 and len(sys.argv) != 3:
    print("Usage: python analyze.py <data directory> <all graph label to use>")
    exit()

gen_all_graph = False
all_graph_label = None
if len(sys.argv) == 3:
    gen_all_graph = True
    all_graph_label = sys.argv[2]

output_directory = sys.argv[1]
project_directory = os.popen('cd .. ; pwd').read().replace(" ", "\\ ").replace("\n", "")

rank_labels = {}
for name in os.listdir(output_directory):
    plot_directory =  f'./plots/{name}'
    data_directory = os.path.join(output_directory, name)
    
    name_data = name.split('-')[2:]
    # print(name_data)
    
    os.system(f'mkdir -p {plot_directory}')


    labels = {}
    data = os.popen(f"cat {data_directory}/*").read() 
    data = [x.split() for x in data.splitlines()] 

    for entry in data:
        # print(entry)
        label = entry[0].split(':')
        # print(label)
        if not label[1] in labels.keys(): labels[label[1]] = [[],[]]
        labels[label[1]][0].append(float(label[0]))
        labels[label[1]][1].append(float(entry[1])/CLOCK_FREQ)

        if not label[0] in rank_labels.keys(): rank_labels[label[0]] = [[],[]]
        
        if gen_all_graph:
            if (label[1] == all_graph_label):
                rank_labels[label[0]][0].append(float(name_data[0]))
                rank_labels[label[0]][1].append(float(entry[1])/CLOCK_FREQ)

    colors = ['r', 'b', 'g']
    i = 0
    for key in labels.keys():
        x = sorted(labels[key][0])
        y = [y for _, y in sorted(zip(labels[key][0], labels[key][1]))]
        plt.figure()
        plt.title(f"{key.replace('_', ' ').title()} versus Rank Count")
        plt.xlabel("Ranks")
        plt.ylabel("Time (sec)")
        plt.scatter(x, y, color=colors[i%len(colors)])
        plt.plot(x, y, label=name, color=colors[i])
        plt.legend()
        plt.savefig(f'{plot_directory}/{key}', bbox_inches='tight')
        plt.close()
        i += 1

if gen_all_graph:
    plt.figure()
    for key in rank_labels.keys():
        if (int(key) in [1]): continue # skip these ranks
        x = sorted(rank_labels[key][0])
        y = [y for _, y in sorted(zip(rank_labels[key][0], rank_labels[key][1]))]
        plt.title(f'{all_graph_label.replace("_", " ").title()} versus Vote Count')
        plt.xlabel('Vote Count')
        plt.ylabel('Time (sec)')
        plt.scatter(x, y)
        plt.plot(x, y, label=f'{key} ranks')
        plt.legend()
    plt.savefig(f'./plots/{all_graph_label}', bbox_inches='tight')
    plt.close()
