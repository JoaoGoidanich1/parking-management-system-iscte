# parking-management-system-iscte
University parking system project using Bash and C (Operating Systems @ ISCTE)

This is a multi-part university project developed during the Operating Systems course at ISCTE.

It simulates a real-world parking management system using:
- Bash scripting (Part 1)
- C programming with signals and logging (Part 2)
- [COMING SOON] Interprocess communication (Part 3 – in progress)

---

## 📁 Project Structure

/part1_bash/ → Bash scripts for registration, maintenance, and statistics
/part2_c/ → C code implementing a client-server model
/part3_ipc/ → (to be added soon)


---

## 🔹 Part 1 – Bash Scripts
Implements a basic parking system simulation using shell scripts:
- `regista_passagem.sh` – Register entry/exit
- `manutencao.sh` – Clean and archive old records
- `stats.sh` – Generate statistics in HTML
- `menu.sh` – Main interface for the operator

---

## 🔹 Part 2 – C Client-Server System
Implements a client-server model in C:
- Handles parking access requests
- Uses signals (SIGUSR1, SIGCHLD, etc.)
- Creates dedicated child processes for each client
- Logs events to file

---

## 🔄 Part 3 – IPC System *(in progress)*
Final version will use:
- Shared memory
- Message queues
- Semaphores  
To support concurrent access and more complex features.

---

## 🛠️ Technologies Used
- Bash
- C
- POSIX system calls: `fork()`, `exec()`, `kill()`, `waitpid()`, `signal()`
- Linux (Ubuntu)

---

## 👤 Author

João Pedro Magliano Goidanich  
BSc in Information Systems and Business Management @ ISCTE

