import socket
import threading
import time

SERVER_IP = '127.0.0.1'
SERVER_PORT = 8080
CONNECTIONS = 500  # 更高并发量
MESSAGE = b'x' * 1024
DURATION = 30

results = []
latencies = []

def worker():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER_IP, SERVER_PORT))
        end_time = time.time() + DURATION
        count = 0
        thread_latencies = []
        while time.time() < end_time:
            start = time.time()
            s.sendall(MESSAGE)
            data = s.recv(len(MESSAGE))
            elapsed = time.time() - start
            thread_latencies.append(elapsed)
            if not data:
                break
            count += 1
        s.close()
        results.append(count)
        latencies.extend(thread_latencies)
    except Exception as e:
        print(f"Error: {e}")

threads = []
for _ in range(CONNECTIONS):
    t = threading.Thread(target=worker)
    t.start()
    threads.append(t)

for t in threads:
    t.join()

total = sum(results)
print(f"Total echo count: {total}")
print(f"QPS: {total / DURATION:.2f}")

if latencies:
    print(f"Avg latency: {sum(latencies)/len(latencies)*1000:.2f} ms")
    print(f"Min latency: {min(latencies)*1000:.2f} ms")
    print(f"Max latency: {max(latencies)*1000:.2f} ms")
