#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "products.h"
#include "customers.h"

void orders_menu(SQLHDBC dbc);

int show_main_menu(void) {
    int choice = 0;
    char buf[16];

    printf("\n (1) Products\n");
    printf(" (2) Orders\n");
    printf(" (3) Customers\n");
    printf(" (4) Quit\n\n");
    printf("Enter a number that corresponds to your choice > ");
    (void) fflush(stdout);
    
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        choice = atoi(buf);
    }
    return choice;
}

int main(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    int choice = 0;

    if (!SQL_SUCCEEDED(odbc_connect(&env, &dbc))) {
        fprintf(stderr, "Error conectando a la base de datos.\n");
        return EXIT_FAILURE;
    }

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

    odbc_disconnect(env, dbc);
    return EXIT_SUCCESS;
}