CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LDLIBS = -lodbc

export PGDATABASE:=classicmodels
export PGUSER:=alumnodb
export PGPASSWORD:=alumnodb
export PGHOST:=localhost

PSQL = psql
CREATEDB = createdb
DROPDB = dropdb --if-exists

OBJS = main.o odbc.o products.o orders.o customers.o

compile: $(OBJS)
	$(CC) $(CFLAGS) -o menu $(OBJS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: dropdb createdb restore compile

createdb:
	@echo Creando BBDD
	@$(CREATEDB) $(PGDATABASE)

dropdb:
	@echo Eliminando BBDD
	@$(DROPDB) $(PGDATABASE)

restore:
	@echo Restaurando datos de classicmodels
	@cat classicmodels.sql | $(PSQL)

clean:
	rm -f *.o menu splint.log