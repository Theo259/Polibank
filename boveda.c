#include <stdio.h>
#include <string.h>
#include <time.h>
#include "polibank.h"
#include "registro_boveda.h"

// autenticarUsuario
// Realiza una búsqueda secuencial para validar el nombre y la clave.
// Retorna la posición del usuario en el arreglo o -1 si no coincide.

int autenticarUsuario(Usuario listaUsuarios[], int totalUsuarios,
                       const char *usuarioIngresado, const char *claveIngresada) {
    for (int i = 0; i < totalUsuarios; i++) {
        // strcmp devuelve 0 cuando dos textos son iguales
        if (strcmp(listaUsuarios[i].usuario, usuarioIngresado) == 0 &&
            strcmp(listaUsuarios[i].clave, claveIngresada) == 0) {
            return i; // se encontro, se devuelve su posicion en la lista
        }
    }
    return -1; // no se encontro a nadie con ese usuario y clave
}

// obtenerFechaHoraActual (función estática)
// Obtiene la fecha y hora del sistema y la formatea en el buffer
// con la estructura dd/mm/aaaa hh:mm:ss.

static void obtenerFechaHoraActual(char *buffer, int tamano) {
    time_t ahora = time(NULL);       // pide al sistema la hora actual
    struct tm *t = localtime(&ahora); // la convierte a dia, mes, ano, hora, minuto, segundo
    strftime(buffer, tamano, "%d/%m/%Y %H:%M:%S", t); // arma el texto con ese formato
}

// abrirJornada
// Registra la apertura de operaciones en el archivo de log (en modo append).
// Guarda el evento, el usuario responsable y la marca de tiempo (timestamp).

void abrirJornada(const char *nombreUsuario, const char *rol) {
    FILE *log = fopen(ARCHIVO_LOG_BOVEDA, "a"); // "a" = agregar al final del archivo sin borrar lo anterior
    if (log == NULL) {
        printf("[Boveda] ERROR: no se pudo abrir el log de boveda.\n");
        return;
    }

    char fechaHora[30];
    obtenerFechaHoraActual(fechaHora, sizeof(fechaHora)); // se obtiene la fecha y hora de ahorita

    // se escriben varias lineas en el archivo con la informacion de la apertura
    fprintf(log, "==========================================\n");
    fprintf(log, "APERTURA DE JORNADA\n");
    fprintf(log, "Fecha/Hora : %s\n", fechaHora);
    fprintf(log, "Usuario    : %s (%s)\n", nombreUsuario, rol);
    fprintf(log, "==========================================\n");

    fclose(log); // se cierra el archivo
    printf("[Boveda] Jornada abierta por '%s'. Registrado en %s\n", nombreUsuario, ARCHIVO_LOG_BOVEDA);
}

// calcularTotalIngresosRecursivo / calcularTotalEgresosRecursivo
// Calcula la suma global de depósitos (ingresos) o retiros (egresos) 
// recorriendo las cuentas y sus movimientos mediante recursividad.
// Caso base: retorna 0 cuando no quedan más cuentas por procesar.

float calcularTotalIngresosRecursivo(Cuenta listaCuentas[], int indiceCuenta, int indiceTransaccion, int tamanoActual) {
    if (indiceCuenta >= tamanoActual) {
        return 0.0f; // ya no hay mas cuentas, aqui se para la recursividad
    }
    if (indiceTransaccion >= listaCuentas[indiceCuenta].numTransacciones) {
        // ya se revisaron todos los movimientos de esta cuenta, se pasa a la siguiente cuenta
        return calcularTotalIngresosRecursivo(listaCuentas, indiceCuenta + 1, 0, tamanoActual);
    }

    float montoActual = 0.0f;
    // si el movimiento actual es un deposito, se suma su monto
    if (strcmp(listaCuentas[indiceCuenta].historial[indiceTransaccion].tipo, "DEPOSITO") == 0) {
        montoActual = listaCuentas[indiceCuenta].historial[indiceTransaccion].monto;
    }

    // se suma este monto con el resultado de seguir revisando los siguientes movimientos
    return montoActual + calcularTotalIngresosRecursivo(listaCuentas, indiceCuenta, indiceTransaccion + 1, tamanoActual);
}

