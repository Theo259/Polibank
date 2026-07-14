// ============================================================================
// main.c - POLI BANK
// Este es el archivo principal: aqui arranca el programa (funcion main) y
// se muestran los menus. Desde aqui se llaman las funciones que estan
// escritas en los otros archivos (registro.c, boveda.c, transacciones.c).
// ============================================================================
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "polibank.h"
#include "registro_boveda.h"

// Se avisa al compilador que estas funciones existen (estan escritas mas abajo)
void gotoxy(int x, int y);
void crearCuentaMenu(Cuenta listaCuentas[], int *tamanoActual);
void menuCliente(Cuenta listaCuentas[], int tamanoActual);
void menuBoveda(Cuenta listaCuentas[], int tamanoActual);
void cerrarSesion(void);

/* ============================================================================
 * gotoxy
 * Mueve el cursor de la consola a la columna "x" y la fila "y", para poder
 * imprimir texto en un lugar especifico de la pantalla (y no siempre nada
 * mas hacia abajo).
 * ==========================================================================*/
void gotoxy(int x, int y)
{
    COORD c;      // COORD es un tipo de dato de Windows que guarda una columna y una fila
    c.X = x;      // se guarda la columna pedida
    c.Y = y;      // se guarda la fila pedida

    SetConsoleCursorPosition(
        GetStdHandle(STD_OUTPUT_HANDLE), c); // se manda la posicion a la consola
}

// Lista de personas que pueden entrar al modulo de Boveda (Cajero/Admin).
// Se deja escrita aqui mismo porque ningun archivo trae de donde sacarla.
Usuario listaUsuariosBoveda[MAX_USUARIOS] = {
    {"admin", "admin123", "ADMIN"},
    {"cajero1", "cajero123", "CAJERO"}
};
int totalUsuariosBoveda = 2; // cuantas personas hay en la lista de arriba

/* ============================================================================
 * crearCuentaMenu
 * Pregunta el nombre del titular y el saldo inicial, arma una cuenta nueva
 * y la agrega al arreglo de cuentas. Al final guarda todo en el archivo.
 * ==========================================================================*/
void crearCuentaMenu(Cuenta listaCuentas[], int *tamanoActual)
{
    system("cls"); // limpia la pantalla
    printf("!!!!!CREAR CUENTA!!!!!\n\n");

    if (*tamanoActual >= MAX_CUENTAS) {
        // ya no caben mas cuentas en el arreglo
        printf("No se pueden crear mas cuentas (limite de %d alcanzado).\n", MAX_CUENTAS);
        getch(); // espera a que el usuario presione una tecla
        return;
    }

    Cuenta nueva; // aqui se arma la cuenta nueva antes de meterla al arreglo
    nueva.numeroCuenta = 1000 + *tamanoActual; // se le da un numero de cuenta que no se repite
    nueva.activo = 1;              // 1 quiere decir que la cuenta queda activa
    nueva.numTransacciones = 0;    // todavia no tiene ningun movimiento

    printf("NOMBRE DEL TITULAR.. ");
    scanf("%s", nueva.nombre); // se guarda el nombre que se escriba

    printf("SALDO INICIAL DE DEPOSITO.. ");
    scanf("%f", &nueva.saldo); // se guarda el saldo que se escriba
    if (nueva.saldo < 0) nueva.saldo = 0; // no se permite empezar con saldo negativo

    listaCuentas[*tamanoActual] = nueva; // se mete la cuenta nueva en el siguiente espacio libre
    (*tamanoActual)++;                    // se suma 1 porque ya hay una cuenta mas

    guardarCuentasEnArchivo(listaCuentas, *tamanoActual, ARCHIVO_CUENTAS); // se guarda el cambio en el archivo

    printf("\nCUENTA CREADA EXITOSAMENTE....\n");
    printf("TU NUMERO DE CUENTA ES: %d (guardalo, lo vas a necesitar para iniciar sesion)\n", nueva.numeroCuenta);
    printf("\nPresiona una tecla para continuar..");
    getch();
}

/* ============================================================================
 * menuCliente
 * Aqui entra un cliente despues de escribir su numero de cuenta. Se le
 * muestra su perfil y un menu con las operaciones que puede hacer:
 * depositar, retirar, transferir, exportar su estado de cuenta o salir.
 * ==========================================================================*/
