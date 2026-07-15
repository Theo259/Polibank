#include <stdio.h>
#include <string.h>
#include "polibank.h"
#include "registro_boveda.h"

// cargarCuentasDesdeArchivo
 // Carga los datos del archivo directamente en el arreglo "listaCuentas".
 // Retorna: La cantidad de cuentas que fueron encontradas y leídas.
 
int cargarCuentasDesdeArchivo(Cuenta listaCuentas[], const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "rb"); // "rb" = abrir para leer en binario
    if (archivo == NULL) {
        // si no se pudo abrir, es porque el archivo todavia no existe (primera vez que se usa el programa)
        printf("[Registro] No existe aun el archivo '%s'. Se iniciara vacio.\n", nombreArchivo);
        return 0; // se devuelve 0 cuentas porque no hay nada que cargar
    }

    int total = 0; // aqui se va contando cuantas cuentas se leyeron
    // fread lee una cuenta del archivo y la pone en listaCuentas[total];
    // el ciclo sigue mientras se puedan seguir leyendo cuentas y no se pase del maximo
    while (total < MAX_CUENTAS && fread(&listaCuentas[total], sizeof(Cuenta), 1, archivo) == 1) {
        total++; // se suma 1 porque se acaba de leer una cuenta mas
    }

    fclose(archivo); // se cierra el archivo porque ya no se necesita abierto
    printf("[Registro] Se cargaron %d cuenta(s) desde '%s'.\n", total, nombreArchivo);
    return total; // se devuelve cuantas cuentas quedaron cargadas en memoria
}

// guardarCuentasEnArchivo
 // Escribe TODO el arreglo de cuentas en el archivo, de una sola vez.
// Se llama cada vez que algo cambia (se crea una cuenta, se deposita, se
// transfiere, etc.) para que esos cambios no se pierdan.

void guardarCuentasEnArchivo(Cuenta listaCuentas[], int tamanoActual, const char *nombreArchivo) {
    FILE *archivo = fopen(nombreArchivo, "wb"); // "wb" = abrir para escribir en binario (borra lo que hubiera antes)
    if (archivo == NULL) {
        printf("[Registro] ERROR: no se pudo abrir '%s' para escritura.\n", nombreArchivo);
        return; // si no se pudo abrir el archivo, no hay nada mas que hacer aqui
    }

    fwrite(listaCuentas, sizeof(Cuenta), tamanoActual, archivo); // escribe las "tamanoActual" cuentas de un jalon
    fclose(archivo); // se cierra el archivo
    printf("[Registro] PROCESO GENERADO EXITOSAMENTE\n");
}

// ordenarPorNumeroCuenta
// Ordena el arreglo de cuentas de menor a mayor usando el método de inserción.
// Es un paso necesario previo a realizar cualquier búsqueda binaria.

void ordenarPorNumeroCuenta(Cuenta listaCuentas[], int tamanoActual) {
    int i, j;
    Cuenta clave; // aqui se guarda temporalmente la cuenta que se esta acomodando

    for (i = 1; i < tamanoActual; i++) {
        clave = listaCuentas[i]; // se guarda una copia de la cuenta actual
        j = i - 1;

        // mientras la cuenta de la izquierda tenga un numero mayor, se recorre un lugar a la derecha
        while (j >= 0 && listaCuentas[j].numeroCuenta > clave.numeroCuenta) {
            listaCuentas[j + 1] = listaCuentas[j];
            j--;
        }
        listaCuentas[j + 1] = clave; // aqui se pone la cuenta guardada en el lugar que le corresponde
    }
}

// buscarCuentaBinaria
// Busca rápidamente un número de cuenta en el arreglo ya ordenado.
// Devuelve la posición de la cuenta o -1 si no la encuentra.

int buscarCuentaBinaria(Cuenta listaCuentas[], int tamanoActual, int numeroBuscar) {
    int bajo = 0;              // limite de abajo del pedazo que se esta revisando
    int alto = tamanoActual - 1; // limite de arriba del pedazo que se esta revisando

    while (bajo <= alto) {
        int medio = bajo + (alto - bajo) / 2; // posicion justo a la mitad

        if (listaCuentas[medio].numeroCuenta == numeroBuscar) {
            return medio; // se encontro la cuenta, se devuelve su posicion
        } else if (listaCuentas[medio].numeroCuenta < numeroBuscar) {
            bajo = medio + 1; // el numero buscado esta en la mitad de la derecha
        } else {
            alto = medio - 1; // el numero buscado esta en la mitad de la izquierda
        }
    }
    return -1; // se acabaron las mitades y no se encontro la cuenta
}

