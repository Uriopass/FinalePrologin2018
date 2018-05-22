# FinalePrologin2018

[Sujet de la final](https://prologin.org/static/archives/2018/finale/sujet/penguins-in-black.pdf)

Voici mon code pour la finale 2018, où je suis arrivé en 1e place.

Un rendu est situé en haut du code, mais voici une version formatée pour le README.

# RENDU
## Introduction

Au debut, je ne savais vraiment pas vers quoi partir. J'ai vu deux directions possible.  
 * Explorer l'action space en se limitant, et en trouvant les coups/ combinaisons de coup interessant.  
 * Simuler un "centre de controle" qui assignerait des aliens a des penguins (non exclusif), puis partir
    de la et essayer d'etre efficace localement (attaquer le penguin adverse etc..)

La deuxieme option me semblait plus facile d'acces au debut, mais apres un peu de reflexion et de calculs je 
me suis vite rendu compte qu'il y avait beaucoup trop d'evenements subtil a gerer pour qu'une simple assignation
alien <-> penguin fonctionne sans faire de compromis qui empecherait la generalisation de l'algorithme.

Ainsi, je suis partit vers la premiere option.

## 1. Explication

L'idee est donc de limiter l'espace des actions possible (plusieurs milliards, selon mes calculs un peu foireux, probablement beaucoup plus).
Pour cela, il va falloir faire des compromis.

- On ne peut pas combiner trop d'actions de multiples agents (ici action = DEPLACER/POUSSER/GLISSER)
car la combinatoire nous dit que les possibilites augmentent exponentiellement.
- On ne peut pas regarder toutes les possibilites d'actions meme pour un seul agent.
- On ne peut pas prevoir plusieurs tour a l'avance

Mais meme si l'espace des actions etait reduit, il faut trouver un moyen de juger ces actions.

Ici, utiliser simplement la meme idee que dans une IA d'echec s'applique : une fonction de cout sur le plateau
A noter que j'ai eu vraiment tres peur que cette fonction soit impossible a designer correctement,
et que j'ai eu l'impression de prendre un enorme risque en partant la dedans, car il etait tres difficile
de prevoir en amont si cette methode aller fonctionner correctement.

La boucle principale de l'algorithme ressemble donc a ca :
 - On regarde tout les coups _interessants_ possibles pour chaque agent
 - On juge chaque coup en l'appliquant au plateau puis en jugeant la qualite du plateau
 - On selectionne le meilleur coup, et on recommence (sauf si le meilleur coup etait de ne rien faire, le plateau est alors localement optimal)

La magie de cette methode est que si la fonction de cout est suffisamment generique, et si les coups sont suffisamment interessant,
alors cet algorithme devrait s'approcher de l'optimalite. 

Voici maintenant les details concernant la generation de coups et la fonction de cout.

## 2. La generation de coup

La generation des coups a beaucoup evolue pendant la duree du concours.
Tout d'abord, il n'y avait que les coups amenant a des aliens ou sur le cote d'un alien.
Puis, il y a eu les coups afin de pousser d'autres agents
Enfin, les coups "un peu nul", si il n'y avait pas grand chose a faire de toute facon.
A noter qu'il faut faire tres attention a ne pas generer trop de coups, 
car la fonction de cout est relativement lente (1/2 ms par evaluation).

Repris de la documentation de la fonction generant les coups :
La liste de coups potentiels est :
 - ne rien faire
 - deplacement vers une case avec un alien dessus (ou qui va apparaitre bientot).
 - deplacement vers les bords d'une case avec un alien ET un agent QUELCONQUE dessus 
 - deplacement vers les bords d'une case avec un alien ET un agent ENNEMI dessus, suivie d'une action POUSSER.
 - deplacement vers les bords d'une case avec un agent ENNEMI dessus seulement, suvie d'une action POUSSER.

## 3. La fonction de cout

Probablement la partie la plus interessante pour quiconque s'interesse a ce genre d'IA.

L'idee est de mettre des scores a different comportements afin d'encourager les agents a se mettre
dans de bonnes situations

A noter que quand je parle du "poids" d'un alien, cela correspond au points qu'il va rapporter.

La fonction de cout est constitue de 4 composantes :

Le cout principal: le score de "capture"  
 L'idee est d'encourager les agents a capturer des aliens. (Duh.)  
 Pour cela on va compter le nombre de points potentiel rapportable a ce tour.  
 Ainsi quand un allie est sur un alien, il va rapporter les points de l'alien au score de capture.  
 En l'occurence, score += poids_alien * (1 + progression_capture^2)  
 Ainsi un alien en cours de capture vaudra beaucoup plus qu'un agent qui vient de se mettre sur un alien.  
 Puis une modification est appliquee (tres importante):  
   Si l'alien a capture n'est pas en securite (si il peut se faire "kick"), alors il perd le gros de sa valeur (pourcentage indique par coeff_kick).  
    
  Verifier si l'alien est en securite est simple, on regarde si il est dans un coin
  (qui peut tres bien etre forme d'agents allies) et si il n'est pas dans un coin,
  qu'aucun agent ennemi (qui n'est pas en train de capturer un alien d'une valeur plus elevee)
  ne peux le pousser avant qu'il finisse sa capture.

Le score de distance :  
 L'idee est d'encourager les agents a se rapprocher des aliens interessant.  
 Pour cela, on va regarder pour chaque alien l'agent allie/enemi qui n'est pas en train de capturer un autre 
 alien le plus proche.
 Puis on va faire la difference de la distance et multiplier par le poids de l'alien.

Le score de presence :  
 L'idee est d'avoir une "presence" sur la map, au sens que les penguins sont tous relativement proche d'aliens dans un futur proche/actuel.  
 A la base, cela correspondait simplement au nombre de cases atteignables avant l'enemi (diagramme de voronoi), 
 mais cela etait une metrique totalement inutile.
 Pour cela, on va ponderer la distance a tout les aliens par leurs poids et leur distance dans le temps.  
 score presence -= dist * poids / dist_temps

Le score d'oracle :    
 L'idee est de pousser les agents a se mettre aux emplacements PRECIS ou un alien va apparaitre (presence pondere simplement)
 Pour cela on va simplement regarder si un alien va bientot apparaitre sur les cases des agents (pondere par leurs poids evidemment).
 score oracle += poids * va_apparaitre

Tout ces scores sont ensuite ponderes par leurs importances, grace a differents coeff.
Ces coeffs ont ete pas mal modifies, notamment quand certains des scores etaient ameliores (devenu plus pertinents).


## 4. Algorithmes employes
 Aucun algorithme avance n'a ete employe, on peut noter un djikstra optimise pour les perfs au prix de la memoire.
 (Pour plus de details, voir la fonction dist_to_cases plus bas)  
 Si la fonction de cout avait ete plus rapide, j'avais pense a faire un minimax avec alpha beta, 
 mais en realite une telle profondeur n'aurait jamais pu etre atteinte

## 5. Strategies employees
 L'explication est je pense, suffisamment detailles en terme de strategie (fonction de cout).  

## 6. Idees n'ayant pas aboutis
 Une tres grande partie de mon temps a ete perdue sur la tentative de predire l'adversaire.  
 L'idee etait pourtant simple, apres avoir applique mon coup on appliquait des coups de l'adversaire qui n'avait
 pas l'air trop bete non plus (me pousser, par exemple).  
 Malheureusement, cela s'est avere etre l'enfer techniquement, et a cause un tel nombre de bugs
 impossibles a trouver que j'ai finalement abandonne cet idee.

 A part ca, la plupars de mes idees se sont finalement averees utile, apres quelques tweak.
 Excepte pour le score de presence base sur le nombre de cases atteinte, qui ne servait a rien.
 
## 7. Si j'avais eu plus de temps
 En realite il reste tellement de choses a faire.

 Il y a probablement encore beaucoup de bugs un peu cache, j'ai passe je pense 50% du temps a trouver les bugs tres bien cache.
 qu'a reellement coder et ameliorer ma solution.  
 
 Optimiser la fonction de cout.
 Actuellement, je recalcule 8 djisktra a chaque evaluation alors qu'il n'y a que tres peu de modifications
 faites entre chaque cout. Il y a probablement moyen de gagner un facteur d'au moins 10.  

 Trouver de meilleurs coefficient
 Les coefficients ont ete pris un peu au pif en trouvant des valeurs qui correspondait dans l'ordre de grandeur.
 Mais de meilleurs coefficients pourrait probablement rendre l'algo plus generique et plus fort.  

 S'interesser a des combinaisons de coup interessant.
 Pour l'instant, l'algorithme ne considere que les coups directement effectue par 1 seul agent. 
 Par exemple, quand un agent capture un alien il pourrait tres bien utiliser ses PA pour aider le deplacement d'un autre agent,
 mais cela n'est pas pris en compte.  

 Trouver d'autres score pour la fonction de cout.
 Il y a encore beaucoup d'idee manquante pour ameliorer cette fonction de cout, et le jeu d'echec (et sa communeaute
 d'IA autour) montre bien qu'une IA n'est jamais parfaite meme quand elle parait jouer de maniere optimale.  

## 8. Details technique
### positions
 Dans mon cas, les positions sont encodes en tant qu'entier entre 0 et 625 plutot qu'un couple ligne colonne,
 car cela est beaucoup plus facile a gerer, et idem pour iterer sur toutes les possibilitees.

### plateau
 un plateau est simplement un tableau d'entier pouvant avoir comme valeur  
 0 -> libre  
 1 -> MUR  
 3 -> agent moi  
 4 -> agent enemi  

### gamestate
 un gamestate contient toutes les informations necessaire pour faire evoluer un plateau  
 l'idee etait de l'utiliser dans un systeme similaire a un algorithme minimax. Mais manque de temps pour cela.
 il a donc un plateau, un plateau pour les points de cap, les coordonnes des agents et le tour actuel.

### aliens
 les aliens sont contenues dans 2 structures de donnees  
 aliens_list pour iterer sur ceux ci  
 aliens_pos permettant de faire le lien position -> alien car celui ci est unique (un alien par position)  

### move
 un move permet de mettre a jour un gamestate et contient 4 informations  
  start -> position de depart  
  pos -> position d'arrivee  
  cout -> prix en PA d'effectuer ce coup  
  flags -> peut contenir des flags et un payload   
    par exemple pour encoder le fait de pousser on va mettre le flag POUSSER et on va ajouter la position
    de l'agent a pousser dans les 10 premiers bits
