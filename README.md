# Calcul d'un arbre des formes (Tree of Shapes)

- Méline Bour-Lang
- Morgane Ritter
- Nathan Roth

Ce projet calcule l'arbre des formes d'une image 2D en temps linéaire, et se base sur le travail de recherche *A quasi-linear algorithm to compute the tree of shapes of nD images* réalisé par T. Géraud, E. Carlinet, S. Crozet et L. Najman en 2013.

## Installation

Une dépendance est requise pour exécuter le programme : la librairie SFML, qui permet un affichage interactif de l'arbre des formes.
Pour l'installer :
```sudo apt install libsfml-dev```.

## Utilisation

- Compiler: `make` ou `make debug`
- Nettoyer: `make clean`
- Lancer: `./tos <filename.pgm> --display`

Les images de test se trouvent dans le repertoire `test/`.

Détail des options disponibles :

- `-n, --no-uninterpolation` : permet de voir l'image non désinterpolée : l'arbre des formes inclut ainsi tous les pixels et *interpixels* ajoutés pour traiter l'image.
- `-f, --file` : permet d'indiquer le fichier d'entrée (il est possible d'indiquer le fichier sans l'option)
- `-d, --display` : affiche l'interface graphique. Il peut être intéressant de la désactiver pour faire des tests de performance.
- `-h, --help` : détail des options.
- `-v, --verbose`
- `-V, --version`

## Interface

Une fois que l'exécution termine, si l'option `--display` a été indiquée, l'interface graphique s'affiche. Pour interagir avec celle-ci :

- Cliquer-glisser : déplacement dans l'interface
- Molette de la souris : zoomer, dézoomer

Au passage de la souris sur un pixel, un tracé va s'afficher. Les pixels encadrés correspondent aux parents du pixel sous la souris, selon l'arbre des formes.
Ils sont reliés entre eux afin de définir un chemin de parenté.

Les couleurs permettent de définir le type des pixels. Cette distinction est utile dans le cas où l'on indique l'option `--no-unterpolation`. Voici la correspondance entre les couleurs et les types de pixels : si le pixel est encadré en
- rouge : il s'agit d'un pixel appartenant à l'image originale.
- vert : il s'agit d'un pixel créé lors de l'interpolation.
- bleu : il s'agit d'un *interpixel*, créé lors de l'interpolation également. 
