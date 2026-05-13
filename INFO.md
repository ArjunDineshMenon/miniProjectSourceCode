# 🏦 Bank Transaction Processing System

> A C-based random-access file bank management system — enhanced from the Deitel & Deitel base program as part of the **24UCS271 Mini Project**.

---

## Table of Contents

- [Overview](#overview)
- [How It Works Internally](#how-it-works-internally)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Compile](#compile)
  - [Run](#run)
- [Features](#features)
  - [1. Export Accounts to Text File](#1-export-accounts-to-text-file)
  - [2. Update an Account](#2-update-an-account)
  - [3. Add a New Account](#3-add-a-new-account)
  - [4. Delete an Account](#4-delete-an-account)
  - [5. Rich List — Sort by Balance](#5-rich-list--sort-by-balance)
  - [6. Transfer Funds Between Accounts](#6-transfer-funds-between-accounts)
  - [7. Search by Name](#7-search-by-name)
  - [Auto File Initialisation](#auto-file-initialisation-error-handling)
- [Data File Structure](#data-file-structure)
- [Bug Fixes Over the Original](#bug-fixes-over-the-original)
- [Code Architecture](#code-architecture)
- [Suggested Future Features](#suggested-future-features)
- [Project Info](#project-info)

---

## Overview

This program manages up to **100 bank accounts** stored in a binary random-access file (`credit.dat`). Each account holds an account number, first name, last name, and balance. All reads and writes go directly to the correct position in the file using `fseek`, making every operation O(1) — no scanning required for account lookups.

The program was built on the original Deitel & Deitel textbook program and extended with new features, bug fixes, input validation, and refactored helpers for the lab mini project.

---

## How It Works Internally

`credit.dat` is a **fixed-size binary file** of exactly 100 slots. Each slot is `sizeof(ClientData)` bytes wide. Account #N always lives at byte offset `(N-1) × sizeof(ClientData)` from the start of the file.

```
Slot 0  → Account #1   [acctNum | lastName | firstName | balance]
Slot 1  → Account #2   [acctNum | lastName | firstName | balance]
...
Slot 99 → Account #100 [acctNum | lastName | firstName | balance]
```

An empty slot has `acctNum == 0`. This means:
- **Lookup** → one `fseek` + one `fread` → O(1)
- **Create/Update/Delete** → one `fseek` + one `fwrite` → O(1)
- **List / Search / Sort** → sequential scan of all 100 slots → O(n)

---

## Getting Started

### Prerequisites

- GCC (or any C99-compatible compiler)
- A Unix/Linux/macOS terminal, or Windows with MinGW / WSL

### Compile

```bash
gcc -Wall -Wextra -o bank trans_enhanced.c
```

### Run

```bash
./bank
```

On the **very first run**, `credit.dat` does not exist yet. The program detects this and automatically creates it with 100 empty slots — no setup step needed.

```
'credit.dat' not found. Creating new file with 100 empty slots...
Done. File ready.
```

---

## Features

When you run the program you will see this menu:

```
╔══════════════════════════════════════╗
║    BANK TRANSACTION SYSTEM v3.0      ║
╠══════════════════════════════════════╣
║  1 - Export accounts to .txt          ║
║  2 - Update account (charge/payment)  ║
║  3 - Add new account                  ║
║  4 - Delete account                   ║
║  5 - Rich List (sort by balance)      ║
║  6 - Transfer funds between accounts  ║
║  7 - Search by name                   ║
║  8 - Exit                             ║
╚══════════════════════════════════════╝
```

---

### 1. Export Accounts to Text File

**Menu option:** `1`

Scans all 100 slots and writes every active account (where `acctNum != 0`) into a human-readable file called `accounts.txt` in the same directory. The file is overwritten each time you use this option.

**Example session:**

```
Choice: 1
'accounts.txt' written with 4 account(s).
```

**Example `accounts.txt` output:**

```
Acct   Last Name       First Name      Balance
------  ---------------  ----------  ------------
3      Brown           Alice          12000.00
10     Smith           John            5000.00
25     Zara            Bob              800.00
50     Adams           Carol           3500.00

Total active accounts: 4
```

> **Tip:** Use this before presenting your project — it gives you a clean printable snapshot of the database at any moment.

---

### 2. Update an Account

**Menu option:** `2`

Apply a charge (positive number) or a payment (negative number) to an existing account's balance.

**Example session:**

```
Choice: 2
Enter account to update (1-100): 10
Acct   Last Name       First Name      Balance
------  ---------------  ----------  ------------
10     Smith           John            5000.00

Enter charge (+) or payment (-): -250
Updated:
10     Smith           John            4750.00
```

**Validation:**
- Account number must be between 1 and 100.
- The account must already exist (not an empty slot).
- Non-numeric input is rejected cleanly.

---

### 3. Add a New Account

**Menu option:** `3`

Create a brand-new account in an empty slot. You choose the account number (1–100), then provide the last name, first name, and opening balance.

**Example session:**

```
Choice: 3
Enter new account number (1-100): 42
Enter last name  (max 14 chars): Kumar
Enter first name (max 9 chars): Ravi
Enter opening balance: 2500
Account #42 created.
Acct   Last Name       First Name      Balance
------  ---------------  ----------  ------------
42     Kumar           Ravi            2500.00
```

**Validation:**
- Will not overwrite an account that already exists in that slot.
- Names are capped at 14 / 9 characters to prevent buffer overflow.
- A warning is shown if the opening balance is negative (allowed but flagged).

---

### 4. Delete an Account

**Menu option:** `4`

Permanently removes an account by overwriting its slot with a blank record (`acctNum = 0`). A confirmation prompt prevents accidental deletion.

**Example session:**

```
Choice: 4
Enter account number to delete (1-100): 25
Acct   Last Name       First Name      Balance
------  ---------------  ----------  ------------
25     Zara            Bob              800.00
Confirm delete account #25? (y/n): y
Account #25 deleted.
```

Enter `n` at the confirmation to cancel safely.

**Validation:**
- Reports an error if the account does not exist.
- The slot number is freed and can be reused by option 3.

---

### 5. Rich List — Sort by Balance

**Menu option:** `5`

Displays all active accounts ranked from the highest balance to the lowest. The file itself is **not modified** — the sort happens entirely in memory.

**Example output:**

```
╔══════ RICH LIST (Highest Balance First) ══════╗
Rank  Acct   Last Name       First Name      Balance
-----  ------  ---------------  ----------  ------------
1     3      Brown           Alice          13000.00
2     10     Smith           John            4000.00
3     50     Adams           Carol           3500.00
4     42     Kumar           Ravi            2500.00

Total accounts ranked: 4
```

**How it works:** All active records are loaded into an in-memory array, sorted with `qsort()` using a custom descending comparator (`cmpBalanceDesc`), then printed. Because the file uses account-number-indexed slots, the on-disk order cannot be changed without breaking all `fseek` offsets — so display-only sorting is the correct approach.

---

### 6. Transfer Funds Between Accounts

**Menu option:** `6`

Move a specified amount from one account to another in a single operation. Both records are updated and written back to the file.

**Example session:**

```
Choice: 6
Enter sender account number (1-100): 3
Enter receiver account number (1-100): 42

Sender  : Acct   Last Name   First Name   Balance
          3      Brown       Alice        13000.00
Receiver: Acct   Last Name   First Name   Balance
          42     Kumar       Ravi          2500.00

Enter amount to transfer: 500

Transfer of 500.00 from Account #3 to Account #42 successful.

Updated sender  : 3    Brown    Alice    12500.00
Updated receiver: 42   Kumar    Ravi      3000.00
```

**Validation checks (all must pass before any write occurs):**

| Check | What happens if it fails |
|---|---|
| Sender account in range 1–100 | Aborted, message shown |
| Sender account exists | Aborted, message shown |
| Receiver account in range 1–100 | Aborted, message shown |
| Receiver account exists | Aborted, message shown |
| Sender ≠ Receiver | Aborted — self-transfer not allowed |
| Amount is positive | Aborted, message shown |
| Sender balance ≥ amount | Aborted — insufficient funds |

If any check fails, the file is left completely unchanged.

---

### 7. Search by Name

**Menu option:** `7`

Scan all accounts and find those whose first or last name contains your search query. The search is **case-insensitive** and supports **partial matches** — you do not need to type the full name.

**Example session:**

```
Choice: 7

Search in:
  1 - Last name
  2 - First name
Choice: 1
Enter name to search (partial match OK): bro

--- Search results for "bro" ---
Acct   Last Name       First Name      Balance
------  ---------------  ----------  ------------
3      Brown           Alice          12500.00

1 match(es) found.
```

Typing `"bro"` finds `"Brown"`. Typing `"a"` would find every account whose last name contains the letter A. A "0 matches" message is shown if nothing is found.

**How it works:** Because the file is indexed by account number (not name), every slot must be read and checked — a linear O(n) scan. Both the stored name and your query are lowercased before comparison using `tolower()`, and `strstr()` is used for substring matching.

---

### Auto File Initialisation (Error Handling)

You never need to run a separate setup program. On first launch, if `credit.dat` is missing the program:

1. Creates `credit.dat` fresh.
2. Writes 100 blank `ClientData` records so every slot offset is correctly initialised.
3. Continues to the menu normally.

The original Deitel program called `exit(-1)` and crashed in this situation. This version is fully self-contained.

---

## Data File Structure

| Field | Type | Size | Notes |
|---|---|---|---|
| `acctNum` | `unsigned int` | 4 bytes | 0 = empty slot |
| `lastName` | `char[15]` | 15 bytes | Null-terminated |
| `firstName` | `char[10]` | 10 bytes | Null-terminated |
| `balance` | `double` | 8 bytes | Signed, supports overdraft |

**Record size:** `sizeof(ClientData)` (typically 40 bytes with padding).  
**File size:** always exactly `100 × sizeof(ClientData)`.  
**Account #N offset:** `(N-1) × sizeof(ClientData)` bytes from start.

---

## Bug Fixes Over the Original

| Location | Original bug | Fix applied |
|---|---|---|
| `textFile()` | `while (!feof(readPtr))` called **before** `fread()` — last record was processed twice | Loop now driven solely by `fread() == 1` |
| `deleteRecord()` | `fseek()` called twice redundantly | Centralised into `writeRecord()` helper |
| `main()` | `exit(-1)` when `credit.dat` missing | Falls back to `fopen("wb+")` and initialises file |
| All input | No return-value checking on `scanf()` | Every `scanf()` call checks return value; `flushInput()` clears `stdin` |
| Name inputs | Unbounded `%s` — buffer overflow risk | Bounded to `%14s` / `%9s` |
| `fseek()` offsets | `int` arithmetic could overflow on large files | Cast to `(long)` before multiplication |

---

## Code Architecture

```
main()
│
├── enterChoice()          — menu display and input
│
├── textFile()             — export to accounts.txt
├── updateRecord()         — apply charge or payment
├── newRecord()            — create account in empty slot
├── deleteRecord()         — overwrite slot with blank record
│
├── richList()             — load → qsort descending → display
│   └── cmpBalanceDesc()   — qsort comparator
│
├── transferFunds()        — validate both accounts → deduct → credit → write
│
├── searchByName()         — linear scan with tolower + strstr
│
└── Helpers
    ├── readRecord()       — fseek + fread for one slot
    ├── writeRecord()      — fseek + fwrite for one slot
    ├── printHeader()      — column header line
    ├── printRecord()      — one formatted account row
    ├── validAccount()     — range check 1–100
    └── flushInput()       — drain leftover stdin characters
```

---

## Suggested Future Features

Here are well-scoped ideas that would each make strong additions to this project:

### Intermediate

**Transaction History Log**
Every charge, payment, and transfer is appended to a `transactions.log` text file with a timestamp (using `time.h`). Lets you audit what happened to any account over time.

**Account Balance Limit / Credit Limit**
Add a `creditLimit` field to `ClientData`. The update and transfer functions check that the resulting balance does not go below `-creditLimit`. Demonstrates struct extension and constraint validation.

**List Accounts with Filters**
Extend option 5 to let the user filter before sorting — e.g. "show only accounts with balance above X" or "show only overdrawn accounts". Uses the same in-memory array + loop approach already in place.

**Pagination for Large Listings**
When listing all accounts, display 10 at a time and press Enter to see the next page. Useful when many accounts are active.

### Advanced

**Backup and Restore**
Option to copy `credit.dat` to `credit_backup.dat` (backup), and restore from it (overwrite `credit.dat` with the backup). Uses `fread`/`fwrite` in a loop — pure C, no system calls needed.

**Deposit / Withdrawal with Daily Limit**
Add a `dailyWithdrawn` field and a configurable `DAILY_LIMIT` constant. The update function tracks how much has been withdrawn today and refuses transactions that would exceed the limit. Resets to zero when the program restarts (or when a date check via `time.h` detects a new day).

**Mini Statement (Last N Transactions)**
Store a circular buffer of the last 5 transactions per account inside the struct (or in a parallel `history.dat` file). Option 2 shows the statement after updating. Teaches circular buffers and struct-within-struct design.

**Interest Calculation**
Add a menu option that applies a configurable annual interest rate (e.g. 3.5%) to all positive-balance accounts and writes the updated balances back. Shows batch file update patterns.

**Account Number Auto-Assignment**
Instead of asking the user to pick a slot number, scan for the first empty slot and assign it automatically. The user just provides name and balance. Makes the UX much friendlier.

---

## Project Info

| Item | Detail |
|---|---|
| Course | 24UCS271 |
| Base reference | Deitel & Deitel — *C How to Program* |
| Language | C (C99) |
| Compiler tested | GCC 13 with `-Wall -Wextra` (zero warnings) |
| Data file | `credit.dat` (binary, auto-created) |
| Max accounts | 100 (change `MAX_ACCOUNTS` to extend) |