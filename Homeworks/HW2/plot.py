import matplotlib.pyplot as plt

# New data for the larger matrix dimension
processes = list(range(1, 17))
elapsed_time_800 = [1.114914, 0.558598, 0.377714, 0.286607, 0.252366, 0.196394, 0.201921, 
                    0.200495, 0.175918, 0.175085, 0.170041, 0.180150, 0.179151, 
                    0.174796, 0.169956, 0.176048]

# Plotting for the larger matrix (dimension 800)
plt.figure(figsize=(10, 6))
plt.plot(processes, elapsed_time_800, marker='o', linestyle='-', color='r')
plt.xlabel('Number of Processes')
plt.ylabel('Elapsed Time (sec)')
plt.title('Matrix Multiplication Time vs Number of Processes (Matrix Dimension: 800)')
plt.grid(True)
plt.xticks(processes)  # Show all process numbers on the x-axis
plt.show()
