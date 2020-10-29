# HMIN 323 Informatique Graphique

## Carte Combinatoires (G-Maps)

On s'intéresse à la modélisation des données graphiques par cartes combinatoires.

Ce modèle se généralise en définissant des *brins* (dart), associés entre eux par des involutions, ainsi que des contraintes sur celles ci par rapport au type de donnée qu'elle représente ("points", "arrêtes" et "faces").

On travail avec le modèle en 3 dimensions, on a donc la carte combinatoire

$$(D, \alpha_0, \alpha_1, \alpha_2)$$

### Implémentation en C++

Ci-joint une implémentation des GMaps en C++.
On définit les involutions par une table des valeurs contenant toutes les relations.

On implémente une fonction, qui pour un degré et un brin donné, nous donne le résultat de l'application de l'involution correspondante.

Le code ci joint permet de calculer des orbites, lier des brins et effectuer plusieurs opérations sur la carte combinatoire.

On possède ensuite une carte combinatoire associée à des données, ici vecteurs 3D, afin de générer de la géométrie.