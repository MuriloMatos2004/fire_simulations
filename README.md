# Parallel Simulation of Wildfire Propagation (OpenMP)

C/OpenMP parallel implementation of a 2D wildfire spread simulation. Evaluates performance, scheduling policies, and memory bottlenecks on multi-core CPUs.

---

## 📌 Project Overview

This application models cellular-automata-based 2D forest fire dynamics across dynamic grid terrains. The implementation compares a single-threaded baseline against an OpenMP-parallelized solver to analyze performance scalability, speedup barriers, and thread scheduling behavior (`dynamic,512` vs `guided,512`).

Key features:
* **Sequential Baseline (`fire_seq.c`):** Core reference implementation.
* **Parallel OpenMP Implementation (`fire_omp.c`):** Parallel grid processing with thread scheduling and race-condition safety.
* **Automated Toolchain:** Scripts for synthetic grid generation, execution benchmarks, and output verification.
