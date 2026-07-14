#include <stdio.h>
#include <string.h>
#include <time.h>
#include "polibank.h"
#include "registro_boveda.h"

/* ============================================================================
 * agregarMovimiento (funcion de apoyo, solo se usa aqui dentro)
 * Cada vez que se deposita, se retira o se transfiere, hay que dejar
 * anotado ese movimiento dentro del historial de la cuenta. Esta funcion
 * hace justamente eso: mete un nuevo movimiento en el historial, con la
 * fecha de hoy.
 * ==========================================================================*/
static void agregarMovimiento(Cuenta *cuenta, const char *tipo, float monto) {
    if (cuenta->numTransacciones >= MAX_TRANSACCIONES) {
        // si ya no hay espacio en el historial, se avisa y no se guarda el movimiento
        printf("[Aviso] Esta cuenta ya alcanzo el maximo de movimientos guardados.\n");
        return;
    }

    time_t ahora = time(NULL);        // pregunta al sistema que dia es hoy
    struct tm *t = localtime(&ahora); // lo convierte a dia, mes y ano

    // "nuevo" apunta al siguiente espacio libre del historial de esta cuenta
    Transaccion *nuevo = &cuenta->historial[cuenta->numTransacciones];
    strcpy(nuevo->tipo, tipo);                                   // se copia el tipo de movimiento ("DEPOSITO", etc)
    nuevo->monto = monto;                                        // se guarda el monto del movimiento
    strftime(nuevo->fecha, sizeof(nuevo->fecha), "%d/%m/%Y", t);  // se guarda la fecha de hoy en formato DD/MM/AAAA

    cuenta->numTransacciones++; // se suma 1 porque ya hay un movimiento mas guardado
}

/* ============================================================================
 * realizarDeposito
 * Busca la cuenta con el numero indicado, y si existe y esta activa, le
 * suma el monto al saldo. Tambien deja el movimiento anotado en su
 * historial y guarda el cambio en el archivo.
 * ==========================================================================*/
void realizarDeposito(Cuenta listaCuentas[], int tamanoActual, int numCuenta, float monto) {
    if (monto <= 0) {
        // no tiene sentido depositar 0 o un monto negativo
        printf("Monto no valido.\n");
        return;
    }

    // se recorre el arreglo de cuentas una por una, buscando la que coincide
    for (int i = 0; i < tamanoActual; i++) {
        if (listaCuentas[i].numeroCuenta == numCuenta && listaCuentas[i].activo == 1) {
            listaCuentas[i].saldo = listaCuentas[i].saldo + monto; // se le suma el monto al saldo
            agregarMovimiento(&listaCuentas[i], "DEPOSITO", monto); // se anota el movimiento en su historial

            guardarCuentasEnArchivo(listaCuentas, tamanoActual, ARCHIVO_CUENTAS); // se guarda el cambio en el archivo

            printf("Deposito realizado. Nuevo saldo: $%.2f\n", listaCuentas[i].saldo);
            return; // ya se hizo el deposito, se termina la funcion aqui
        }
    }

    // si el ciclo termino y no se encontro la cuenta, se avisa
    printf("Cuenta no encontrada.\n");
}

/* ============================================================================
 * realizarRetiro
 * Igual que el deposito, pero restando el monto del saldo, y revisando
 * primero que la cuenta SI tenga suficiente dinero disponible.
 * ==========================================================================*/
void realizarRetiro(Cuenta listaCuentas[], int tamanoActual, int numCuenta, float monto) {
    if (monto <= 0) {
        printf("Monto no valido.\n");
        return;
    }

    for (int i = 0; i < tamanoActual; i++) {
        if (listaCuentas[i].numeroCuenta == numCuenta && listaCuentas[i].activo == 1) {
            if (monto > listaCuentas[i].saldo) {
                // no se puede retirar mas dinero del que hay disponible
                printf("Fondos insuficientes.\n");
                return;
            }

            listaCuentas[i].saldo = listaCuentas[i].saldo - monto;  // se le resta el monto al saldo
            agregarMovimiento(&listaCuentas[i], "RETIRO", monto);    // se anota el movimiento

            guardarCuentasEnArchivo(listaCuentas, tamanoActual, ARCHIVO_CUENTAS); // se guarda el cambio

            printf("Retiro realizado. Nuevo saldo: $%.2f\n", listaCuentas[i].saldo);
            return;
        }
    }

    printf("Cuenta no encontrada.\n");
}

/* ============================================================================
 * procesarTransferencia
 * Mueve dinero de una cuenta a otra: le resta el monto a la cuenta de
 * origen y se lo suma a la cuenta de destino. Para encontrar las cuentas
 * usa busqueda binaria (ordenarPorNumeroCuenta + buscarCuentaBinaria), que
 * es mas rapida que ir revisando cuenta por cuenta.
 * ==========================================================================*/
void procesarTransferencia(Cuenta listaCuentas[], int tamanoActual, int cuentaOrigen, int cuentaDestino, float monto) {
    if (cuentaOrigen == cuentaDestino) {
        // no tiene sentido transferirse dinero a uno mismo
        printf("No puedes transferir dinero a tu propia cuenta.\n");
        return;
    }

    if (monto <= 0) {
        printf("Monto no valido.\n");
        return;
    }

    // se ordenan las cuentas por numero para poder usar la busqueda binaria
    ordenarPorNumeroCuenta(listaCuentas, tamanoActual);
    int idxOrigen  = buscarCuentaBinaria(listaCuentas, tamanoActual, cuentaOrigen);
    int idxDestino = buscarCuentaBinaria(listaCuentas, tamanoActual, cuentaDestino);

    if (idxOrigen == -1 || !listaCuentas[idxOrigen].activo) {
        printf("La cuenta de origen no existe.\n");
        return;
    }
    if (idxDestino == -1 || !listaCuentas[idxDestino].activo) {
        printf("La cuenta de destino no existe.\n");
        return;
    }

    if (monto > listaCuentas[idxOrigen].saldo) {
        // no se puede transferir mas dinero del que hay disponible en el origen
        printf("Fondos insuficientes en la cuenta de origen.\n");
        return;
    }

    listaCuentas[idxOrigen].saldo  -= monto; // se le quita el dinero al origen
    listaCuentas[idxDestino].saldo += monto; // se le suma el dinero al destino

    // se anota el movimiento en el historial de las dos cuentas
    agregarMovimiento(&listaCuentas[idxOrigen], "TRANSFERENCIA", monto);
    agregarMovimiento(&listaCuentas[idxDestino], "TRANSFERENCIA", monto);

    guardarCuentasEnArchivo(listaCuentas, tamanoActual, ARCHIVO_CUENTAS); // se guarda el cambio en el archivo

    printf("Transferencia de $%.2f realizada con exito. Nuevo saldo: $%.2f\n", monto, listaCuentas[idxOrigen].saldo);
}
