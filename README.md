# Reader-Writer-Data-Synchronization
•	Developed a multi-threaded reader-writer pipeline using QNX Neutrino API.
•	Implemented efficient synchronization using POSIX mutexes and condition variables to avoid polling and CPU wastage.
•	Designed a shared buffer queue to handle data from multiple writer threads while ensuring each data packet is processed only once by a reader thread.
•	Utilized real-time threading techniques to optimize performance and minimize latency in a concurrent environment.
•	Applied QNX IPC mechanisms to facilitate communication between threads.

