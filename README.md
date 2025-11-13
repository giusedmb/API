# 🍰 Pastry Shop Order Management System (C)

This project is a simulation of an industrial pastry shop's order management system, implemented in C. It handles recipe cataloging, inventory with expiry dates, order processing, wait-listing for unfulfillable orders, and courier dispatch based on capacity and weight. The simulation operates on discrete time steps.

-----

## 📋 Project Goal

The primary objective is to manage the flow of ingredients and customer orders according to specific rules defined by the prompt:

  * **FIFO Consumption**: Ingredients must be consumed from the batches with the **earliest expiry date** first[cite: 14].
  * **Wait List (FIFO)**: Orders that cannot be fulfilled immediately due to insufficient ingredients are placed on a wait list and processed in **chronological arrival order** when a new ingredient shipment arrives[cite: 15, 19].
  * **Courier Dispatch**: Ready orders are chosen for the courier in **chronological arrival order** up to the capacity limit[cite: 21, 22]. Loaded orders are then printed, sorted by **decreasing weight**, and by **chronological arrival time** for ties[cite: 24, 25]. The weight of a prepared item equals the sum of its ingredients' quantities[cite: 23].

-----

## 🛠️ Implementation Details & Data Structures

The system relies heavily on **Hash Tables** for quick lookups and **Doubly Linked Lists** for managing ordered data like ingredient batches (by expiry) and waiting lists (by arrival time).

| Component | Data Structure | Purpose |
| :--- | :--- | :--- |
| **Recipe Catalog (`ricettario_tipo`)** | Hash Table (Open Addressing with Lazy Deletion/Tombs) | Stores and retrieves recipes by name. Includes **rehashing** logic when the load factor exceeds 0.5. |
| **Warehouse/Inventory (`magazzino_tipo`)** | Hash Table (Open Addressing) | Stores and retrieves the overall quantity of each ingredient by name. |
| **Ingredient Batches (`nodo_lista_magazzino_tipo`)** | Linked List (Sorted by `scad`) | Each entry in the Warehouse Hash Table points to a list of ingredient batches, sorted by ascending expiry time (oldest first). |
| **Order Wait List (`nodo_wait_list_tipo`)** | Doubly Linked List (FIFO Queue) | Stores orders waiting for missing ingredients, sorted by arrival time. |
| **Wait Pickup List (`wait_pickup_list_tipo`)** | Doubly Linked List (FIFO Queue) | Stores orders ready for courier pickup, sorted by arrival time. |

### Key Logic: Ingredient Consumption

When an order is placed:

1.  The system checks the total ingredient quantity against the order requirement.
2.  Crucially, it **removes any expired ingredient batches** (`scad <= time`) *before* checking the total stock (`check_expired_batch`).
3.  If feasible, ingredients are consumed from the head of the ingredient batch list (the batch with the **earliest expiry date**).
4.  If not feasible, the insufficient ingredient is moved to the **head of the recipe's ingredient list** using `swap_ingredienti` to optimize future checks, and the order is moved to the **Wait List**.

-----

## ➡️ Commands

The program reads commands line-by-line from standard input:

| Command | Syntax (Example) | Description | Output |
| :--- | :--- | :--- | :--- |
| **`aggiungi_ricetta`** | `aggiungi_ricetta torta farina 50 uova 10` | Adds a new recipe to the catalog. Ignored if already present. | `aggiunta` or `ignorato` |
| **`rimuovi_ricetta`** | `rimuovi_ricetta cannoncini` | Removes a recipe. Fails if there are orders for it that haven't been shipped yet. | `rimossa`, `ordini in sospeso`, or `non presente` |
| **`rifornimento`** | `rifornimento zucchero 200 150 farina 1000 220` | Adds new ingredient batches: `(name) (quantity) (expiry_time)`. This triggers a check for all pending orders. | `rifornito` |
| **`ordine`** | `ordine torta_paradiso 36` | Places an order for a quantity of a recipe. | `accettato` or `rifiutato` (if recipe doesn't exist) |

**Courier Output**:
The courier is checked at time $k \times n$ (where $n$ is the periodicity) before processing the command at that time. Orders loaded onto the truck are printed as:

```
(arrival_time) (recipe_name) (quantity)
```

If the truck is empty, it prints: `camioncino vuoto`[cite: 48, 49].

-----

## 🚀 Build and Run

This project is a single C source file (`main.c`).

1.  **Compile** using GCC:

    ```bash
    gcc -o pastry_shop main.c
    ```

2.  **Run** the compiled executable, passing the input data via standard input:

    ```bash
    ./pastry_shop < input_data.txt
    ```
