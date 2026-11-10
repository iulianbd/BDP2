#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "customers.h"
#include "odbc.h"

void customer_find(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[64], pattern[70];
    int cnum = 0;
    char cname[51], fname[51], lname[51];

    printf("Enter customer name > ");
    (void) fflush(stdout);
    if (!fgets(input, sizeof(input), stdin)) return;
    input[strcspn(input, "\r\n")] = 0;
    snprintf(pattern, sizeof(pattern), "%%%s%%", input);

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT customernumber, customername, contactfirstname, contactlastname FROM customers WHERE contactfirstname LIKE ? OR contactlastname LIKE ? ORDER BY customernumber", SQL_NTS))) {
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, pattern, 0, NULL);
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, pattern, 0, NULL);

        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            SQLBindCol(stmt, 1, SQL_C_SLONG, &cnum, 0, NULL);
            SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*)cname, sizeof(cname), NULL);
            SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*)fname, sizeof(fname), NULL);
            SQLBindCol(stmt, 4, SQL_C_CHAR, (SQLCHAR*)lname, sizeof(lname), NULL);

            typedef struct { int n; char cn[51]; char fn[51]; char ln[51]; } Res;
            Res* results = NULL;
            int count = 0, capacity = 10;
            results = malloc(capacity * sizeof(Res));

            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                if (count >= capacity) {
                    capacity *= 2;
                    results = realloc(results, capacity * sizeof(Res));
                }
                results[count].n = cnum;
                strcpy(results[count].cn, cname);
                strcpy(results[count].fn, fname);
                strcpy(results[count].ln, lname);
                count++;
            }

            if (count == 0) {
                printf("No se encontraron clientes.\n");
            } else {
                int page = 0;
                int total_pages = (count + 9) / 10;
                char cmd[16];
                
                while(1) {
                    int start = page * 10;
                    int end = (start + 10 > count) ? count : start + 10;
                    for (int i = start; i < end; i++) {
                        printf("%d %s %s %s\n", results[i].n, results[i].cn, results[i].fn, results[i].ln);
                    }
                    if (total_pages <= 1) break;
                    
                    printf("\nPage %d of %d. Enter '<' (prev), '>' (next), or 'q' (quit) > ", page+1, total_pages);
                    fflush(stdout);
                    if (!fgets(cmd, sizeof(cmd), stdin)) break;
                    if (cmd[0] == 'q' || cmd[0] == 'Q') break;
                    if (cmd[0] == '<' && page > 0) page--;
                    else if (cmd[0] == '>' && page < total_pages - 1) page++;
                }
            }
            free(results);
        } else {
            odbc_extract_error("SQLExecute Find", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Find", stmt, SQL_HANDLE_STMT);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

void customer_list_products(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[32];
    int cnum = 0;
    char pname[71];
    int total = 0;

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (!fgets(input, sizeof(input), stdin)) return;
    cnum = atoi(input);

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT p.productname, SUM(od.quantityordered) FROM products p JOIN orderdetails od ON p.productcode = od.productcode JOIN orders o ON od.ordernumber = o.ordernumber WHERE o.customernumber = ? GROUP BY p.productcode, p.productname ORDER BY p.productcode", SQL_NTS))) {
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cnum, 0, NULL);
        
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*)pname, sizeof(pname), NULL);
            SQLBindCol(stmt, 2, SQL_C_SLONG, &total, 0, NULL);

            typedef struct { char pn[71]; int t; } Res;
            Res* results = NULL;
            int count = 0, capacity = 10;
            results = malloc(capacity * sizeof(Res));

            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                if (count >= capacity) {
                    capacity *= 2;
                    results = realloc(results, capacity * sizeof(Res));
                }
                strcpy(results[count].pn, pname);
                results[count].t = total;
                count++;
            }

            if (count == 0) {
                printf("No se encontraron productos para este cliente.\n");
            } else {
                int page = 0;
                int total_pages = (count + 9) / 10;
                char cmd[16];
                
                while(1) {
                    int start = page * 10;
                    int end = (start + 10 > count) ? count : start + 10;
                    for (int i = start; i < end; i++) {
                        printf("%s %d\n", results[i].pn, results[i].t);
                    }
                    if (total_pages <= 1) break;
                    
                    printf("\nPage %d of %d. Enter '<' (prev), '>' (next), or 'q' (quit) > ", page+1, total_pages);
                    fflush(stdout);
                    if (!fgets(cmd, sizeof(cmd), stdin)) break;
                    if (cmd[0] == 'q' || cmd[0] == 'Q') break;
                    if (cmd[0] == '<' && page > 0) page--;
                    else if (cmd[0] == '>' && page < total_pages - 1) page++;
                }
            }
            free(results);
        } else {
            odbc_extract_error("SQLExecute List Products", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare List Products", stmt, SQL_HANDLE_STMT);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

void customer_balance(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[32];
    int cnum = 0;
    double balance = 0.0;

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (!fgets(input, sizeof(input), stdin)) return;
    cnum = atoi(input);

    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    char* query = "SELECT COALESCE((SELECT SUM(amount) FROM payments WHERE customernumber = ?), 0) - COALESCE((SELECT SUM(quantityordered * priceeach) FROM orderdetails od JOIN orders o ON od.ordernumber = o.ordernumber WHERE o.customernumber = ?), 0)";

    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)query, SQL_NTS))) {
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cnum, 0, NULL);
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cnum, 0, NULL);

        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            SQLBindCol(stmt, 1, SQL_C_DOUBLE, &balance, 0, NULL);
            if (SQL_SUCCEEDED(SQLFetch(stmt))) {
                printf("Balance = %.2f\n", balance);
            }
        } else {
            odbc_extract_error("SQLExecute Balance", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Balance", stmt, SQL_HANDLE_STMT);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

void customers_menu(SQLHDBC dbc) {
    int choice = 0;
    char buf[16];
    do {
        printf("\n (1) Find\n (2) List Products\n (3) Balance\n (4) Back\n\n");
        printf("Enter a number that corresponds to your choice > ");
        (void) fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) choice = atoi(buf);
        
        switch (choice) {
            case 1: customer_find(dbc); break;
            case 2: customer_list_products(dbc); break;
            case 3: customer_balance(dbc); break;
            case 4: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 4);
}