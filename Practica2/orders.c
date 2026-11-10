#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orders.h"
#include "odbc.h"

/**
 * @brief Obtiene y muestra los identificadores de pedidos no enviados.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
static void order_open(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    int ordernumber = 0;

    /* Preparacion de la consulta SQL */
    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT ordernumber FROM orders WHERE shippeddate IS NULL ORDER BY ordernumber", SQL_NTS))) {
        
        /* Ejecucion y extraccion de resultados */
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            (void) SQLBindCol(stmt, 1, SQL_C_SLONG, &ordernumber, 0, NULL);
            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                printf("%d\n", ordernumber);
            }
        } else {
            odbc_extract_error("SQLExecute Open", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Open", stmt, SQL_HANDLE_STMT);
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

/**
 * @brief Muestra los pedidos realizados dentro de un rango de fechas.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
static void order_range(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[64] = "";
    char date1[11] = "", date2[11] = "";
    int ordernumber = 0;
    char orderdate[11] = "", shippeddate[11] = "";
    SQLLEN ind1 = 0, ind2 = 0;

    /* Peticion de fechas y parseo seguro */
    printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
    (void) fflush(stdout);
    if (!fgets(input, (int)sizeof(input), stdin)) return;
    
    if (sscanf(input, "%10s - %10s", date1, date2) != 2) {
        printf("Formato de fecha invalido.\n");
        return;
    }

    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT ordernumber, orderdate, shippeddate FROM orders WHERE orderdate >= ? AND orderdate <= ? ORDER BY ordernumber", SQL_NTS))) {
        
        /* Vinculacion de los parametros de fecha */
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, date1, 0, NULL);
        (void) SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, date2, 0, NULL);
        
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            (void) SQLBindCol(stmt, 1, SQL_C_SLONG, &ordernumber, 0, NULL);
            (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*)orderdate, (SQLLEN)sizeof(orderdate), &ind1);
            (void) SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*)shippeddate, (SQLLEN)sizeof(shippeddate), &ind2);
            
            /* Iteracion controlando los valores NULL en shippeddate */
            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                if (ind2 == SQL_NULL_DATA) {
                    printf("%d %s\n", ordernumber, orderdate);
                } else {
                    printf("%d %s %s\n", ordernumber, orderdate, shippeddate);
                }
            }
        } else {
            odbc_extract_error("SQLExecute Range", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Range", stmt, SQL_HANDLE_STMT);
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

/**
 * @brief Obtiene la fecha, el estado, el coste total y los productos de un pedido.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
static void order_detail(SQLHDBC dbc) {
    SQLHSTMT stmt = NULL;
    char input[32] = "";
    int ordernumber = 0;
    char orderdate[11] = "", status[16] = "";
    SQLLEN ind1 = 0, ind2 = 0;

    printf("Enter ordernumber > ");
    (void) fflush(stdout);
    if (!fgets(input, (int)sizeof(input), stdin)) return;
    ordernumber = atoi(input);

    /* 1. Consulta de Cabecera (orderdate, status) */
    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT orderdate, status FROM orders WHERE ordernumber = ?", SQL_NTS))) {
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &ordernumber, 0, NULL);
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*)orderdate, (SQLLEN)sizeof(orderdate), &ind1);
            (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*)status, (SQLLEN)sizeof(status), &ind2);
            
            if (SQL_SUCCEEDED(SQLFetch(stmt))) {
                printf("%s %s\n", orderdate, status);
            } else {
                printf("Pedido no encontrado.\n");
                (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
                return; /* Fin temprano si el pedido no existe */
            }
        } else {
            odbc_extract_error("SQLExecute Detail Header", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Detail Header", stmt, SQL_HANDLE_STMT);
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* 2. Consulta del Coste Total */
    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT SUM(quantityordered * priceeach) FROM orderdetails WHERE ordernumber = ?", SQL_NTS))) {
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &ordernumber, 0, NULL);
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            double total_cost = 0.0;
            SQLLEN ind = 0;
            (void) SQLBindCol(stmt, 1, SQL_C_DOUBLE, &total_cost, 0, &ind);
            if (SQL_SUCCEEDED(SQLFetch(stmt)) && ind != SQL_NULL_DATA) {
                printf("%.2f\n", total_cost);
            }
        } else {
            odbc_extract_error("SQLExecute Detail Cost", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Detail Cost", stmt, SQL_HANDLE_STMT);
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* 3. Consulta de Lineas de pedido */
    (void) SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)"SELECT productcode, quantityordered, priceeach FROM orderdetails WHERE ordernumber = ? ORDER BY orderlinenumber", SQL_NTS))) {
        (void) SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &ordernumber, 0, NULL);
        if (SQL_SUCCEEDED(SQLExecute(stmt))) {
            char productcode[16] = "";
            int quantity = 0;
            double price = 0.0;
            (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*)productcode, (SQLLEN)sizeof(productcode), NULL);
            (void) SQLBindCol(stmt, 2, SQL_C_SLONG, &quantity, 0, NULL);
            (void) SQLBindCol(stmt, 3, SQL_C_DOUBLE, &price, 0, NULL);
            
            while (SQL_SUCCEEDED(SQLFetch(stmt))) {
                printf("%s %d %.2f\n", productcode, quantity, price);
            }
        } else {
            odbc_extract_error("SQLExecute Detail Lines", stmt, SQL_HANDLE_STMT);
        }
    } else {
        odbc_extract_error("SQLPrepare Detail Lines", stmt, SQL_HANDLE_STMT);
    }
    (void) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

/**
 * @brief Submenu de operaciones relacionadas con Orders.
 * 
 * @param dbc Manejador de conexion a la base de datos.
 * @author Raducu Iulian Bidila
 */
void orders_menu(SQLHDBC dbc) {
    int choice = 0;
    char buf[16] = "";
    do {
        printf("\n (1) Open\n (2) Range\n (3) Detail\n (4) Back\n\n");
        printf("Enter a number that corresponds to your choice > ");
        (void) fflush(stdout);
        if (fgets(buf, (int)sizeof(buf), stdin)) choice = atoi(buf);
        
        switch (choice) {
            case 1: order_open(dbc); break;
            case 2: order_range(dbc); break;
            case 3: order_detail(dbc); break;
            case 4: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 4);
}