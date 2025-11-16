import matplotlib.pyplot as plt
import csv


# for every thread count, plot grid size vs avg_time
thread_counts = {}
with open("timing.csv") as f:
    reader = csv.reader(f, delimiter=",")
    headers = next(reader)
    for row in reader:
        num_threads = int(row[0])
        
        if num_threads not in thread_counts.keys():
            thread_counts[num_threads] = ([],[], []) # x,y1,y2 where x=grid_size and y1=avg_time, and y2=speedup
        thread_counts[num_threads][0].append(int(row[1]))
        thread_counts[num_threads][1].append(float(row[2]))
        thread_counts[num_threads][2].append(float(row[3]))

times = [tc[1] for tc in thread_counts.values()]
if False:
    plt.xlabel('Grid Size')
    plt.ylabel('Average Time Per Tick')
    for t in thread_counts.keys():
        tc = thread_counts[t]
        line = plt.plot(tc[0], tc[1], label=f'{t} threads')
        # line.set_label(f'{t} threads')
        
    plt.legend()
    plt.savefig('timing.png')

else:
    plt.xlabel('Grid Size')
    plt.ylabel('Speedup')
    for t in thread_counts.keys():
        tc = thread_counts[t]
        plt.plot(tc[0], tc[2], label=f'{t} threads')
    plt.legend()
    plt.savefig('speedup.png')