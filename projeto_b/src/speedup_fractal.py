import subprocess
import re

import matplotlib.pyplot as plt

def run_mandelbrot(thread_count, runs=5):
    serial_times = []
    thread_times = []

    for _ in range(runs):
        result = subprocess.run(
            ["../build/mandelbrot", "-t", str(thread_count)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        output = result.stdout

        serial_match = re.search(r"\[mandelbrot serial\]:\s+\[([\d.]+)\] ms", output)
        thread_match = re.search(r"\[mandelbrot thread\]:\s+\[([\d.]+)\] ms", output)

        if serial_match and thread_match:
            serial_times.append(float(serial_match.group(1)))
            thread_times.append(float(thread_match.group(1)))

    avg_serial_time = sum(serial_times) / len(serial_times) if serial_times else None
    avg_thread_time = sum(thread_times) / len(thread_times) if thread_times else None

    return avg_serial_time, avg_thread_time

def main():
    thread_counts = range(1, 25)
    serial_times = []
    thread_times = []

    for thread_count in thread_counts:
        avg_serial, avg_thread = run_mandelbrot(thread_count)
        serial_times.append(avg_serial)
        thread_times.append(avg_thread)

    eficiency = []
    for i in range(len(thread_times)):
        eficiency.append(serial_times[i] / thread_times[i])
    print("Eficiencia: ")
    for i in range(len(thread_counts)):
        print(f"Threads: {thread_counts[i]}, Eficiencia: {eficiency[i]}")

    speedup = [serial / thread if thread else 0 for serial, thread in zip(serial_times, thread_times)]

    plt.figure(figsize=(6, 5))
    plt.plot(thread_counts, speedup, label="Speedup Fractal Generation", marker='o', color='orange')
    plt.xlabel("Número de threads")
    plt.ylabel("Speedup")
    plt.title("Mandelbrot Speedup vs Número de Threads")
    plt.legend()
    plt.grid(True)
    plt.show()

    plt.savefig("speedup_fractal.png")
    print("Speedup plot saved as speedup_fractal.png")

if __name__ == "__main__":
    main()