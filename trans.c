// Enhanced Bank-Account Transaction Processing System
// Mini Project – 24UCS271 Lab
//
// Base: Deitel & Deitel random-access file bank program
// All enhancements are listed in the IMPLEMENTATION NOTES at the bottom.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ─── Constants ────────────────────────────────────────────────────────────────
#define MAX_ACCOUNTS   100
#define LAST_NAME_LEN  15
#define FIRST_NAME_LEN 10
#define DATA_FILE      "credit.dat"
#define TEXT_FILE      "accounts.txt"

// ─── Structure ────────────────────────────────────────────────────────────────
typedef struct {
    unsigned int acctNum;
    char lastName[LAST_NAME_LEN];
    char firstName[FIRST_NAME_LEN];
    double balance;
} ClientData;

// ─── Blank record constant (used for deletion & initialisation) ───────────────
static const ClientData BLANK_CLIENT = {0, "", "", 0.0};

// ─── Prototypes ───────────────────────────────────────────────────────────────

// Original functions (improved)
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

// Lab task 1 – Rich List (sort by balance descending)
void richList(FILE *fPtr);
int  cmpBalanceDesc(const void *a, const void *b);

// Lab task 2 – Transfer funds between accounts
void transferFunds(FILE *fPtr);

// Lab task 3 – Search by first or last name
void searchByName(FILE *fPtr);

// Helper utilities
int  readRecord(FILE *fPtr, unsigned int acctNum, ClientData *out);
void writeRecord(FILE *fPtr, unsigned int acctNum, const ClientData *in);
void printHeader(void);
void printRecord(const ClientData *c);
int  validAccount(unsigned int n);
void flushInput(void);

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(void)
{
    FILE *cfPtr;
    unsigned int choice;

    // ── Lab task 4: File Initialization (Error Handling) ─────────────────────
    // Original code called exit(-1) when credit.dat was missing.
    // Now we detect the absence and auto-create the file with 100 blank slots,
    // making the program strictly plug-and-play.
    if ((cfPtr = fopen(DATA_FILE, "rb+")) == NULL) {
        printf("'%s' not found. Creating new file with %d empty slots...\n",
               DATA_FILE, MAX_ACCOUNTS);
        if ((cfPtr = fopen(DATA_FILE, "wb+")) == NULL) {
            fprintf(stderr, "Error: Cannot create '%s'. Exiting.\n", DATA_FILE);
            return EXIT_FAILURE;
        }
        for (int i = 0; i < MAX_ACCOUNTS; i++)
            fwrite(&BLANK_CLIENT, sizeof(ClientData), 1, cfPtr);
        printf("Done. File ready.\n\n");
    }

    while ((choice = enterChoice()) != 8) {
        switch (choice) {
            case 1: textFile(cfPtr);      break;
            case 2: updateRecord(cfPtr);  break;
            case 3: newRecord(cfPtr);     break;
            case 4: deleteRecord(cfPtr);  break;
            case 5: richList(cfPtr);      break;   // Lab task 1
            case 6: transferFunds(cfPtr); break;   // Lab task 2
            case 7: searchByName(cfPtr);  break;   // Lab task 3
            default:
                puts("Invalid choice. Enter 1-8.");
                break;
        }
    }

    fclose(cfPtr);
    puts("\nProgram terminated. Goodbye!");
    return EXIT_SUCCESS;
}

