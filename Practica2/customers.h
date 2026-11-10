#ifndef CUSTOMERS_H
#define CUSTOMERS_H

#include <sql.h>
#include <sqlext.h>

void customers_menu(SQLHDBC dbc);
void customer_find(SQLHDBC dbc);
void customer_list_products(SQLHDBC dbc);
void customer_balance(SQLHDBC dbc);

#endif