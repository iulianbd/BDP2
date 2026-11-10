#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "customers.h"
#include "odbc.h"

/* Estructuras de almacenamiento para paginacion en memoria */
typedef struct { int n; char cn[51]; char fn[51]; char ln[51]; } ResFind;
typedef struct { char pn[71]; int t; } ResProd;

/**
 * @brief Busca clientes por coincidencias parciales en nombre o apellido.
 *        Muestra los resultados con paginacion interactiva.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
static void customer_find(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[64] = "", pattern[70] = "";
    int cnum = 0;
    char cname[51] = "", fname[51] = "", lname[51] = "";
    
    ResFind* results = NULL;
    int count = 0, capacity = 10;
    int page = 0, total_pages = 0, start = 0, end = 0, i = 0;
    char cmd[16] = "";

    printf("Enter customer name > ");
    (void) fflush(stdout);
    if (!fgets(input, (int)sizeof(input), stdin)) return;
    input[strcspn(input, "\r\n")] = '\0';
    
    /* Envolvemos el texto entre porcentajes para la clausula LIKE */
    (void) snprintf(pattern, sizeof(pattern), "%%%s%%", input);

    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT customernumber, customername, contactfirstname, contactlastname FROM customers WHERE contactfirstname LIKE ? OR contactlastname LIKE ? ORDER BY customernumber", SQL_NTS))) {
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, pattern, 0, NULL);
        (void) SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, pattern, 0, NULL);

        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            (void) SQLBindCol(stmt, 1, SQL_C_SLONG, &cnum, 0, NULL);
            (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*)cname, (SQLLEN)sizeof(cname), NULL);
            (void) SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*)fname, (SQLLEN)sizeof(fname), NULL);
            (void) SQLBindCol(stmt, 4, SQL_C_CHAR, (SQLCHAR*)lname, (SQLLEN)sizeof(lname), NULL);

            /* Reserva inicial de memoria para los resultados */
            results = calloc((size_t)capacity, sizeof(ResFind));
            if (results == NULL) {
                (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                return;
            }

            /* Almacenamiento dinamico de los registros */
            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                if (count >= capacity) {
                    ResFind* temp = NULL;
                    int old_cap = capacity;
                    capacity *= 2;
                    temp = calloc((size_t)capacity, sizeof(ResFind));
                    if (temp == NULL) {
                        free(results);
                        (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                        return;
                    }
                    temp = (ResFind*) memcpy(temp, results, (size_t)old_cap * sizeof(ResFind));
                    free(results);
                    results = temp;
                }
                results[count].n = cnum;
                (void) strcpy(results[count].cn, cname);
                (void) strcpy(results[count].fn, fname);
                (void) strcpy(results[count].ln, lname);
                count++;
            }

            /* Logica interactiva de paginacion */
            if (count == 0) {
                printf("No se encontraron clientes.\n");
            } else {
                page = 0;
                total_pages = (count + 9) / 10;
                
                for (;;) {
                    start = page * 10;
                    end = (start + 10 > count) ? count : start + 10;
                    for (i = start; i < end; i++) {
                        printf("%d %s %s %s\n", results[i].n, results[i].cn, results[i].fn, results[i].ln);
                    }
                    if (total_pages <= 1) break;
                    
                    printf("\nPage %d of %d. Enter '<' (prev), '>' (next), or 'q' (quit) > ", page+1, total_pages);
                    (void) fflush(stdout);
                    if (!fgets(cmd, (int)sizeof(cmd), stdin)) break;
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
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

/**
 * @brief Lista los productos comprados por un cliente concreto con el recuento total.
 *        Muestra los resultados con paginacion interactiva.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
static void customer_list_products(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[32] = "";
    int cnum = 0;
    char pname[71] = "";
    int total = 0;
    
    ResProd* results = NULL;
    int count = 0, capacity = 10;
    int page = 0, total_pages = 0, start = 0, end = 0, i = 0;
    char cmd[16] = "";

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (!fgets(input, (int)sizeof(input), stdin)) return;
    cnum = atoi(input);

    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT p.productname, SUM(od.quantityordered) FROM products p JOIN orderdetails od ON p.productcode = od.productcode JOIN orders o ON od.ordernumber = o.ordernumber WHERE o.customernumber = ? GROUP BY p.productcode, p.productname ORDER BY p.productcode", SQL_NTS))) {
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cnum, 0, NULL);
        
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*)pname, (SQLLEN)sizeof(pname), NULL);
            (void) SQLBindCol(stmt, 2, SQL_C_SLONG, &total, 0, NULL);

            /* Reserva inicial de memoria */
            results = calloc((size_t)capacity, sizeof(ResProd));
            if (results == NULL) {
                (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                return;
            }

            /* Almacenamiento y expansion dinamica */
            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                if (count >= capacity) {
                    ResProd* temp = NULL;
                    int old_cap = capacity;
                    capacity *= 2;
                    temp = calloc((size_t)capacity, sizeof(ResProd));
                    if (temp == NULL) {
                        free(results);
                        (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                        return;
                    }
                    temp = (ResProd*) memcpy(temp, results, (size_t)old_cap * sizeof(ResProd));
                    free(results);
                    results = temp;
                }
                (void) strcpy(results[count].pn, pname);
                results[count].t = total;
                count++;
            }

            /* Logica interactiva de paginacion */
            if (count == 0) {
                printf("No se encontraron productos para este cliente.\n");
            } else {
                page = 0;
                total_pages = (count + 9) / 10;
                
                for(;;) {
                    start = page * 10;
                    end = (start + 10 > count) ? count : start + 10;
                    for (i = start; i < end; i++) {
                        printf("%s %d\n", results[i].pn, results[i].t);
                    }
                    if (total_pages <= 1) break;
                    
                    printf("\nPage %d of %d. Enter '<' (prev), '>' (next), or 'q' (quit) > ", page+1, total_pages);
                    (void) fflush(stdout);
                    if (!fgets(cmd, (int)sizeof(cmd), stdin)) break;
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
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

/**
 * @brief Calcula y muestra el saldo (balance) de un cliente a traves de subconsultas COALESCE.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
static void customer_balance(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[32] = "";
    int cnum = 0;
    double balance = 0.0;
    
    /* Consulta optimizada que transfiere el calculo al motor de la base de datos */
    char* query = "SELECT COALESCE((SELECT SUM(amount) FROM payments WHERE customernumber = ?), 0) - COALESCE((SELECT SUM(quantityordered * priceeach) FROM orderdetails od JOIN orders o ON od.ordernumber = o.ordernumber WHERE o.customernumber = ?), 0)";

    printf("Enter customer number > ");
    (void) fflush(stdout);
    if (!fgets(input, (int)sizeof(input), stdin)) return;
    cnum = atoi(input);

    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)query, SQL_NTS))) {
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cnum, 0, NULL);
        (void) SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cnum, 0, NULL);

        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            (void) SQLBindCol(stmt, 1, SQL_C_DOUBLE, &balance, 0, NULL);
            if (SQL_SUCCEEDED(SQLFetch(stmt))) {
                printf("Balance = %.2f\n", balance);
            }
        } else {
            odbc_extract_error("SQLExecute Balance", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Balance", stmt, SQL_HANDLE_STMT);
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

/**
 * @brief Submenu de operaciones relacionadas con Customers.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
void customers_menu(SQLHDBC dbc) {
    int choice = 0;
    char buf[16] = "";
    do {
        printf("\n (1) Find\n (2) List Products\n (3) Balance\n (4) Back\n\n");
        printf("Enter a number that corresponds to your choice > ");
        (void) fflush(stdout);
        if (fgets(buf, (int)sizeof(buf), stdin)) choice = atoi(buf);
        
        switch (choice) {
            case 1: customer_find(dbc); break;
            case 2: customer_list_products(dbc); break;
            case 3: customer_balance(dbc); break;
            case 4: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 4);
}