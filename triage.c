/*
Aplicacion en C que simula el sistema de Triage de la guardia de un hospital.
La cola se implementa con memoria dinamica y respeta una Cola de Prioridad:
1: Rojo (Prioridad Maxima - Riesgo de vida)
2: Amarillo (Prioridad Media - Urgencia)
3: Verde (Prioridad Baja - Leve)

A igual prioridad, se respeta el orden de llegada (FIFO).
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// ==========================================
// ESTRUCTURA DE DATOS
// ==========================================

typedef struct Paciente
{
    int idPaciente;
    char nombreApellido[50];
    int nivelGravedad;
    char motivoConsulta[100];
} Paciente;

typedef struct Nodo
{
    Paciente dato;
    struct Nodo *siguiente;
} Nodo;

// Prototipos de funciones de la estructura (Punteros)
Nodo *crearNodo(Paciente p);
int estaVacia(Nodo *frente);
void acolarPaciente(Nodo **frente, Nodo **final, Paciente p);
Paciente atenderSiguiente(Nodo **frente, Nodo **final, int *exito);
int abandonarGuardia(Nodo **frente, Nodo **final, int idBuscado);
void vaciarGuardia(Nodo **frente, Nodo **final);

// Prototipos de funciones de interfaz / consola
void mostrarSalaDeEspera(Nodo *frente);
void mostrarPaciente(Paciente p);
int menuHospital(void);
void limpiarPantalla(void);
void pausar(void);
const char* obtenerGravedadStr(int nivel);

// ==========================================
// FUNCION PRINCIPAL
// ==========================================

int main(void)
{
    Nodo *frenteCola = NULL;
    Nodo *finalCola = NULL;
    Paciente p;
    int opcion, idBuscado, exito, enumerador = 1;

    do
    {
        opcion = menuHospital();

        switch (opcion)
        {
        case 1:
            limpiarPantalla();
            printf("--- Ingresar nuevo paciente (Triage) ---\n");
            
            p.idPaciente = enumerador++;
            
            printf("Ingrese Nombre y Apellido: ");
            scanf(" %49[^\n]", p.nombreApellido);
            
            printf("Ingrese motivo de consulta: ");
            scanf(" %99[^\n]", p.motivoConsulta);
            
            do {
                printf("Nivel de gravedad (1=Rojo, 2=Amarillo, 3=Verde): ");
                scanf("%d", &p.nivelGravedad);
            } while(p.nivelGravedad < 1 || p.nivelGravedad > 3);

            // Pasamos las direcciones de los punteros porque la lista se modificará
            acolarPaciente(&frenteCola, &finalCola, p);
            
            printf("\nPaciente %s ingresado con ID %d.\n", p.nombreApellido, p.idPaciente);
            pausar();
            break;

        case 2:
            limpiarPantalla();
            printf("--- Llamar al siguiente paciente ---\n");
            
            p = atenderSiguiente(&frenteCola, &finalCola, &exito);
            if (exito)
            {
                printf("\n>>> LLAMANDO A CONSULTORIO <<<\n");
                mostrarPaciente(p);
            }
            else
            {
                printf("\n[!] La sala de espera esta vacia. No hay pacientes por atender.\n");
            }
            pausar();
            break;

        case 3:
            mostrarSalaDeEspera(frenteCola);
            break;

        case 4:
            limpiarPantalla();
            printf("--- Paciente abandona la guardia ---\n");
            
            if (estaVacia(frenteCola))
            {
                printf("\n[!] No hay pacientes en la sala de espera.\n");
            }
            else
            {
                printf("Ingrese ID del paciente que se retira: ");
                scanf("%d", &idBuscado);
                
                if (abandonarGuardia(&frenteCola, &finalCola, idBuscado)) {
                    printf("\nPaciente con ID %d fue retirado de la fila correctamente.\n", idBuscado);
                } else {
                    printf("\n[!] No se encontro ningun paciente con ese ID.\n");
                }
            }
            pausar();
            break;

        case 5:
            limpiarPantalla();
            printf("--- Vaciando sala de espera ---\n");
            vaciarGuardia(&frenteCola, &finalCola);
            printf("Todos los pacientes fueron retirados del sistema.\n");
            pausar();
            break;
        }
    } while (opcion != 6);

    vaciarGuardia(&frenteCola, &finalCola);
    printf("\nSistema cerrado. Memoria liberada correctamente.\n");
    return 0;
}

// ==========================================
// LOGICA DE LA ESTRUCTURA (PUNTEROS Y NODOS)
// ==========================================

Nodo *crearNodo(Paciente p)
{
    // Se reserva memoria dinámica en el heap para alojar un nuevo nodo
    Nodo *nuevoNodo = (Nodo *)malloc(sizeof(Nodo));
    if (nuevoNodo != NULL)
    {
        nuevoNodo->dato = p;
        nuevoNodo->siguiente = NULL; 
    }
    return nuevoNodo;
}

int estaVacia(Nodo *frente)
{
    return frente == NULL;
}

void acolarPaciente(Nodo **frente, Nodo **final, Paciente p)
{
    Nodo *nuevoNodo = crearNodo(p);

    if (nuevoNodo == NULL)
    {
        printf("Error critico: no hay memoria disponible.\n");
        return;
    }

    // CASO 1: La cola está vacía
    if (estaVacia(*frente))
    {
        *frente = nuevoNodo;
        *final = nuevoNodo;
    }
    // CASO 2: El nuevo nodo tiene MAYOR prioridad (número menor) que el primero de la fila.
    // Ej: El frente actual es 2 (Amarillo) y llega un 1 (Rojo).
    else if (p.nivelGravedad < (*frente)->dato.nivelGravedad)
    {
        nuevoNodo->siguiente = *frente;
        *frente = nuevoNodo;
    }
    // CASO 3: El paciente va al medio o al final de la cola (debe respetar FIFO a igual gravedad)
    else
    {
        Nodo *actual = *frente;
        Nodo *anterior = NULL;

        // Recorremos mientras haya nodos Y la gravedad del nodo actual sea menor o igual (<=) a la del nuevo.
        // El <= garantiza que si entra un 2, pasará a todos los 1 y a los 2 que llegaron antes, ubicándose al final de los 2.
        while (actual != NULL && actual->dato.nivelGravedad <= p.nivelGravedad)
        {
            anterior = actual;
            actual = actual->siguiente;
        }

        // Insertamos el nuevo nodo entre 'anterior' y 'actual'
        anterior->siguiente = nuevoNodo;
        nuevoNodo->siguiente = actual;

        // Control crítico: si 'actual' es NULL, significa que recorrimos toda la lista y lo insertamos
        // en la última posición absoluta. Debemos actualizar el puntero 'final' del main.
        if (actual == NULL)
        {
            *final = nuevoNodo;
        }
    }
}

Paciente atenderSiguiente(Nodo **frente, Nodo **final, int *exito)
{
    Paciente pacienteRetornado = {0};

    if (estaVacia(*frente))
    {
        *exito = 0;
        return pacienteRetornado;
    }

    // Guardamos la dirección del nodo del frente para poder hacer free() luego sin perder la cadena
    Nodo *nodoAuxiliar = *frente;
    pacienteRetornado = nodoAuxiliar->dato;

    // Desplazamos el frente al segundo elemento (desacolar clásico)
    *frente = (*frente)->siguiente;

    // Si la cola quedó vacía tras retirar al paciente, 'final' debe apuntar a NULL
    if (*frente == NULL)
    {
        *final = NULL;
    }

    free(nodoAuxiliar);
    *exito = 1;
    return pacienteRetornado;
}

int abandonarGuardia(Nodo **frente, Nodo **final, int idBuscado)
{
    Nodo *actual = *frente;
    Nodo *anterior = NULL; 

    while (actual != NULL)
    {
        if (actual->dato.idPaciente == idBuscado)
        {
            Nodo *aEliminar = actual;

            // Si el paciente a eliminar es el primero de la fila
            if (anterior == NULL)
            {
                *frente = actual->siguiente;
            }
            else
            {
                // Puenteamos el nodo a eliminar
                anterior->siguiente = actual->siguiente;
            }

            // Si estamos eliminando el ÚLTIMO nodo de la cola, debemos actualizar el puntero final
            if (actual->siguiente == NULL)
            {
                *final = anterior;
            }

            free(aEliminar);
            return 1;
        }
        
        anterior = actual;
        actual = actual->siguiente;
    }

    return 0;
}

void vaciarGuardia(Nodo **frente, Nodo **final)
{
    Nodo *nodoAuxiliar;

    while (!estaVacia(*frente))
    {
        nodoAuxiliar = *frente;
        *frente = (*frente)->siguiente;
        free(nodoAuxiliar);
    }

    *final = NULL;
}

// ==========================================
// FUNCIONES DE INTERFAZ
// ==========================================

const char* obtenerGravedadStr(int nivel) {
    switch(nivel) {
        case 1: return "ROJO (Riesgo de Vida)";
        case 2: return "AMARILLO (Urgencia)";
        case 3: return "VERDE (Leve)";
        default: return "DESCONOCIDO";
    }
}

void mostrarPaciente(Paciente p)
{
    printf("ID Paciente : %d\n", p.idPaciente);
    printf("Nombre      : %s\n", p.nombreApellido);
    printf("Gravedad    : Nivel %d - %s\n", p.nivelGravedad, obtenerGravedadStr(p.nivelGravedad));
    printf("Motivo      : %s\n", p.motivoConsulta);
}

void mostrarSalaDeEspera(Nodo *frente)
{
    Nodo *actual = frente;

    limpiarPantalla();
    printf("=== SALA DE ESPERA (ORDEN DE ATENCION) ===\n\n");

    if (estaVacia(frente))
    {
        printf("[ No hay pacientes en espera ]\n");
    }
    else
    {
        printf("SIGUIENTE EN SER LLAMADO ->\n\n");

        while (actual != NULL)
        {
            mostrarPaciente(actual->dato);
            printf("----------------------------------------\n");
            actual = actual->siguiente;
        }

        printf("FIN DE LA FILA\n");
    }

    printf("\n==========================================\n");
    pausar();
}

int menuHospital(void)
{
    int opcion;

    do
    {
        limpiarPantalla();
        printf("    HOSPITAL CENTRAL - SISTEMA DE TRIAGE\n");
        printf("==========================================\n");
        printf("1. Ingresar nuevo paciente (Triage)\n");
        printf("2. Llamar al siguiente paciente\n");
        printf("3. Mostrar sala de espera actual\n");
        printf("4. Paciente abandona guardia sin ser atendido\n");
        printf("5. Vaciar sala de espera\n");
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