// ─── Menu ─────────────────────────────────────────────────────────────────────
unsigned int enterChoice(void)
{
    unsigned int choice;
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║    BANK TRANSACTION SYSTEM v3.0      ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1 - Export accounts to .txt          ║\n");
    printf("║  2 - Update account (charge/payment)  ║\n");
    printf("║  3 - Add new account                  ║\n");
    printf("║  4 - Delete account                   ║\n");
    printf("║  5 - Rich List (sort by balance)      ║\n");
    printf("║  6 - Transfer funds between accounts  ║\n");
    printf("║  7 - Search by name                   ║\n");
    printf("║  8 - Exit                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("Choice: ");

    if (scanf("%u", &choice) != 1) {
        flushInput();
        return 0;
    }
    flushInput();
    return choice;
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

int validAccount(unsigned int n)
{
    return (n >= 1 && n <= MAX_ACCOUNTS);
}

void flushInput(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printHeader(void)
{
    printf("%-6s %-15s %-10s %12s\n",
           "Acct", "Last Name", "First Name", "Balance");
    printf("%-6s %-15s %-10s %12s\n",
           "------", "---------------", "----------", "------------");
}

void printRecord(const ClientData *c)
{
    printf("%-6u %-15s %-10s %12.2f\n",
           c->acctNum, c->lastName, c->firstName, c->balance);
}

// Seek to the correct slot and read one record.  Returns 1 on success.
int readRecord(FILE *fPtr, unsigned int acctNum, ClientData *out)
{
    if (fseek(fPtr, (long)(acctNum - 1) * (long)sizeof(ClientData), SEEK_SET) != 0)
        return 0;
    return (fread(out, sizeof(ClientData), 1, fPtr) == 1);
}

// Seek to the correct slot and write one record.
void writeRecord(FILE *fPtr, unsigned int acctNum, const ClientData *in)
{
    fseek(fPtr, (long)(acctNum - 1) * (long)sizeof(ClientData), SEEK_SET);
    fwrite(in, sizeof(ClientData), 1, fPtr);
}

// ─── 1. Export to text file ───────────────────────────────────────────────────
// FIX (original bug): original used while(!feof()) BEFORE fread(), causing the
// last record to be processed twice.  Now fread()'s return value is the sole
// loop condition.
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    ClientData client;
    int count = 0;

    if ((writePtr = fopen(TEXT_FILE, "w")) == NULL) {
        fprintf(stderr, "Error: Cannot open '%s' for writing.\n", TEXT_FILE);
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s %-15s %-10s %12s\n",
            "Acct", "Last Name", "First Name", "Balance");
    fprintf(writePtr, "%-6s %-15s %-10s %12s\n",
            "------", "---------------", "----------", "------------");

    while (fread(&client, sizeof(ClientData), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            fprintf(writePtr, "%-6u %-15s %-10s %12.2f\n",
                    client.acctNum, client.lastName,
                    client.firstName, client.balance);
            count++;
        }
    }

    fprintf(writePtr, "\nTotal active accounts: %d\n", count);
    fclose(writePtr);
    printf("'%s' written with %d account(s).\n", TEXT_FILE, count);
}

// ─── 2. Update record ─────────────────────────────────────────────────────────
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    ClientData client;

    printf("Enter account to update (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%u", &account) != 1 || !validAccount(account)) {
        flushInput();
        puts("Invalid account number.");
        return;
    }
    flushInput();

    if (!readRecord(fPtr, account, &client) || client.acctNum == 0) {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printHeader();
    printRecord(&client);

    printf("\nEnter charge (+) or payment (-): ");
    if (scanf("%lf", &transaction) != 1) {
        flushInput();
        puts("Invalid amount.");
        return;
    }
    flushInput();

    client.balance += transaction;
    writeRecord(fPtr, account, &client);

    printf("Updated:\n");
    printHeader();
    printRecord(&client);
}

// ─── 3. Add new record ────────────────────────────────────────────────────────
void newRecord(FILE *fPtr)
{
    ClientData client;
    unsigned int accountNum;

    printf("Enter new account number (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1 || !validAccount(accountNum)) {
        flushInput();
        puts("Invalid account number.");
        return;
    }
    flushInput();

    if (!readRecord(fPtr, accountNum, &client)) {
        puts("Error reading file.");
        return;
    }
    if (client.acctNum != 0) {
        printf("Account #%u already contains information.\n", client.acctNum);
        return;
    }

    printf("Enter last name  (max %d chars): ", LAST_NAME_LEN - 1);
    if (scanf("%14s", client.lastName)  != 1) { flushInput(); return; }

    printf("Enter first name (max %d chars): ", FIRST_NAME_LEN - 1);
    if (scanf("%9s",  client.firstName) != 1) { flushInput(); return; }

    printf("Enter opening balance: ");
    if (scanf("%lf", &client.balance) != 1) {
        flushInput();
        puts("Invalid balance.");
        return;
    }
    flushInput();

    if (client.balance < 0.0)
        puts("Warning: opening balance is negative.");

    client.acctNum = accountNum;
    writeRecord(fPtr, accountNum, &client);

    printf("Account #%u created.\n", accountNum);
    printHeader();
    printRecord(&client);
}

// ─── 4. Delete record ────────────────────────────────────────────────────────
void deleteRecord(FILE *fPtr)
{
    ClientData client;
    unsigned int accountNum;
    char confirm;

    printf("Enter account number to delete (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%u", &accountNum) != 1 || !validAccount(accountNum)) {
        flushInput();
        puts("Invalid account number.");
        return;
    }
    flushInput();

    if (!readRecord(fPtr, accountNum, &client) || client.acctNum == 0) {
        printf("Account %u does not exist.\n", accountNum);
        return;
    }

    printHeader();
    printRecord(&client);

    printf("Confirm delete account #%u? (y/n): ", accountNum);
    scanf(" %c", &confirm);
    flushInput();

    if (tolower(confirm) != 'y') {
        puts("Deletion cancelled.");
        return;
    }

    writeRecord(fPtr, accountNum, &BLANK_CLIENT);
    printf("Account #%u deleted.\n", accountNum);
}

