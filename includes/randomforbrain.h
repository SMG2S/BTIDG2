/* Fonctions utilisées pour faire de l'aléatoire dans la générationd de cerveau / de matrice-cerveau */
/*Nicolas HOCHART*/

#pragma once

#include <stdlib.h>

/*--------------------------
--- Décision "aléatoire" ---
--------------------------*/

float rand_0_1()
{
    /*Renvoie un nombre aléatoire entre 0 et 1. Permet de faire une décision aléatoire*/
    return (float) rand() / (float) RAND_MAX;
}
