#ifndef POLIBANK_H
#define POLIBANK_H

// Aqui se ponen los limites del sistema: cuantas cuentas puede haber como
// maximo, y cuantos movimientos puede guardar el historial de cada cuenta.
#define MAX_CUENTAS 100
#define MAX_TRANSACCIONES 10

// ============================================================================
// Un "Transaccion" es un solo movimiento de dinero (un deposito, un retiro o
// una transferencia) que se guarda dentro del historial de una cuenta.
// ============================================================================
typedef struct {
    char tipo[20];   // guarda el texto "DEPOSITO", "RETIRO" o "TRANSFERENCIA"
    float monto;     // guarda la cantidad de dinero de ese movimiento
    char fecha[11];  // guarda la fecha en formato DD/MM/AAAA
} Transaccion;

// ============================================================================
// Una "Cuenta" es la informacion de un cliente del banco: su numero, su
// nombre, cuanto dinero tiene, si sigue activa, y todos sus movimientos.
// Todos los campos tienen tamano fijo (no usan punteros) para poder
// guardar la cuenta completa en un archivo binario con fwrite/fread.
// ============================================================================
typedef struct {
    int numeroCuenta;      // el numero que identifica a la cuenta
    char nombre[50];       // el nombre del titular de la cuenta
    float saldo;           // cuanto dinero tiene disponible ahorita
    int activo;            // 1 quiere decir que la cuenta sigue existiendo, 0 que fue borrada

    Transaccion historial[MAX_TRANSACCIONES]; // aqui se guardan sus ultimos movimientos
    int numTransacciones;  // cuantos movimientos tiene guardados ahorita
} Cuenta;

// ============================================================================
// Lista de todas las funciones que se pueden usar desde cualquier archivo
// que incluya este header. Cada una se explica con detalle en el archivo
// donde esta escrita de verdad.
// ============================================================================

// --- Gestion de cuentas (crear, ver, editar, borrar) ---
void crearCuenta(Cuenta listaCuentas[], int *tamanoActual);
void listarCuentas(Cuenta listaCuentas[], int tamanoActual);
void actualizarCuenta(Cuenta listaCuentas[], int tamanoActual, int numCuenta);
void eliminarCuenta(Cuenta listaCuentas[], int tamanoActual, int numCuenta);

// --- Transacciones y transferencias (depositar, retirar, mover dinero) ---
int  buscarCuentaBinaria(Cuenta listaCuentas[], int tamanoActual, int numeroBuscar);
void realizarDeposito(Cuenta listaCuentas[], int tamanoActual, int numCuenta, float monto);
void realizarRetiro(Cuenta listaCuentas[], int tamanoActual, int numCuenta, float monto);
void procesarTransferencia(Cuenta listaCuentas[], int tamanoActual, int cuentaOrigen, int cuentaDestino, float monto);

// --- Reportes, boveda y funciones recursivas ---
void  ordenarCuentasQuicksort(Cuenta listaCuentas[], int bajo, int alto);
float calcularBalanceBovedaRecursivo(Cuenta listaCuentas[], int indice, int tamanoActual);
void  exportarEstadoCuentaTXT(Cuenta cuentaEspecifica);

#endif