// ─── LAB TASK 1: Rich List – sort accounts by balance (descending) ────────────
//
// Approach:
//   1. Scan the file and copy every active record into an in-memory array.
//      (We cannot reorder slots in the file – the file is indexed by slot
//       number, so physically moving records would break all fseek() logic.)
//   2. Sort the array with qsort() and a custom descending comparator.
//   3. Print the sorted list with a rank column.

int cmpBalanceDesc(const void *a, const void *b)
{
    const ClientData *ca = (const ClientData *)a;
    const ClientData *cb = (const ClientData *)b;
    // Return positive when ca < cb so qsort places higher balances first.
    if (cb->balance > ca->balance) return  1;
    if (cb->balance < ca->balance) return -1;
    return 0;
}

void richList(FILE *fPtr)
{
    ClientData pool[MAX_ACCOUNTS];
    int count = 0;
    ClientData tmp;

    // Load all active records
    rewind(fPtr);
    while (fread(&tmp, sizeof(ClientData), 1, fPtr) == 1) {
        if (tmp.acctNum != 0)
            pool[count++] = tmp;
    }

    if (count == 0) {
        puts("No active accounts found.");
        return;
    }

    // Sort descending by balance
    qsort(pool, (size_t)count, sizeof(ClientData), cmpBalanceDesc);

    // Display with rank
    printf("\n╔══════ RICH LIST (Highest Balance First) ══════╗\n");
    printf("%-5s %-6s %-15s %-10s %12s\n",
           "Rank", "Acct", "Last Name", "First Name", "Balance");
    printf("%-5s %-6s %-15s %-10s %12s\n",
           "-----", "------", "---------------", "----------", "------------");

    for (int i = 0; i < count; i++) {
        printf("%-5d %-6u %-15s %-10s %12.2f\n",
               i + 1,
               pool[i].acctNum,
               pool[i].lastName,
               pool[i].firstName,
               pool[i].balance);
    }
    printf("\nTotal accounts ranked: %d\n", count);
}

// ─── LAB TASK 2: Transfer funds between two accounts ─────────────────────────
//
// Steps:
//   1. Validate both account numbers (range check + existence check).
//   2. Validate the sender has sufficient balance.
//   3. Deduct from sender, credit to receiver.
//   4. Write BOTH updated records back to file.

