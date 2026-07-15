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

// MODIFICACION: Prototipos de funciones seguras de entrada de datos y PIN
void leerPinOfuscado(char *pinDestino);
int leerEnteroSeguro(void);
float leerFloatSeguro(void);

// MODIFICACION: Prototipo para ocultar con asteriscos la clave de Boveda (Cajero/Admin)
void leerClaveOfuscada(char *claveDestino, int maxLen);

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
 * MODIFICACION: leerPinOfuscado
 * Intercepta las teclas una por una con getch() de <conio.h> para no mostrarlas.
 * Solo acepta caracteres numericos del '0' al '9', dibuja un '*' y permite
 * retroceder y borrar con Backspace de forma visualmente limpia.
 * ==========================================================================*/
void leerPinOfuscado(char *pinDestino) {
    int i = 0;
    char ch;
    while (i < 4) {
        ch = getch(); // Lee del teclado directamente sin eco en pantalla
        if (ch >= '0' && ch <= '9') {
            pinDestino[i] = ch;
            printf("*"); // Imprime asterisco para ocultar el PIN real
            i++;
        } else if (ch == 8 && i > 0) { // Codigo ASCII 8 es retroceso (Backspace)
            i--;
            printf("\b \b"); // Truco estetico: borra el asterisco anterior en la terminal
        }
    }
    pinDestino[4] = '\0'; // Cierra la cadena para poder compararla con strcmp
}

/* ============================================================================
 * MODIFICACION: leerClaveOfuscada
 * Igual que leerPinOfuscado, pero pensada para la clave de Boveda: aqui SI
 * se aceptan letras y numeros (porque claves como "admin123" no son solo
 * digitos), y la captura termina cuando se presiona Enter, en vez de
 * pararse en un largo fijo de 4 caracteres como el PIN.
 * ==========================================================================*/
void leerClaveOfuscada(char *claveDestino, int maxLen) {
    int i = 0;
    char ch;
    while (i < maxLen - 1) {
        ch = getch(); // Lee del teclado directamente sin eco en pantalla
        if (ch == 13) { // Codigo ASCII 13 es Enter, termina la captura
            break;
        } else if (ch == 8 && i > 0) { // Codigo ASCII 8 es retroceso (Backspace)
            i--;
            printf("\b \b"); // Borra el asterisco anterior en la terminal
        } else if (ch != 8) {
            claveDestino[i] = ch;
            printf("*"); // Imprime asterisco para ocultar la clave real
            i++;
        }
    }
    claveDestino[i] = '\0'; // Cierra la cadena para poder compararla con strcmp
}

/* ============================================================================
 * MODIFICACION: leerEnteroSeguro
 * Lee un entero de la consola. Si el usuario ingresa letras o simbolos,
 * limpia el buffer stdin para evitar bucles infinitos y crash de consola.
 * ==========================================================================*/
int leerEnteroSeguro(void) {
    int numero;
    char temp;
    while (scanf("%d", &numero) != 1 || numero < 0) {
        while ((temp = getchar()) != '\n' && temp != EOF); // Vacia el buffer de entrada
        printf("[ERROR] Entrada invalida. Digite un numero positivo valido: ");
    }
    while ((temp = getchar()) != '\n' && temp != EOF); // Limpieza residual
    return numero;
}

/* ============================================================================
 * MODIFICACION: leerFloatSeguro
 * Valida montos de dinero flotantes evitando entradas incoherentes como
 * letras, simbolos o montos negativos (menores o iguales a cero).
 * ==========================================================================*/
