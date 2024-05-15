
## Options de compilation faust utiles :

-fm <file>  --fast-math <file>          use optimized versions of mathematical functions implemented in <file>, use 'faust/dsp/fastmath.cpp' when file is 'def', assume functions are defined in the architecture file when file is 'arch'.

 -e        --export-dsp                  export expanded DSP (with all included libraries).
 
## Ce que j'ai pour l'instant :

En faisant :

 faust -a faust2circle.cpp X.dsp -e -o expended.dsp
 faust -a faust2circle expended.dsp -fm 'def' -o circle.cpp

On obtient la nouvelle version de circle qui est ici

Je sais que si on a faust installé sous Linux, on peut trouver fastmath.cpp ici :

    /usr/local/include/faust/dsp/fastmath.cpp

Dans le dsp expended on a accès à toutes les librairies faust nécessaires c'est les :

    library_path0, library_path1, ...

Il reste juste à les ajouter dans le makefile de circle.
Ce serait cool d'avoir juste à les copier depuis le expended.dsp dans un autre fichier et le makefile va les chercher dedans mais jsp comment on ferait ça.

On a encore le problème que malgré ça, on à besoin du math.h standard et je sais pas ou le trouver, j'en ai un sur internet mais aucune idée de si il est bon, je le mets ici à tester avec circle.