float calcularTotalEgresosRecursivo(Cuenta listaCuentas[], int indiceCuenta, int indiceTransaccion, int tamanoActual) {
    if (indiceCuenta >= tamanoActual) {
        return 0.0f; // ya no hay mas cuentas
    }
    if (indiceTransaccion >= listaCuentas[indiceCuenta].numTransacciones) {
        return calcularTotalEgresosRecursivo(listaCuentas, indiceCuenta + 1, 0, tamanoActual); // pasar a la siguiente cuenta
    }

    float montoActual = 0.0f;
    // si el movimiento actual es un retiro, se suma su monto
    if (strcmp(listaCuentas[indiceCuenta].historial[indiceTransaccion].tipo, "RETIRO") == 0) {
        montoActual = listaCuentas[indiceCuenta].historial[indiceTransaccion].monto;
    }

    return montoActual + calcularTotalEgresosRecursivo(listaCuentas, indiceCuenta, indiceTransaccion + 1, tamanoActual);
}

// calcularBalanceBovedaRecursivo
// Calcula recursivamente la suma de los saldos de todas las cuentas activas.
// El resultado representa el capital total que respalda al banco.

float calcularBalanceBovedaRecursivo(Cuenta listaCuentas[], int indice, int tamanoActual) {
    if (indice >= tamanoActual) {
        return 0.0f; // ya no hay mas cuentas que sumar
    }

    // si la cuenta esta activa se toma su saldo, si no, se toma 0
    float saldoCuentaActual = listaCuentas[indice].activo ? listaCuentas[indice].saldo : 0.0f;

    // se suma el saldo de esta cuenta con el resultado de sumar el resto de las cuentas
    return saldoCuentaActual + calcularBalanceBovedaRecursivo(listaCuentas, indice + 1, tamanoActual);
}

// cerrarJornada
// Calcula los ingresos, egresos y el balance total mediante las funciones 
// recursivas, y guarda este resumen financiero en el archivo de registro.

void cerrarJornada(Cuenta listaCuentas[], int tamanoActual, const char *nombreUsuario) {
    float totalIngresos = calcularTotalIngresosRecursivo(listaCuentas, 0, 0, tamanoActual); // suma de depositos
    float totalEgresos   = calcularTotalEgresosRecursivo(listaCuentas, 0, 0, tamanoActual);  // suma de retiros
    float balanceBoveda  = calcularBalanceBovedaRecursivo(listaCuentas, 0, tamanoActual);     // suma de todos los saldos

    FILE *log = fopen(ARCHIVO_LOG_BOVEDA, "a"); // se agrega al final del archivo de registro
    if (log == NULL) {
        printf("[Boveda] ERROR: no se pudo abrir el log de boveda.\n");
        return;
    }

    char fechaHora[30];
    obtenerFechaHoraActual(fechaHora, sizeof(fechaHora));

    // se escribe el resumen del cierre en el archivo
    fprintf(log, "------------------------------------------\n");
    fprintf(log, "CIERRE DE JORNADA\n");
    fprintf(log, "Fecha/Hora        : %s\n", fechaHora);
    fprintf(log, "Cerrado por       : %s\n", nombreUsuario);
    fprintf(log, "Total Ingresos    : $%.2f\n", totalIngresos);
    fprintf(log, "Total Egresos     : $%.2f\n", totalEgresos);
    fprintf(log, "Balance de Boveda : $%.2f\n", balanceBoveda);
    fprintf(log, "==========================================\n\n");

    fclose(log);

    // se muestra el mismo resumen en pantalla
    printf("\n[Boveda] ===== CIERRE DE JORNADA =====\n");
    printf("Total Ingresos    : $%.2f\n", totalIngresos);
    printf("Total Egresos     : $%.2f\n", totalEgresos);
    printf("Balance de Boveda : $%.2f\n", balanceBoveda);
    printf("Registrado en '%s'\n", ARCHIVO_LOG_BOVEDA);
}