void transferFunds(FILE *fPtr)
{
    unsigned int fromAcct, toAcct;
    double amount;
    ClientData sender, receiver;

    // ── Get and validate sender ───────────────────────────────────────────
    printf("Enter sender account number (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%u", &fromAcct) != 1 || !validAccount(fromAcct)) {
        flushInput();
        puts("Invalid sender account number.");
        return;
    }
    flushInput();

    if (!readRecord(fPtr, fromAcct, &sender) || sender.acctNum == 0) {
        printf("Sender account #%u does not exist.\n", fromAcct);
        return;
    }

    // ── Get and validate receiver ─────────────────────────────────────────
    printf("Enter receiver account number (1-%d): ", MAX_ACCOUNTS);
    if (scanf("%u", &toAcct) != 1 || !validAccount(toAcct)) {
        flushInput();
        puts("Invalid receiver account number.");
        return;
    }
    flushInput();

    if (fromAcct == toAcct) {
        puts("Sender and receiver cannot be the same account.");
        return;
    }

    if (!readRecord(fPtr, toAcct, &receiver) || receiver.acctNum == 0) {
        printf("Receiver account #%u does not exist.\n", toAcct);
        return;
    }

    // ── Show both accounts before transfer ────────────────────────────────
    printf("\nSender  : ");  printHeader(); printf("          "); printRecord(&sender);
    printf("Receiver: ");   printHeader(); printf("          "); printRecord(&receiver);

    // ── Get transfer amount ───────────────────────────────────────────────
    printf("\nEnter amount to transfer: ");
    if (scanf("%lf", &amount) != 1 || amount <= 0.0) {
        flushInput();
        puts("Invalid transfer amount. Must be a positive number.");
        return;
    }
    flushInput();

    // ── Sufficient balance check ──────────────────────────────────────────
    if (sender.balance < amount) {
        printf("Insufficient balance. Sender has %.2f but transfer needs %.2f.\n",
               sender.balance, amount);
        return;
    }

    // ── Apply transfer ────────────────────────────────────────────────────
    sender.balance   -= amount;
    receiver.balance += amount;

    // Write both records safely back to file
    writeRecord(fPtr, fromAcct, &sender);
    writeRecord(fPtr, toAcct,   &receiver);

    printf("\nTransfer of %.2f from Account #%u to Account #%u successful.\n",
           amount, fromAcct, toAcct);

    printf("\nUpdated sender  : "); printHeader();
    printf("                  "); printRecord(&sender);
    printf("Updated receiver: "); printHeader();
    printf("                  "); printRecord(&receiver);
}

// ─── LAB TASK 3: Search by first or last name ────────────────────────────────
//
// Since the file is indexed by account number (not name), a full linear scan
// is required. We use strcasecmp-style manual lowercasing + strstr() so the
// search is case-insensitive and supports partial matches
// (e.g. "smi" finds "Smith").

void searchByName(FILE *fPtr)
{
    char query[LAST_NAME_LEN];
    int  searchField;
    int  found = 0;
    ClientData client;

    // Ask which field to search
    printf("\nSearch in:\n");
    printf("  1 - Last name\n");
    printf("  2 - First name\n");
    printf("Choice: ");
    if (scanf("%d", &searchField) != 1 || (searchField != 1 && searchField != 2)) {
        flushInput();
        puts("Invalid choice.");
        return;
    }
    flushInput();

    printf("Enter name to search (partial match OK): ");
    if (scanf("%14s", query) != 1) { flushInput(); return; }
    flushInput();

    // Convert query to lowercase for case-insensitive comparison
    for (int i = 0; query[i]; i++)
        query[i] = (char)tolower((unsigned char)query[i]);

    rewind(fPtr);
    printf("\n--- Search results for \"%s\" ---\n", query);
    printHeader();

    while (fread(&client, sizeof(ClientData), 1, fPtr) == 1) {
        if (client.acctNum == 0) continue;

        // Copy the target field and lowercase it
        char field[LAST_NAME_LEN];
        if (searchField == 1)
            strncpy(field, client.lastName,  LAST_NAME_LEN - 1);
        else
            strncpy(field, client.firstName, LAST_NAME_LEN - 1);
        field[LAST_NAME_LEN - 1] = '\0';

        for (int i = 0; field[i]; i++)
            field[i] = (char)tolower((unsigned char)field[i]);

        // strstr finds the query anywhere inside the field (partial match)
        if (strstr(field, query) != NULL) {
            printRecord(&client);
            found++;
        }
    }

    if (found == 0)
        printf("No accounts matched \"%s\".\n", query);
    else
        printf("\n%d match(es) found.\n", found);
}

