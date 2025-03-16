Simplified Harmony Search

1. Overview:
The Simplified Harmony Search (SHS) algorithm is an enhanced metaheuristic algorithm based on the classical Harmony Search (HS) algorithm. Different from the HS algorithm that iteratively improves the solution population through checking two random values in two stages to generate a new "note" (consisting of the decision variables of the concerned optimization problem) among three operations (i.e., selecting an arbitrary previous note, selecting an arbitrary previous note and further perturbing it, and randomly generating a note), the SHS algorithm checks only one random value in one stage to generate a new note from the three operations iteratively, drastically reducing computational overhead and improving search efficiency. It is suitable for solving various engineering design optimization problems and benchmark function optimization tasks, demonstrating superior convergence speed, stability, and global search capabilities.

2. Key Features:
Efficiency: Reduce computational cost by streamlining the note generation process.
Stability: Consistently achieve high precision and demonstrate stable convergence behavior.
Flexibility: Easily integrated with additional strategies (e.g., local search, and mutation operations) to further enhance global search ability.

3. Pseudocode

![图片1](https://github.com/user-attachments/assets/4749631b-10ec-46d5-b514-b892dbab72f5)


4. License
This project is licensed under the MIT License. See the LICENSE file for more details.

5. Contact
For questions or further discussions, please contact: Chun-Cheng Lin (cclin321@nycu.edu.tw)
