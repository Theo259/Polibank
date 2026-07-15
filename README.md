# Sistema Bancario POLIBANK en C

## Descripción

Este proyecto es un sistema bancario desarrollado en lenguaje C que simula la operación básica de un banco. Permite la creación de cuentas, el inicio de sesión de clientes, la realización de depósitos, retiros y transferencias, la consulta de perfiles y estados de cuenta, y el control de la bóveda (apertura y cierre de jornada) por parte de Cajeros y Administradores.

---

## Características

### Módulo de Cuentas

- Crear una nueva cuenta bancaria.
- Asignar automáticamente un número de cuenta único.
- Registrar nombre del titular y saldo inicial.
- Guardar la cuenta de forma persistente en archivo binario.

### Módulo de Registro

- Consultar el perfil completo de una cuenta datos y historial de movimientos.
- Ordenar las cuentas por número antes de buscarlas.
- Localizar una cuenta mediante búsqueda binaria.
- Exportar el estado de cuenta de un cliente a un archivo `.txt`.

### Módulo de Transacciones

- Realizar depósitos, validando que el monto sea mayor a cero.
- Realizar retiros, validando que existan fondos suficientes.
- Registrar cada movimiento en el historial de la cuenta con su fecha.

### Módulo de Transferencias

- Transferir dinero entre dos cuentas existentes.
- Ubicar la cuenta origen y destino mediante búsqueda binaria.
- Actualizar el saldo de ambas cuentas y registrar el movimiento en su historial.

### Módulo de Bóveda

- Control de acceso para perfiles de Cajero y Administrador.
- Abrir la jornada bancaria, dejando constancia de quién y cuándo la abrió.
- Cerrar la jornada, calculando ingresos, egresos y balance total.
- Generar un log de texto plano con cada apertura y cierre.

---

## Librerias utilizadas

  - `stdio.h`
  - `stdlib.h`
  - `string.h`
  - `conio.h`
  - `windows.h`
  - `time.h`

---

## Estructura de Archivos

El sistema genera y utiliza los siguientes archivos:

### `cuentas.dat`

Almacena en binario todas las cuentas registradas (número, titular, saldo, estado e historial de movimientos).

### `boveda_log.txt`

Guarda cada apertura y cierre de jornada bancaria, con fecha, usuario, rol y los totales calculados al cierre.

Ejemplo:
```
APERTURA DE JORNADA
Fecha/Hora : 14/07/2026 10:32:05
Usuario    : admin (ADMIN)
```

### `estado_cuenta_<numero>.txt`

Se genera uno por cada cliente que exporte su estado de cuenta, con su información y el detalle de sus movimientos.

Ejemplo: `estado_cuenta_1000.txt`

---

## Funcionamiento General

### Inicio del programa

El usuario puede seleccionar una de las siguientes opciones:

1. Crear una cuenta bancaria
2. Iniciar sesión (cliente)
3. Acceso Bóveda (Cajero/Admin)
4. Salir

### Flujo del Cliente

1. Inicia sesión con su número de cuenta.
2. Consulta su perfil y su historial de movimientos.
3. Puede depositar, retirar o transferir dinero.
4. Puede exportar su estado de cuenta.
5. Cierra sesión o sale del sistema.

### Flujo del Cajero/Administrador

1. Inicia sesión con usuario y clave.
2. Se abre la jornada bancaria automáticamente.
3. Puede cerrar la jornada, generando el log con los totales.

Credenciales predeterminadas:

- Usuario: `admin` — Contraseña: `admin123` (rol ADMIN)
- Usuario: `cajero1` — Contraseña: `cajero123` (rol CAJERO)

---

## Validaciones Implementadas

### Validación de operaciones con dinero

- El monto de un depósito, retiro o transferencia debe ser mayor a cero.
- No se permite retirar o transferir más dinero del disponible en el saldo.
- No se puede transferir dinero a la misma cuenta de origen.

### Validación de cuentas

- La cuenta debe existir y estar activa para poder operar sobre ella.
- Se respeta un límite máximo de cuentas (`MAX_CUENTAS`) y de movimientos guardados por cuenta (`MAX_TRANSACCIONES`).

---


## Algoritmos Implementados

- Búsqueda binaria para localizar cuentas por número.
- Ordenamiento por inserción para ordenar las cuentas por número antes de aplicar la búsqueda binaria.
- Recursividad para calcular el total de ingresos, el total de egresos y el balance general de la bóveda.

---

## Posibles Mejoras Futuras

- Encriptar las contraseñas de Cajero/Administrador.
- Implementar una interfaz gráfica.
- Utilizar una base de datos en lugar de archivos de texto.
- Permitir múltiples jornadas o sucursales simultáneas.