void menuCliente(Cuenta listaCuentas[], int tamanoActual)
{
    system("cls");
    int numeroCuenta, opcion;

    printf("==== INICIO DE SESION ====\n\n");
    printf("NUMERO DE CUENTA.. ");
    scanf("%d", &numeroCuenta); // se pide el numero de cuenta con el que se va a trabajar

    // se ordena y se busca la cuenta para revisar que si exista antes de dejarlo entrar
    ordenarPorNumeroCuenta(listaCuentas, tamanoActual);
    int idx = buscarCuentaBinaria(listaCuentas, tamanoActual, numeroCuenta);

    if (idx == -1 || !listaCuentas[idx].activo) {
        printf("\nCuenta no encontrada.\n");
        getch();
        return; // no se puede continuar si la cuenta no existe
    }

    float monto;       // aqui se guarda el monto que el cliente vaya a escribir
    int numeroDestino;  // aqui se guarda la cuenta destino cuando se hace una transferencia

    do {
        consultarPerfilCuenta(listaCuentas, tamanoActual, numeroCuenta); // se muestran los datos y el historial

        printf("\n 1....DEPOSITAR");
        printf("\n 2....RETIRAR");
        printf("\n 3....TRANSFERIR DINERO");
        printf("\n 4....EXPORTAR ESTADO DE CUENTA (.txt)");
        printf("\n 5....CERRAR SESION");
        printf("\n 6....SALIR\n\n");
        printf("INGRESA TU OPCION..");
        scanf("%d", &opcion); // se lee la opcion elegida

        switch (opcion) {
        case 1: // depositar
            printf("Monto a depositar.. $");
            scanf("%f", &monto);
            realizarDeposito(listaCuentas, tamanoActual, numeroCuenta, monto);
            getch();
            break;
        case 2: // retirar
            printf("Monto a retirar.. $");
            scanf("%f", &monto);
            realizarRetiro(listaCuentas, tamanoActual, numeroCuenta, monto);
            getch();
            break;
        case 3: // transferir
            printf("Numero de cuenta destino.. ");
            scanf("%d", &numeroDestino);
            printf("Monto a transferir.. $");
            scanf("%f", &monto);
            procesarTransferencia(listaCuentas, tamanoActual, numeroCuenta, numeroDestino, monto);
            getch();
            break;
        case 4: { // exportar estado de cuenta
            // se vuelve a buscar la cuenta por si su posicion cambio al ordenar
            ordenarPorNumeroCuenta(listaCuentas, tamanoActual);
            int idxActual = buscarCuentaBinaria(listaCuentas, tamanoActual, numeroCuenta);
            exportarEstadoCuentaTXT(listaCuentas[idxActual]);
            getch();
            break;
        }
        case 5: // cerrar sesion
            cerrarSesion();
            return;
        case 6: // salir del programa
            exit(0);
        }
        system("cls"); // se limpia la pantalla antes de volver a mostrar el menu
    } while (opcion != 5);
}

/* ============================================================================
 * menuBoveda
 * Aqui entra un Cajero o un Admin. Primero se revisan sus credenciales, si
 * son correctas se abre la jornada de trabajo, y al final se puede cerrar
 * esa jornada para que quede el resumen anotado en el archivo de registro.
 * ==========================================================================*/
void menuBoveda(Cuenta listaCuentas[], int tamanoActual)
{
    system("cls");
    char usuario[30], clave[30];
    int opcion;

    printf("==== ACCESO BOVEDA (CAJERO/ADMIN) ====\n\n");
    printf("USUARIO.. ");
    scanf("%s", usuario);
    printf("CLAVE.. ");
    scanf("%s", clave);

    // se revisa si el usuario y la clave escritos coinciden con alguien de la lista
    int idx = autenticarUsuario(listaUsuariosBoveda, totalUsuariosBoveda, usuario, clave);

    if (idx == -1) {
        printf("\nCredenciales invalidas.\n");
        getch();
        return; // no se puede continuar si las credenciales estan mal
    }

    abrirJornada(listaUsuariosBoveda[idx].usuario, listaUsuariosBoveda[idx].rol); // se anota la apertura del dia

    do {
        printf("\n==== JORNADA BANCARIA ====\n");
        printf(" 1....CERRAR JORNADA Y GENERAR LOG\n");
        printf(" 2....SALIR SIN CERRAR\n\n");
        printf("INGRESA TU OPCION..");
        scanf("%d", &opcion);
    } while (opcion != 1 && opcion != 2); // se repite hasta que escriban 1 o 2

    if (opcion == 1) {
        cerrarJornada(listaCuentas, tamanoActual, listaUsuariosBoveda[idx].usuario); // se anota el cierre del dia
    }

    printf("\nPresiona una tecla para continuar..");
    getch();
}

/* ============================================================================
 * cerrarSesion
 * Muestra un mensaje avisando que la sesion del cliente se cerro.
 * ==========================================================================*/
void cerrarSesion(void)
{
    system("cls");
    printf("Cerrando sesion...\n");
    printf("Sesion cerrada exitosamente..\n");
    printf("\nPresiona una tecla para continuar..");
    getch();
}

/* ============================================================================
 * main
 * Aqui empieza el programa: primero se cargan las cuentas guardadas, y
 * despues se muestra el menu principal hasta que el usuario decida salir.
 * ==========================================================================*/
int main()
{
    Cuenta listaCuentas[MAX_CUENTAS]; // aqui se guardan todas las cuentas mientras el programa esta abierto
    int tamanoActual = cargarCuentasDesdeArchivo(listaCuentas, ARCHIVO_CUENTAS); // se leen las cuentas del archivo
    int opcion;

    do {
        system("cls"); // limpia la pantalla antes de dibujar el menu
        gotoxy(20, 3);
        printf("BIENVENIDO AL SISTEMA BANCARIO\n\n");
        gotoxy(18, 5);
        printf("**********************************");
        gotoxy(25, 7);
        printf("----GRUPO 3----");
        gotoxy(20, 10);
        printf("1.... CREAR UNA CUENTA BANCARIA");
        gotoxy(20, 12);
        printf("2.... YA ERES USUARIO? INICIA SESION");
        gotoxy(20, 14);
        printf("3.... ACCESO BOVEDA (CAJERO/ADMIN)");
        gotoxy(20, 16);
        printf("4.... SALIR\n\n");

        printf("\n\nINGRESA TU OPCION..");
        scanf("%d", &opcion); // se lee la opcion elegida por el usuario

        switch (opcion) {
        case 1:
            crearCuentaMenu(listaCuentas, &tamanoActual); // se manda el arreglo y su tamano "por referencia" (con &) para poder cambiarlos
            break;
        case 2:
            menuCliente(listaCuentas, tamanoActual);
            break;
        case 3:
            menuBoveda(listaCuentas, tamanoActual);
            break;
        case 4:
            exit(0); // termina el programa
        }
    } while (1); // el menu se repite para siempre, hasta que se elija la opcion de salir

    return 0;
}
