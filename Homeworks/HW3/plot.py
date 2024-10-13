import matplotlib.pyplot as plt

# Read data from the file
threads = []
times = []

with open('performance.txt', 'r') as f:
    for line in f:
        parts = line.strip().split(',')
        if len(parts) == 2:
            thread_num = int(parts[0].split('#')[1])
            elapsed_time = float(parts[1].split()[1])
            threads.append(thread_num)
            times.append(elapsed_time)

# Create the plot
plt.figure(figsize=(10, 6))
plt.plot(threads, times, marker='o', linestyle='-', color='b')
plt.xlabel('Number of Worker Threads')
plt.ylabel('Elapsed Time (ms)')
plt.title('Worker Thread Performance')
plt.grid(True)
plt.xticks(threads)

# Save the plot
plt.savefig('performance.png')
print("Plot saved as worker_thread_performance.png")