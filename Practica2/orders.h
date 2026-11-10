#ifndef ORDERS_H
#define ORDERS_H

#include <sql.h>
#include <sqlext.h>

void orders_menu(SQLHDBC dbc);
void order_open(SQLHDBC dbc);
void order_range(SQLHDBC dbc);
void order_detail(SQLHDBC dbc);

#endif
