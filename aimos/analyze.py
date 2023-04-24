import os
import sys
import matplotlib.pyplot as plt

# print(plt.style.available)
plt.style.use(['ggplot'])
plt.rcParams['figure.figsize'] = [6, 3]


CLOCK_FREQ = 512000000

if len(sys.argv) != 2:
    print("Usage: python analyze.py <data directory>")
    exit()

basename = os.path.basename(sys.argv[1])
data_directory = os.path.join(sys.argv[1], '*')
project_directory = os.popen('cd .. ; pwd').read().replace(" ", "\\ ").replace("\n", "")
plot_directory =  f'./plots/{basename}'

os.system(f'mkdir -p {plot_directory}')


labels = {}
data = os.popen(f"cat {data_directory}").read() 
data = [x.split() for x in data.splitlines()] 


for entry in data:
    # print(entry)
    label = entry[0].split(':')
    if not label[1] in labels.keys(): labels[label[1]] = [[],[]]
    labels[label[1]][0].append(float(label[0]))
    labels[label[1]][1].append(float(entry[1])/CLOCK_FREQ)

colors = ['r', 'b', 'g']
i = 0
for key in labels.keys():
    x = sorted(labels[key][0])
    y = [y for _, y in sorted(zip(labels[key][0], labels[key][1]))]
    plt.figure()
    plt.title(key.replace('_', ' '))
    plt.scatter(x, y, color=colors[i%len(colors)])
    plt.plot(x, y, label=basename, color=colors[i])
    plt.legend(loc="upper right")
    plt.savefig(f'{plot_directory}/{key}')
    i += 1
