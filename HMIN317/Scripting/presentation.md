# HMIN317 Moteur de Jeux - Étude Documentaire

# Bytecode et Scripting : Pattern, implémentations et solutions

## Gameplay & Comportement

Lors du développement d'un jeu, nous avons besoin d'y intégrer des éléments narratifs, de gameplay.
Un jeu est analogue à un film ou une scène de théâtre, on possède une scène, des acteurs, et ceux si intéragissent pour former l'environnement dans lequel est plongé le joueur.

Une partie du développement y est donc consacrée, on pense souvent au Gameplay Programming, mais c'est un peu plus large que cela. 
On va parler plus généralement de Behavioral Programming.

[Behavioral Programming 🔗](http://www.wisdom.weizmann.ac.il/~bprogram/more.html)

Dans un jeu, il existe une énorme quantité d'acteurs : items, quêtes, monstres, PNJs, *Portes*...
Nous faison face à deux problèmes majeurs ici:

#### Rapidité et Facilité de Développement

>Au vu de la quantité énorme de comportements à développer, il est impératif d'être rapide à développer ceux ci.  
Les codebases de jeux sont aussi très complexes et très volumineuses, il est donc nécessaire de trouver un moyen de simplifier le développement.

#### Maintenabilité

>Qui parle d'un très grand nombre de comportements à produire, parle aussi de maintenabilité du code.
Les interactions entre comportements peuvent très vite dégénérées, une bonne gestion des ressources doit impérativement être faite.

Cependant, le développement de tels comportements, pour la majorité, n'est que très peu demandant en ressources, et on peut très facilement réduire la précision de ceux-cis.  
Nous avons donc les critères suivants :
 - Besoin d'un développement rapide
 - Besoin d'une certaine sûreté
 - Non besoin de puissance de calcul très élevée

Pour répondre à ces critères, il existe plusieurs solutions, nous allons ici nous intéresser au paradigme du **Scripting**, et au pattern **Bytecode**.

*Note : ces critères diffèrent selon les besoins, c'est pour cela qu'il n'existe pas de solution universelle au scripting, nous allons voir ça par la suite*

## Scripting

#### Définition
> En informatique, un script désigne un programme (ou un bout de programme) chargé d'exécuter une action pré-définie quand un utilisateur réalise une action. Il s'agit d'une suite de commandes simples et souvent peu structurées qui permettent l'automatisation de certaines tâches successives dans un ordre donné.

*Source : [Journal du net 🔗](https://www.journaldunet.fr/web-tech/dictionnaire-du-webmastering/1203599-script-definition/)*

Dans le cadre des jeux, l'idée est de penser un script comme un **Asset**, au même titre qu'un modèle 3D, une texture ou un son.

Ce paradigme nous permet d'avoir une bonne maintenabilité, les scripts étant des composants indépendants pouvant être ajotué ou enlevé à la volée.

### Langages de Scripting

Plusieurs langages de scripting existent, comme le *Bash*, le *Ruby*, ou le *Javascript*.

## Interpretation et Compilation

Il existe plusieurs manières d'implémenter le paradigme du scripting.

**Interpretation** Méthode simple à mettre en place, mais coûteuse.  
**Compilation** Apporte de bonnes performances, mais est très  difficile à mettre en place et réduit la productivité.

De nos jours, il n'existe que très peu de langages directement interpretés dans les jeux, car le coût en performance est très important.  
Souvent, ce sont des langages custom, comme par exemple pour un système de dialogue.

La compilation peut se présenter sous plusieurs formes :  
**Compilation Classique (Statique ou Dynamique)** Le code est compilé en amont  
**Compilation Just-in-time (JIT)** Le code est compilé en temps réel.

Le *JavaScript* par exemple, est compilé en temps réel si on utilise le moteur *V8* de *Google*.

## Bytecode

L'interpretation et la compilations possèdent tous deux avantages et inconvénients.  
L'interpretation est très lente là où la compilation est moins portable, plus complexe et plus lourde à mettre en place.

Pour obtenir le meilleur des deux mondes, il existe un Pattern : le **Bytecode**.

Le principe est de définir un jeu d'instructions, compiler un programme pour ce jeu d'instruction et l'interprêter.
Le programme compilé est appelé *bytecode*.

Ainsi, toutes les instructions sont simples, réduisant l'overhead de l'interpretation pour un très bon gain de performance, et nous obtenons la modularité nécessaire, ce qui permet par exemple de faire du *Live Scripting*.

#### Avantages du bytecode
- *Dense et contigü.* Optimisation de la mémoire
- *Linéaire.* Utilisation de la stack possible, on évite au maximum les memory jumps.
- *Rapide*. Très peu d'overhead, les instructions sont très simples.

#### Désavantages
- Nécessite énormément d'outils (Debugger, Linter, Profiler etc...)

## Quelques solutions existantes

### LUA
**Langage** Lua  
**Lien**  [lua 🔗](https://www.lua.org/)  

Très utilisé dans l'indsutrie, solution très facile à mettre en place avec beaucoup d'outils existants.

### Mono
**Langages** C#, F#, Java, Python, Javascript etc...  
**Lien** [mono-project 🔗](https://www.mono-project.com/)  

Utilisé par Unity utilisant le C#, très puissant, mais difficile à mettre en place et nécessite une license pour embarquer mono.

### V8 Javascript

**Langage** Javascript   
**Lien** [v8 🔗](https://v8.dev/)  

Très puissant, propose de la compilation JIT, mais possède une grosse codebase, et pas très bien documenté pour les néophytes.





