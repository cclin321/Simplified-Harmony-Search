Simplified Harmony Search: Novel algorithm design and its applications in engineering design optimization problems
1. Overview
The Simplified Harmony Search (SHS) algorithm is an enhanced metaheuristic optimization method based on the classical Harmony Search (HS) algorithm. By streamlining the process of generating new "notes" (decision variables) through the use of a single random value, SHS reduces computational overhead and improves search efficiency. It is suitable for solving various engineering design optimization problems and benchmark function optimization tasks, demonstrating superior convergence speed, stability, and global search capabilities.

2. Background and Motivation
The classical HS algorithm simulates musicians improvising to create harmonious melodies, where each iteration involves generating new notes by using two random values to decide among three operations. This two-stage process incurs a high computational cost, especially when dealing with complex or high-dimensional problems. The SHS algorithm addresses this issue by simplifying the note generation process—using only one random value to select the operation—which significantly accelerates the search while maintaining robustness in avoiding local optima.

3. Key Features
Efficiency: Reduces computational cost by streamlining the note generation process.
Stability: Consistently achieves high precision and demonstrates stable convergence behavior.
Flexibility: Easily integrated with additional strategies (e.g., local search, mutation operations) to further enhance global search ability.

4.Pseudocode

![图片1](https://github.com/user-attachments/assets/4749631b-10ec-46d5-b514-b892dbab72f5)


5. License
This project is licensed under the MIT License. See the LICENSE file for more details.

6. Contact
For questions or further discussions, please contact:Chun-Cheng Lin (cclin321@nycu.edu.tw)
