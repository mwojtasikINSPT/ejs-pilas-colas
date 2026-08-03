/*
Aplicacion en C que simula la gestion de un deposito de mercaderia (Pila de Cajas).
Cada caja almacena:
    Codigo de la caja.
    Contenido.
    Peso en kg.

La Pila se implementa con memoria dinamica y respeta estrictamente el orden LIFO (Last In, First Out).
Se incluye una funcion para extraer una caja especifica utilizando una Pila Auxiliar sin romper el orden.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ==========================================
// ESTRUCTURA DE DATOS
// ==========================================

typedef struct Caja
{
    int codigoCaja;
    char contenido[50];
    float pesoKg;
} Caja;

typedef struct Nodo
{
    Caja dato;
    struct Nodo *siguiente;
} Nodo;

// Prototipos de funciones de la Pila
Nodo *crearNodoCaja(Caja c);
int estaVaciaPila(Nodo *tope);
void apilar(Nodo **tope, Caja c);
Caja desapilar(Nodo **tope, int *exito);
void vaciarPila(Nodo **tope);
int extraerCajaEspecifica(Nodo **topePrincipal, int codigoBuscado);

// Prototipos de funciones de interfaz / consola
void mostrarPila(Nodo *tope);
void mostrarCaja(Caja c);
int menuDeposito(void);
void limpiarPantalla(void);
void pausar(void);

// ==========================================
// FUNCION PRINCIPAL
// ==========================================

int main(void)
{
    Nodo *topePila = NULL;
    Caja c;
    int opcion, codigoBuscado, enumerador = 1;

    do
    {
        opcion = menuDeposito();

        switch (opcion)
        {
        case 1:
            limpiarPantalla();
            printf("--- Apilar nueva caja ---\n");

            c.codigoCaja = enumerador++;

            printf("Ingrese contenido de la caja: ");
            scanf(" %49[^\n]", c.contenido);

            printf("Ingrese peso en kg: ");
            scanf("%f", &c.pesoKg);

            apilar(&topePila, c);
            printf("\nCaja apilada correctamente con Codigo %d.\n", c.codigoCaja);
            pausar();
            break;

        case 2:
            limpiarPantalla();
            printf("--- Desapilar caja superior (Retirar de la cima) ---\n");

            int exito;
            c = desapilar(&topePila, &exito);
            if (exito)
            {
                printf("\nCaja retirada de la cima:\n");
                mostrarCaja(c);
            }
            else
            {
                printf("\n[!] Error: El deposito esta vacio. No hay cajas para retirar.\n");
            }
            pausar();
            break;

        case 3:
            mostrarPila(topePila);
            break;

        case 4:
            limpiarPantalla();
            printf("--- Extraer caja especifica por codigo ---\n");

            if (estaVaciaPila(topePila))
            {
                printf("\n[!] El deposito esta vacio.\n");
            }
            else
            {
                printf("Ingrese codigo de la caja a retirar: ");
                scanf("%d", &codigoBuscado);

                if (extraerCajaEspecifica(&topePila, codigoBuscado))
                {
                    printf("\nCaja con Codigo %d extraida exitosamente del sistema.\n", codigoBuscado);
                }
                else
                {
                    printf("\n[!] No se encontro ninguna caja con ese codigo en el deposito.\n");
                }
            }
            pausar();
            break;

        case 5:
            limpiarPantalla();
            printf("--- Vaciar deposito completo ---\n");
            vaciarPila(&topePila);
            printf("Deposito vaciado correctamente. Memoria liberada.\n");
            pausar();
            break;
        }
    } while (opcion != 6);

    vaciarPila(&topePila);
    printf("\nMemoria liberada correctamente. Hasta luego!\n");
    return 0;
}

// ==========================================
// LOGICA DE LA ESTRUCTURA (PILAS Y PUNTEROS)
// ==========================================

Nodo *crearNodoCaja(Caja c)
{
    // Se reserva memoria dinámicamente en el heap para alojar un nuevo nodo de tipo Caja
    Nodo *nuevoNodo = (Nodo *)malloc(sizeof(Nodo));

    if (nuevoNodo != NULL)
    {
        nuevoNodo->dato = c;
        // El nodo nuevo creado quedará en la cima, por lo que su campo 'siguiente' 
        // apuntará temporalmente a NULL hasta que se enlace con el tope anterior.
        nuevoNodo->siguiente = NULL;
    }

    return nuevoNodo;
}

int estaVaciaPila(Nodo *tope)
{
    // Una Pila está vacía si el puntero al tope apunta a NULL
    return tope == NULL;
}

void apilar(Nodo **tope, Caja c)
{
    Nodo *nuevoNodo = crearNodoCaja(c);

    if (nuevoNodo == NULL)
    {
        printf("Error critico: no hay memoria disponible.\n");
        return;
    }

    // Al ser una estructura LIFO, el nuevo elemento se coloca siempre en la cima (tope).
    // El 'siguiente' del nuevo nodo pasa a apuntar al que hasta ahora era el tope actual.
    nuevoNodo->siguiente = *tope;

    // Actualizamos el puntero 'tope' del main para que ahora apunte a este nuevo nodo.
    *tope = nuevoNodo;
}

Caja desapilar(Nodo **tope, int *exito)
{
    Caja cajaRetornada = {0};

    if (estaVaciaPila(*tope))
    {
        *exito = 0;
        return cajaRetornada;
    }

    // Guardamos la dirección del nodo que está en el tope para poder extraerlo y liberar su memoria luego.
    Nodo *nodoAuxiliar = *tope;
    cajaRetornada = nodoAuxiliar->dato;

    // El nuevo tope de la pila pasa a ser el elemento que estaba inmediatamente debajo
    *tope = (*tope)->siguiente;

    // Liberamos la memoria RAM ocupada por el nodo extraído
    free(nodoAuxiliar);
    *exito = 1;
    return cajaRetornada;
}

void vaciarPila(Nodo **tope)
{
    Nodo *nodoAuxiliar;

    // Recorremos desapilando nodo por nodo hasta que la pila quede completamente vacía
    while (!estaVaciaPila(*tope))
    {
        nodoAuxiliar = *tope;
        *tope = (*tope)->siguiente;
        free(nodoAuxiliar);
    }
}

int extraerCajaEspecifica(Nodo **topePrincipal, int codigoBuscado)
{
    Nodo *topeAuxiliar = NULL;
    int encontrada = 0;
    int exitoDesapilar;

    // Para buscar y extraer una caja que se encuentra debajo de otras sin romper el orden LIFO,
    // debemos retirar las cajas superiores una a una y guardarlas temporalmente en una Pila Auxiliar.
    while (!estaVaciaPila(*topePrincipal))
    {
        Caja cTemp = desapilar(topePrincipal, &exitoDesapilar);

        if (cTemp.codigoCaja == codigoBuscado)
        {
            encontrada = 1;
            // Encontramos la caja buscada. Salimos del bucle sin volver a apilarla (queda eliminada).
            break;
        }
        else
        {
            // Si no es la caja buscada, la guardamos temporalmente en la pila auxiliar
            apilar(&topeAuxiliar, cTemp);
        }
    }

    // Una vez que encontramos y descartamos la caja (o si recorrimos todo y no estaba),
    // debemos devolver todas las cajas resguardadas en la pila auxiliar de vuelta a la pila principal.
    // Esto restaura exactamente el orden original de las cajas restantes.
    while (!estaVaciaPila(topeAuxiliar))
    {
        Caja cTemp = desapilar(&topeAuxiliar, &exitoDesapilar);
        apilar(topePrincipal, cTemp);
    }

    return encontrada;
}

// ==========================================
// FUNCIONES DE INTERFAZ
// ==========================================

void mostrarCaja(Caja c)
{
    printf("Codigo de Caja : %d\n", c.codigoCaja);
    printf("Contenido      : %s\n", c.contenido);
    printf("Peso           : %.2f kg\n", c.pesoKg);
}

void mostrarPila(Nodo *tope)
{
    Nodo *actual = tope;

    limpiarPantalla();
    printf("=== DEPOSITO DE CAJAS (PILA - TOPE ARRIBA) ===\n\n");

    if (estaVaciaPila(tope))
    {
        printf("[ El deposito esta completamente vacio ]\n");
    }
    else
    {
        printf("TOPE DE LA PILA ->\n\n");

        while (actual != NULL)
        {
            mostrarCaja(actual->dato);
            printf("---------------------------------------------\n");
            actual = actual->siguiente;
        }

        printf("BASE DE LA PILA\n");
    }

    printf("\n=============================================\n");
    pausar();
}

int menuDeposito(void)
{
    int opcion;

    do
    {
        limpiarPantalla();
        printf("        LOGISTICA EXPRESS - DEPOSITO\n");
        printf("==========================================\n");
        printf("1. Apilar nueva caja\n");
        printf("2. Desapilar caja superior (Retirar cima)\n");
        printf("3. Mostrar estado del deposito\n");
        printf("4. Extraer caja especifica por codigo\n");
        printf("5. Vaciar deposito completo\n");
        printf("6. Salir del sistema\n");
        printf("==========================================\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", &opcion) != 1)
        {
            while (getchar() != '\n')
                ;
            opcion = 0;
        }
    } while (opcion < 1 || opcion > 6);

    return opcion;
}

void limpiarPantalla(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void)
{
    printf("\nPresione Enter para continuar...");
    while (getchar() != '\n')
        ;
    getchar();
}