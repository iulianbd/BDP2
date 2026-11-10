#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "products.h"
#include "odbc.h"

static void product_stock(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char code[32] = "";
    int stock = 0;

    printf("Enter productcode > ");
    (void) fflush(stdout);
    if (!fgets(code, (int)sizeof(code), stdin)) return;
    code[strcspn(code, "\r\n")] = '\0';

    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    (void) SQLPrepare(stmt, (SQLCHAR*)"SELECT quantityinstock FROM products WHERE productcode = ?", SQL_NTS);
    (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, code, 0, NULL);
    
    if (SQL_SUCCEEDED(SQLExecute(stmt))) {
        (void) SQLBindCol(stmt, 1, SQL_C_SLONG, &stock, 0, NULL);
        if (SQL_SUCCEEDED(SQLFetch(stmt))) {
            printf("%d\n", stock);
        }
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

static void product_find(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[64] = "", pattern[70] = "";
    char pcode[16] = "", pname[71] = "";

    printf("Enter productname > ");
    (void) fflush(stdout);
    if (!fgets(input, (int)sizeof(input), stdin)) return;
    input[strcspn(input, "\r\n")] = '\0';

    (void) snprintf(pattern, sizeof(pattern), "%%%s%%", input);

    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    (void) SQLPrepare(stmt, (SQLCHAR*)"SELECT productcode, productname FROM products WHERE productname LIKE ? ORDER BY productcode", SQL_NTS);
    (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, pattern, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(stmt))) {
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*)pcode, (SQLLEN)sizeof(pcode), NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*)pname, (SQLLEN)sizeof(pname), NULL);
        while (SQL_SUCCEEDED(SQLFetch(stmt))) {
            printf("%s %s\n", pcode, pname);
        }
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

void products_menu(SQLHDBC dbc) {
    int choice = 0;
    char buf[16] = "";
    do {
        printf("\n (1) Stock\n (2) Find\n (3) Back\n\n");
        printf("Enter a number that corresponds to your choice > ");
        (void) fflush(stdout);
        if (fgets(buf, (int)sizeof(buf), stdin)) choice = atoi(buf);
        
        switch (choice) {
            case 1: product_stock(dbc); break;
            case 2: product_find(dbc); break;
            case 3: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 3);
}
