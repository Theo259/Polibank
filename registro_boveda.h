#ifndef REGISTRO_BOVEDA_H
#define REGISTRO_BOVEDA_H

#include "polibank.h" // aqui esta la definicion de cuenta, la necesitamos para los prototipos de abajo

#define MAX_USUARIOS 10                     // cuantos cajeros/admins puede haber como maximo
#define ARCHIVO_CUENTAS   "cuentas.dat"      // nombre del archivo donde se guardan las cuentas
#define ARCHIVO_LOG_BOVEDA "boveda_log.txt"  // nombre del archivo donde se escribe el registro de la boveda

// ============================================================================
// ESTRUCTURA PARA CONTROL DE ACCESO (Cajero / Admin)
// Guarda el usuario, la clave y el rol de una persona que puede entrar a
// la boveda del banco.
// ============================================================================
typedef struct {
    char usuario[30]; // nombre con el que inicia sesion el cajero o admin
    char clave[30];   // contrasena de esa persona
    char rol[10];     // dice si es "ADMIN" o "CAJERO"
} Usuario;

// ============================================================================
// MODULO REGISTRO (consulta de perfiles + guardar/leer el archivo de cuentas)
// ============================================================================
int  cargarCuentasDesdeArchivo(Cuenta listaCuentas[], const char *nombreArchivo); // lee las cuentas del archivo y las mete al arreglo
void guardarCuentasEnArchivo(Cuenta listaCuentas[], int tamanoActual, const char *nombreArchivo); // escribe el arreglo completo al archivo
void ordenarPorNumeroCuenta(Cuenta listaCuentas[], int tamanoActual); // pone las cuentas en orden por numero, para poder buscarlas rapido
void consultarPerfilCuenta(Cuenta listaCuentas[], int tamanoActual, int numeroCuenta); // muestra en pantalla los datos de una cuenta

// Ya declaradas en polibank.h, se implementan aquí 
// int   buscarCuentaBinaria(Cuenta listaCuentas[], int tamanoActual, int numeroBuscar);
// void  exportarEstadoCuentaTXT(Cuenta cuentaEspecifica);
// float calcularBalanceBovedaRecursivo(Cuenta listaCuentas[], int indice, int tamanoActual);

// ============================================================================
// MODULO BOVEDA (control de acceso y jornada bancaria)
// ============================================================================
int  autenticarUsuario(Usuario listaUsuarios[], int totalUsuarios,
                        const char *usuarioIngresado, const char *claveIngresada); // revisa si el usuario y clave son correctos
void abrirJornada(const char *nombreUsuario, const char *rol); // anota en el log que se abrio el dia de trabajo
void cerrarJornada(Cuenta listaCuentas[], int tamanoActual, const char *nombreUsuario); // anota en el log el cierre del dia, con los totales
float calcularTotalIngresosRecursivo(Cuenta listaCuentas[], int indiceCuenta, int indiceTransaccion, int tamanoActual); // suma todos los depositos
float calcularTotalEgresosRecursivo(Cuenta listaCuentas[], int indiceCuenta, int indiceTransaccion, int tamanoActual); // suma todos los retiros

#endif