float leerFloatSeguro(void) {
    float monto;
    char temp;
    while (scanf("%f", &monto) != 1 || monto <= 0) {
        while ((temp = getchar()) != '\n' && temp != EOF); // Vacia buffer para que no se raye el main
        printf("[ERROR] Entrada invalida. Digite un monto positivo: $");
    }
    while ((temp = getchar()) != '\n' && temp != EOF); // Limpieza residual
    return monto;
}

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
    nueva.numeroCuenta = 10000000 + *tamanoActual; // MODIFICACION: Genera cuentas unicas de 8 digitos estrictos
    nueva.activo = 1;              // 1 quiere decir que la cuenta queda activa
    nueva.numTransacciones = 0;    // todavia no tiene ningun movimiento

    printf("NOMBRE DEL TITULAR.. ");
    scanf("%s", nueva.nombre); // se guarda el nombre que se escriba

    // MODIFICACION: Registro y validacion de PIN obligatorio
    printf("CREA TU PIN DE SEGURIDAD (4 DIGITOS NUMERICOS).. ");
    leerPinOfuscado(nueva.pin);
    printf("\n");

    printf("SALDO INICIAL DE DEPOSITO.. ");
    nueva.saldo = leerFloatSeguro(); // MODIFICACION: Lectura a prueba de errores

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
    char pinIngresado[5];

    printf("==== INICIO DE SESION ====\n\n");
    printf("NUMERO DE CUENTA (8 DIGITOS).. ");
    numeroCuenta = leerEnteroSeguro(); // MODIFICACION: Lectura de cuenta controlada

    // se ordena y se busca la cuenta para revisar que si exista antes de dejarlo entrar
    ordenarPorNumeroCuenta(listaCuentas, tamanoActual);
    int idx = buscarCuentaBinaria(listaCuentas, tamanoActual, numeroCuenta);

    if (idx == -1 || !listaCuentas[idx].activo) {
        printf("\nCuenta no encontrada.\n");
        getch();
        return; // no se puede continuar si la cuenta no existe
    }

    // MODIFICACION: Solicitar PIN de acceso de manera ofuscada con asteriscos
    printf("INGRESA TU PIN DE 4 DIGITOS.. ");
    leerPinOfuscado(pinIngresado);
    printf("\n");

    if (strcmp(listaCuentas[idx].pin, pinIngresado) != 0) {
        printf("[ERROR] PIN de seguridad incorrecto.\n");
        getch();
        return; // Deniega el acceso si el PIN falla
    }

    float monto;       // aqui se guarda el monto que el cliente vaya a escribir
    int numeroDestino;  // aqui se guarda la cuenta destino cuando se hace una transferencia

    do {
        // MODIFICACION: Volvemos a buscar el indice por si las transacciones reordenaron el arreglo
        ordenarPorNumeroCuenta(listaCuentas, tamanoActual);
        int idxActual = buscarCuentaBinaria(listaCuentas, tamanoActual, numeroCuenta);
        
        consultarPerfilCuenta(listaCuentas, tamanoActual, numeroCuenta); // se muestran los datos y el historial

        printf("\n 1....DEPOSITAR");
        printf("\n 2....RETIRAR");
        printf("\n 3....TRANSFERIR DINERO");
        printf("\n 4....EXPORTAR ESTADO DE CUENTA (.txt)");
        printf("\n 5....CERRAR SESION");
        printf("\n 6....SALIR\n\n");
        printf("INGRESA TU OPCION..");
        opcion = leerEnteroSeguro(); // MODIFICACION: Lectura de opciones sin trabarse si ingresan letras

        switch (opcion) {
        case 1: // depositar
            {
                // MODIFICACION: Doble autenticacion de PIN obligatoria antes de operar transacciones
                char pinConfirmar[5];
                printf("\nCONFIRMA TU PIN DE TRANSACCION.. ");
                leerPinOfuscado(pinConfirmar);
                printf("\n");
                if (strcmp(listaCuentas[idxActual].pin, pinConfirmar) != 0) {
                    printf("[ERROR] Transaccion cancelada: PIN incorrecto.\n");
                    getch();
                    break;
                }

                printf("Monto a depositar.. $");
                monto = leerFloatSeguro(); // MODIFICACION: Entrada financiera a prueba de letras
                realizarDeposito(listaCuentas, tamanoActual, numeroCuenta, monto);
                getch();
                break;
            }
        case 2: // retirar
            {
                char pinConfirmar[5];
                printf("\nCONFIRMA TU PIN DE TRANSACCION.. ");
                leerPinOfuscado(pinConfirmar);
                printf("\n");
                if (strcmp(listaCuentas[idxActual].pin, pinConfirmar) != 0) {
                    printf("[ERROR] Transaccion cancelada: PIN incorrecto.\n");
                    getch();
                    break;
                }

                printf("Monto a retirar.. $");
                monto = leerFloatSeguro(); // MODIFICACION: Entrada financiera validada
                realizarRetiro(listaCuentas, tamanoActual, numeroCuenta, monto);
                getch();
                break;
            }
        case 3: // transferir
            {
                char pinConfirmar[5];
                printf("\nCONFIRMA TU PIN DE TRANSACCION.. ");
                leerPinOfuscado(pinConfirmar);
                printf("\n");
                if (strcmp(listaCuentas[idxActual].pin, pinConfirmar) != 0) {
                    printf("[ERROR] Transaccion cancelada: PIN incorrecto.\n");
                    getch();
                    break;
                }

                printf("Numero de cuenta destino (8 digitos).. ");
                numeroDestino = leerEnteroSeguro(); // MODIFICACION: Entrada de cuenta destino controlada
                printf("Monto a transferir.. $");
                monto = leerFloatSeguro(); // MODIFICACION: Entrada de dinero controlada
                procesarTransferencia(listaCuentas, tamanoActual, numeroCuenta, numeroDestino, monto);
                getch();
                break;
            }
        case 4: { // exportar estado de cuenta
            ordenarPorNumeroCuenta(listaCuentas, tamanoActual);
            int idxActualExport = buscarCuentaBinaria(listaCuentas, tamanoActual, numeroCuenta);
            exportarEstadoCuentaTXT(listaCuentas[idxActualExport]);
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
    leerClaveOfuscada(clave, sizeof(clave)); // MODIFICACION: Oculta la clave con asteriscos al escribirla
    printf("\n");

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
        opcion = leerEnteroSeguro(); // MODIFICACION: Entrada de opcion segura para boveda
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
        printf("BIENVENIDO AL POLIBANK 2.0\n\n");
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
        opcion = leerEnteroSeguro(); // MODIFICACION: Control de opcion a prueba de caracteres invalidos

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