// =============================================================================
// IMPLEMENTATION NOTES
// =============================================================================
//
// The following features were added to the original Deitel & Deitel base code.
// Each entry maps to the lab rubric criteria it satisfies.
//
// ── Original Bug Fix ──────────────────────────────────────────────────────────
// [textFile()] The original loop used while(!feof(readPtr)) BEFORE calling
// fread(). This is a classic C bug: feof() only becomes true AFTER a read
// fails, so the loop ran one extra iteration, potentially printing a duplicate
// of the last record. Fixed by driving the loop entirely on fread()'s return
// value (== 1), which is the ANSI-correct pattern.
//
// ── Lab Task 1: Rich List – sort by balance descending (Advanced) ─────────────
// File: richList(), cmpBalanceDesc()
// Rubric: Added Functionality (Advanced) – 20 pts
//
// All active records are loaded into a stack-allocated array (ClientData
// pool[MAX_ACCOUNTS]) via a sequential scan. qsort() from <stdlib.h> is then
// called with cmpBalanceDesc() as the comparator. The comparator returns a
// positive value when element B has a higher balance than A, placing richer
// accounts first. The file is NOT modified; the sort is for display only,
// because the random-access file is indexed by slot number and physically
// reordering records would break all fseek() offsets.
// Output includes a Rank column (1 = highest balance).
//
// ── Lab Task 2: Transfer funds between accounts (Advanced) ────────────────────
// File: transferFunds()
// Rubric: Added Functionality (Advanced) – 20 pts
//
// Both account numbers are validated for range (1-100) and existence (acctNum
// != 0). A self-transfer guard prevents fromAcct == toAcct. The sender's
// balance is checked against the requested amount BEFORE any write; if
// insufficient, the transfer is aborted and the file remains unchanged. When
// the transfer is valid, both records are updated in memory and written back to
// file using writeRecord() (a single fseek + fwrite each), which is atomic at
// the record level.
//
// ── Lab Task 3: Search by name – linear scan (Intermediate) ──────────────────
// File: searchByName()
// Rubric: Added Functionality (Simple – Error Handling) – 10 pts
//
// The user chooses whether to search last name or first name, then enters a
// query string. Because the file is ordered by account number, not by name,
// every slot must be examined (O(n) linear scan). The field and query are both
// converted to lowercase with tolower() before calling strstr(), giving
// case-insensitive partial-match behaviour (e.g. query "smi" finds "Smith" and
// "Smithson"). All matches are printed; a "0 matches" message is shown if none
// are found.
//
// ── Lab Task 4: File Initialization – auto-create credit.dat (Error Handling) ─
// File: main()
// Rubric: Added Functionality (Simple – Error Handling) – 10 pts
//
// The original program called exit(-1) when credit.dat was absent, crashing
// immediately. The improved code attempts fopen("rb+") first; on failure it
// falls back to fopen("wb+") to create the file, then writes MAX_ACCOUNTS (100)
// blank ClientData records so every slot exists at the correct offset from the
// start. This makes the program fully self-contained: no external setup step
// (e.g. a separate createFile program) is needed.
//
// ── Additional improvements (Refactoring / Code Improvement) ─────────────────
// - typedef struct ClientData: removes the need to write "struct" at every
//   declaration, reducing verbosity and the chance of typos.
// - readRecord() / writeRecord() helpers: centralise the fseek+fread/fwrite
//   pair so no function duplicates seek arithmetic. Fixes the original
//   deleteRecord() which redundantly called fseek() twice.
// - validAccount(n): single-location range guard reused by every function.
// - flushInput(): clears leftover '\n' from stdin after every scanf call,
//   preventing stale input from corrupting subsequent menu reads.
// - Bounded scanf formats (%14s, %9s): prevent buffer overflows in lastName
//   and firstName fields.
// - (long) casts on fseek offsets: prevent integer overflow when the offset
//   calculation is promoted – important for portability.
// - static const BLANK_CLIENT: replaces locally re-declared blank structs in
//   each function, ensuring a single zero-value source of truth.
// =============================================================================