#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "products.h"
#include "orders.h"
#include "customers.h"

/**
 * @brief Muestra el menu principal y captura la seleccion del usuario.
 * 
 * @return Opcion seleccionada (entero).
 * @author Raducu Iulian Bidila y Rodrigo Lopez Vicente
 */
static int show_main_menu(void) {
    int choice = 0;
    char buf[16] = "";

    /* Impresion de las opciones principales */
    printf("\n (1) Products\n");
    printf(" (2) Orders\n");
    printf(" (3) Customers\n");
    printf(" (4) Quit\n\n");
    printf("Enter a number that corresponds to your choice > ");
    (void) fflush(stdout);
    
    /* Lectura segura de teclado */
    if (fgets(buf, (int)sizeof(buf), stdin) != NULL) {
        choice = atoi(buf);
    }
    return choice;
}

/**
 * @brief Punto de entrada principal. Gestiona la conexion ODBC y el bucle de la aplicacion.
 * 
 * @return Codigo de salida (EXIT_SUCCESS o EXIT_FAILURE).
 * @author Raducu Iulian Bidila y Rodrigo Lopez Vicente
 */
int main(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    int choice = 0;

    /* Establecimiento de la conexion con la base de datos */
    if (!SQL_SUCCEEDED(odbc_connect(&env, &dbc))) {
        fprintf(stderr, "Error conectando a la base de datos.\n");
        return EXIT_FAILURE;
    }

    /* Bucle principal de la aplicacion */
    do {
        choice = show_main_menu();
        switch (choice) {
            case 1:
                products_menu(dbc);
                break;
            case 2:
                orders_menu(dbc);
                break;
            case 3:
                customers_menu(dbc);
                break;
            case 4:
                printf("Bye Bye\n\n");
                break;
            default:
                printf("You have entered an invalid choice. Please try again\n\n");
        }
    } while (choice != 4);

    /* Cierre y liberacion de recursos ODBC */
    (void) odbc_disconnect(env, dbc);
    return EXIT_SUCCESS;
}