/**************************************************************************
 *
 * Departamento de Informática de Sistema y Computadores (DISCA)
 * Universidad Politécnica de Valencia.
 *
 * Author: Sergio Sáez (ssaez@disca.upv.es)
 *
 * File: f_lanzamiento.c
 *
 * Description:
 *      Contiene la fase de lanzamiento de instrucciones del algoritmo de
 *      Tomasulo con especulación
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *************************************************************************/

#define f_lanzamiento_C

/***** Includes ***********************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "instrucciones.h"
#include "prediccion.h"
#include "presentacion.h"

/***************************************************************
 *
 * Func: fase_ISS
 *
 **************************************************************/

void fase_ISS ()
{
    /*************************************/
    /*  Variables locales                */
    /*************************************/

    int     s;
    marca_t b;

    /*************************************/
    /*  Cuerpo función                   */
    /*************************************/

    /* Decodificación */

#define I_OP    IF_ISS_2.IR.codop
#define I_S1    IF_ISS_2.IR.Rfuente1
#define I_S2    IF_ISS_2.IR.Rfuente2
#define I_D     IF_ISS_2.IR.Rdestino
#define I_INM   IF_ISS_2.IR.inmediato
#define I_PC    IF_ISS_2.PC
#define I_ORDEN IF_ISS_2.orden
#define I_EXC   IF_ISS_2.excepcion
#define I_PRED  IF_ISS_2.prediccion

    /*** VISUALIZACIÓN ****/
    PC_ISS= I_PC;
    /**********************/

    /*** Si no sale correctamente hay que parar */

    if (Control_1.Cancelar) {   /* Este ciclo está cancelado */
        /*** VISUALIZACIÓN ****/
        muestra_fase("X", I_ORDEN);
        /**********************/
        return;
    }
    else if (Control_2.Cancelar) {
        return;
    }
    else {
        /*** VISUALIZACIÓN ****/
        muestra_fase("I", I_ORDEN);
        /**********************/

        Control_1.Parar= SI;
    } /* endif */

    /*** Busca un hueco en la cola */

    if (RB_long < TAM_REORDER)
        b= RB_fin;
    else
        return;                 /* No hay huecos en el ROB */

    RB[b].excepcion= EXC_NONE;
    RB[b].prediccion= I_PRED;

    /*** Lanza la instruccion */

    switch (I_OP) {
    case OP_L_D:
        /*** Busca un hueco en el tampón de lectura */
        for (s= INICIO_TAMPON_LECT; s<= FIN_TAMPON_LECT; s++)
            if (!TL[s].ocupado) break;

        if (s > FIN_TAMPON_LECT) return ;
        /* No hay sitio en la estación de reserva */

        /*** Reserva la entrada del ROB */
        RB[b].ocupado= SI;
        RB[b].OP= I_OP;
        RB[b].dest= I_D;
        RB[b].estado= EX;

        /*** Reserva el tampón de lectura */
        TL[s].rob= b;
        TL[s].ocupado= SI;
        TL[s].OP= I_OP;

        /*** Cálculo de la dirección */
        if (Rint[I_S1].rob == MARCA_NULA)
            TL[s].direccion= I_INM + Rint[I_S1].valor;
        else if (RB[Rint[I_S1].rob].estado == WB)
            TL[s].direccion= I_INM + RB[Rint[I_S1].rob].valor.i;

        /*** Reserva del registro destino */
        Rfp[I_D].rob= b;

        /*** VISUALIZACION ***/
        TL[s].orden= I_ORDEN;
        TL[s].PC= I_PC;
        RB[b].orden= I_ORDEN;
        RB[b].PC= I_PC;

        break;

    case OP_S_D: // TODO test
        /*** Busca un hueco en el tampón de escritura */
        for (s = INICIO_TAMPON_ESCR; s <= FIN_TAMPON_ESCR; s++)
            if (!TL[s].ocupado) break;

        if (s > FIN_TAMPON_ESCR) return;

        /*** Reserva la entrada del ROB */
        RB[b].ocupado = SI;
        RB[b].OP = I_OP;
        RB[b].dest = s;
        RB[b].estado = WB;

        /*** Reserva el tampón de escritura */
        TE[s].rob = b;
        TE[s].ocupado = SI;
        TE[s].OP = I_OP;
        TE[s].confirm = NO;

        /*** Cálculo de la dirección */
        if (Rint[I_S1].rob == MARCA_NULA)
            TE[s].direccion = I_INM + Rint[I_S1].valor;
        else if (RB[Rint[I_S1].rob].estado == WB)
            TE[s].direccion = I_INM + RB[Rint[I_S1].rob].valor.i;

        /*** Operando 2 ***/
        if (Rfp[I_S2].rob == MARCA_NULA) {
            TE[s].Vk.f = Rfp[I_S2].valor;
            TE[s].Qk = MARCA_NULA;
        } else if (RB[Rint[I_S2].rob].estado == WB) {
            TE[s].Vk = RB[Rfp[I_S2].rob].valor;
            TE[s].Qk = MARCA_NULA;
        } else {
            RS[s].Qk = Rfp[I_S2].rob;
        }

        /*** VISUALIZACION ***/
        TE[s].orden= I_ORDEN;
        TE[s].PC= I_PC;
        RB[b].orden= I_ORDEN;
        RB[b].PC= I_PC;

        break;

    case OP_ADD_D:
    case OP_SUB_D:
        /*** Busca un hueco en la estación de reserva */

        for (s = INICIO_RS_SUMA_RESTA; s <= FIN_RS_SUMA_RESTA; s++)
            if (!RS[s].ocupado) break;

        if (s > FIN_RS_SUMA_RESTA) return;

        /*** Reserva la entrada del ROB */
        RB[b].ocupado = SI;
        RB[b].OP = I_OP;
        RB[b].dest = I_D;
        RB[b].estado = EX;

        /*** Reserva el operador virtual */
        RS[s].rob = b;
        RS[s].ocupado = SI;
        RS[s].OP = I_OP;

        /*** Operando 1 ***/
        if (Rfp[I_S1].rob == MARCA_NULA) {
            RS[s].Vj.f = Rfp[I_S1].valor;
            RS[s].Qj = MARCA_NULA;
        } else if (RB[Rfp[I_S1].rob].estado == WB) {
            RS[s].Vj = RB[Rfp[I_S1].rob].valor;
            RS[s].Qj = MARCA_NULA;
        } else {
            RS[s].Qj = Rfp[I_S1].rob;
        }

        /*** Operando 2 ***/
        if (Rfp[I_S2].rob == MARCA_NULA) {
            RS[s].Vk.f = Rfp[I_S2].valor;
            RS[s].Qk = MARCA_NULA;
        } else if (RB[Rfp[I_S2].rob].estado == WB) {
            RS[s].Vk = RB[Rfp[I_S2].rob].valor;
            RS[s].Qk = MARCA_NULA;
        } else {
            RS[s].Qk = Rfp[I_S2].rob;
        }

        /*** Reserva del registro destino */
        Rfp[I_D].rob = b;

        /*** VISUALIZACION ***/
        RS[s].orden= I_ORDEN;
        RS[s].PC= I_PC;
        RB[b].orden= I_ORDEN;
        RB[b].PC= I_PC;

        break;

    case OP_MUL_D:
    case OP_DIV_D:
        /*** Busca un hueco en la estación de reserva */
        for (s = INICIO_RS_MULT_DIV; s <= FIN_RS_MULT_DIV; s++)
            if (!RS[s].ocupado) break;

        if (s > FIN_RS_MULT_DIV) return;

        /*** Reserva la entrada del ROB */
        RB[b].ocupado = SI;
        RB[b].OP = I_OP;
        RB[b].dest = I_D;
        RB[b].estado = EX;

        /*** Reserva el operador virtual */
        RS[s].rob = b;
        RS[s].ocupado = SI;
        RS[s].OP = I_OP;

        /*** Operando 1 ***/
        if (Rfp[I_S1].rob = MARCA_NULA) {
            RS[s].Vj.f = Rfp[I_S1].valor;
            RS[s].Qj = MARCA_NULA;
        } else if (RB[Rfp[I_S1].rob].estado == WB) {
            RS[s].Vj = RB[Rfp[I_S1].rob].valor;
            RS[s].Qj = MARCA_NULA;
        } else {
            RS[s].Qj = Rfp[I_S1].rob;
        }

        /*** Operando 2 ***/
        if (Rfp[I_S2].rob = MARCA_NULA) {
            RS[s].Vk.f = Rfp[I_S2].valor;
            RS[s].Qk = MARCA_NULA;
        } else if (RB[Rfp[I_S2].rob].estado == WB) {
            RS[s].Vk = RB[Rfp[I_S2].rob].valor;
            RS[s].Qk = MARCA_NULA;
        } else {
            RS[s].Qk = Rfp[I_S2].rob;
        }

        /*** Reserva del registro destino */
        Rfp[I_D].rob = b;

        /*** VISUALIZACION ***/
        RS[s].orden= I_ORDEN;
        RS[s].PC= I_PC;
        RB[b].orden= I_ORDEN;
        RB[b].PC= I_PC;

        break;

    default:
        fprintf(stderr, "ERROR (%s:%d): Operacion no implementada\n", __FILE__, __LINE__);
        exit(1);
        break;
    } /* endswitch */

    /*** La instrucción se ha lanzado correctamente */

    Control_1.Parar= NO;
    RB_fin= (RB_fin + 1) % TAM_REORDER;
    RB_long++ ;

    return ;

} /* end fase_ISS */