// consultarPerfilCuenta
// Ordena el arreglo, busca la cuenta mediante búsqueda binaria y, 
// si existe, muestra sus datos y movimientos en pantalla.

void consultarPerfilCuenta(Cuenta listaCuentas[], int tamanoActual, int numeroCuenta) {
    ordenarPorNumeroCuenta(listaCuentas, tamanoActual); // primero se ordena, la busqueda binaria lo necesita
    int idx = buscarCuentaBinaria(listaCuentas, tamanoActual, numeroCuenta); // se busca la cuenta

    if (idx == -1) {
        printf("\n[Registro] La cuenta N° %d no existe.\n", numeroCuenta);
        return; // no hay nada mas que mostrar si no existe
    }

    Cuenta c = listaCuentas[idx]; // se hace una copia para trabajar mas comodo

    if (!c.activo) {
        printf("\n[Registro] La cuenta N° %d fue eliminada (inactiva).\n", c.numeroCuenta);
        return;
    }

    // Se imprimen los datos principales de la cuenta
    printf("\n===== PERFIL DE CLIENTE =====\n");
    printf("Cuenta Nro   : %d\n", c.numeroCuenta);
    printf("Titular     : %s\n", c.nombre);
    printf("Saldo actual: $%.2f\n", c.saldo);
    printf("Estado      : %s\n", c.activo ? "ACTIVA" : "INACTIVA");
    printf("--- Historial de transacciones (%d) ---\n", c.numTransacciones);

    if (c.numTransacciones == 0) {
        printf("  (sin movimientos registrados)\n");
    } else {
        // se recorre el historial y se imprime cada movimiento, uno por uno
        for (int i = 0; i < c.numTransacciones; i++) {
            printf("  %d) %-13s $%9.2f   %s\n",
                   i + 1,
                   c.historial[i].tipo,
                   c.historial[i].monto,
                   c.historial[i].fecha);
        }
    }
    printf("==============================\n");
}

// exportarEstadoCuentaTXT
// Genera un archivo .txt con el estado de la cuenta.
// La cuenta se recibe por valor para evitar modificaciones accidentales.

void exportarEstadoCuentaTXT(Cuenta cuentaEspecifica) {
    char nombreArchivo[60];
    // arma el nombre del archivo usando el numero de cuenta, por ejemplo "estado_cuenta_1000.txt"
    sprintf(nombreArchivo, "estado_cuenta_%d.txt", cuentaEspecifica.numeroCuenta);

    FILE *archivo = fopen(nombreArchivo, "w"); // "w" = abrir para escribir texto normal
    if (archivo == NULL) {
        printf("[Registro] ERROR: no se pudo generar '%s'.\n", nombreArchivo);
        return;
    }

    // Se escribe el encabezado del recibo
    fprintf(archivo, "==========================================\n");
    fprintf(archivo, "        POLI BANK - ESTADO DE CUENTA       \n");
    fprintf(archivo, "==========================================\n");
    fprintf(archivo, "Cuenta Nro   : %d\n", cuentaEspecifica.numeroCuenta);
    fprintf(archivo, "Titular     : %s\n", cuentaEspecifica.nombre);
    fprintf(archivo, "Estado      : %s\n", cuentaEspecifica.activo ? "ACTIVA" : "INACTIVA");
    fprintf(archivo, "Saldo actual: $%.2f\n", cuentaEspecifica.saldo);
    fprintf(archivo, "------------------------------------------\n");
    fprintf(archivo, "Historial de movimientos:\n");

    if (cuentaEspecifica.numTransacciones == 0) {
        fprintf(archivo, "  (sin movimientos registrados)\n");
    } else {
        // se recorre el historial y se escribe cada movimiento en el archivo
        for (int i = 0; i < cuentaEspecifica.numTransacciones; i++) {
            fprintf(archivo, "  %2d) %-13s $%9.2f   Fecha: %s\n",
                    i + 1,
                    cuentaEspecifica.historial[i].tipo,
                    cuentaEspecifica.historial[i].monto,
                    cuentaEspecifica.historial[i].fecha);
        }
    }

    fprintf(archivo, "==========================================\n");
    fprintf(archivo, "   Gracias por confiar en POLI BANK        \n");
    fprintf(archivo, "==========================================\n");

    fclose(archivo); // se cierra el archivo, con eso queda guardado
    printf("[Registro] Estado de cuenta generado: '%s'\n", nombreArchivo);
}
