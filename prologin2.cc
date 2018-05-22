/**
 *  === RENDU ===
 * Introduction
 * ------------
 * 
 * Au debut, je ne savais vraiment pas vers quoi partir. J'ai vu deux directions possible.
 *  -> Explorer l'action space en se limitant, et en trouvant les coups/ combinaisons de coup interessant.
 *  -> Simuler un "centre de controle" qui assignerait des aliens a des penguins (non exclusif), puis partir
 *     de la et essayer d'etre efficace localement (attaquer le penguin adverse etc..)
 * 
 * La deuxieme option me semblait plus facile d'acces au debut, mais apres un peu de reflexion et de calculs je 
 * me suis vite rendu compte qu'il y avait beaucoup trop d'evenements subtil a gerer pour qu'une simple assignation
 * alien <-> penguin fonctionne sans faire de compromis qui empecherait la generalisation de l'algorithme.
 * 
 * Ainsi, je suis partit vers la premiere option.
 * 
 * ** 1. Explication **
 * ---------------------------
 * 
 * L'idee est donc de limiter l'espace des actions possible (plusieurs milliards, selon mes calculs un peu foireux, probablement beaucoup plus).
 * Pour cela, il va falloir faire des compromis.
 * 
 * - On ne peut pas combiner trop d'actions de multiples agents (ici action = DEPLACER/POUSSER/GLISSER)
 * car la combinatoire nous dit que les possibilites augmentent exponentiellement.
 * - On ne peut pas regarder toutes les possibilites d'actions meme pour un seul agent.
 * - On ne peut pas prevoir plusieurs tour a l'avance
 * 
 * Mais meme si l'espace des actions etait reduit, il faut trouver un moyen de juger ces actions.
 * 
 * Ici, utiliser simplement la meme idee que dans une IA d'echec s'applique : une fonction de cout sur le plateau
 * A noter que j'ai eu vraiment tres peur que cette fonction soit impossible a designer correctement,
 * et que j'ai eu l'impression de prendre un enorme risque en partant la dedans, car il etait tres difficile
 * de prevoir en amont si cette methode aller fonctionner correctement.
 * 
 * La boucle principale de l'algorithme ressemble donc a ca :
 *  - On regarde tout les coups _interessants_ possibles pour chaque agent
 *  - On juge chaque coup en l'appliquant au plateau puis en jugeant la qualite du plateau
 *  - On selectionne le meilleur coup, et on recommence (sauf si le meilleur coup etait de ne rien faire, le plateau est alors localement optimal)
 *
 * La magie de cette methode est que si la fonction de cout est suffisamment generique, et si les coups sont suffisamment interessant,
 * alors cet algorithme devrait s'approcher de l'optimalite. 
 * 
 * Voici maintenant les details concernant la generation de coups et la fonction de cout.
 * 
 * ** 2. La generation de coup **
 * ------------------------------
 * 
 * La generation des coups a beaucoup evolue pendant la duree du concours.
 * Tout d'abord, il n'y avait que les coups amenant a des aliens ou sur le cote d'un alien.
 * Puis, il y a eu les coups afin de pousser d'autres agents
 * Enfin, les coups "un peu nul", si il n'y avait pas grand chose a faire de toute facon.
 * A noter qu'il faut faire tres attention a ne pas generer trop de coups, 
 * car la fonction de cout est relativement lente (1/2 ms par evaluation).
 * 
 * Repris de la documentation de la fonction generant les coups :
 * La liste de coups potentiels est :
 *  - ne rien faire
 *  - deplacement vers une case avec un alien dessus (ou qui va apparaitre bientot).
 *  - deplacement vers les bords d'une case avec un alien ET un agent QUELCONQUE dessus 
 *  - deplacement vers les bords d'une case avec un alien ET un agent ENNEMI dessus, suivie d'une action POUSSER.
 *  - deplacement vers les bords d'une case avec un agent ENNEMI dessus seulement, suvie d'une action POUSSER.
 * 
 * ** 3. La fonction de cout ** 
 * ----------------------------
 * 
 * Probablement la partie la plus interessante pour quiconque s'interesse a ce genre d'IA.
 * 
 * L'idee est de mettre des scores a different comportements afin d'encourager les agents a se mettre
 * dans de bonnes situations
 * 
 * A noter que quand je parle du "poids" d'un alien, cela correspond au points qu'il va rapporter.
 * 
 * La fonction de cout est constitue de 4 composantes :
 * 
 * Le cout principal: le score de "capture"
 *  L'idee est d'encourager les agents a capturer des aliens. (Duh.)
 *  Pour cela on va compter le nombre de points potentiel rapportable a ce tour.
 *  Ainsi quand un allie est sur un alien, il va rapporter les points de l'alien au score de capture.
 *  En l'occurence, score += poids_alien * (1 + progression_capture^2)
 *  Ainsi un alien en cours de capture vaudra beaucoup plus qu'un agent qui vient de se mettre sur un alien.
 *  Puis une modification est appliquee (tres importante):
 *   Si l'alien a capture n'est pas en securite (si il peut se faire "kick"), 
 *   alors il perd le gros de sa valeur (pourcentage indique par coeff_kick).
 *   
 *   Verifier si l'alien est en securite est simple, on regarde si il est dans un coin
 *   (qui peut tres bien etre forme d'agents allies) et si il n'est pas dans un coin,
 *   qu'aucun agent ennemi (qui n'est pas en train de capturer un alien d'une valeur plus elevee)
 *   ne peux le pousser avant qu'il finisse sa capture.
 * 
 * Le score de distance :
 *  L'idee est d'encourager les agents a se rapprocher des aliens interessant.
 *  Pour cela, on va regarder pour chaque alien l'agent allie/enemi qui n'est pas en train de capturer un autre 
 *  alien le plus proche.
 *  Puis on va faire la difference de la distance et multiplier par le poids de l'alien.
 * 
 * Le score de presence :
 *  L'idee est d'avoir une "presence" sur la map, au sens que les penguins sont tous relativement proche d'aliens dans un futur proche/actuel.
 *  A la base, cela correspondait simplement au nombre de cases atteignables avant l'enemi (diagramme de voronoi), 
 *  mais cela etait une metrique totalement inutile.
 *  Pour cela, on va ponderer la distance a tout les aliens par leurs poids et leur distance dans le temps.
 *  score presence -= dist * poids / dist_temps
 * 
 * Le score d'oracle :
 *  L'idee est de pousser les agents a se mettre aux emplacements PRECIS ou un alien va apparaitre (presence pondere simplement)
 *  Pour cela on va simplement regarder si un alien va bientot apparaitre sur les cases des agents (pondere par leurs poids evidemment).
 *  score oracle += poids * va_apparaitre
 * 
 * Tout ces scores sont ensuite ponderes par leurs importances, grace a differents coeff.
 * Ces coeffs ont ete pas mal modifies, notamment quand certains des scores etaient ameliores (devenu plus pertinents).
 * 
 * 
 * ** 4. Algorithmes employes **
 *  Aucun algorithme avance n'a ete employe, on peut noter un djikstra optimise pour les perfs au prix de la memoire.
 *  (Pour plus de details, voir la fonction dist_to_cases plus bas)
 *  Si la fonction de cout avait ete plus rapide, j'avais pense a faire un minimax avec alpha beta, 
 *  mais en realite une telle profondeur n'aurait jamais pu etre atteinte
 * 
 * ** 5. Strategies employees **
 *  L'explication est je pense, suffisamment detailles en terme de strategie (fonction de cout).
 * 
 * ** 6. Idees n'ayant pas aboutis **
 *  Une tres grande partie de mon temps a ete perdue sur la tentative de predire l'adversaire
 *  L'idee etait pourtant simple, apres avoir applique mon coup on appliquait des coups de l'adversaire qui n'avait
 *  pas l'air trop bete non plus (me pousser, par exemple).
 *  Malheureusement, cela s'est avere etre l'enfer techniquement, et a cause un tel nombre de bugs
 *  impossibles a trouver que j'ai finalement abandonne cet idee.
 * 
 *  A part ca, la plupars de mes idees se sont finalement averees utile, apres quelques tweak.
 *  Excepte pour le score de presence base sur le nombre de cases atteinte, qui ne servait a rien.
 *  
 * ** 7. Si j'avais eu plus de temps **
 *  En realite il reste tellement de choses a faire.
 * 
 *  Il y a probablement encore beaucoup de bugs un peu cache, j'ai passe je pense 50% du temps a trouver les bugs tres bien cache.
 *  qu'a reellement coder et ameliorer ma solution.
 *  
 *  Optimiser la fonction de cout.
 *  Actuellement, je recalcule 8 djisktra a chaque evaluation alors qu'il n'y a que tres peu de modifications
 *  faites entre chaque cout. Il y a probablement moyen de gagner un facteur d'au moins 10.
 * 
 *  Trouver de meilleurs coefficient
 *  Les coefficients ont ete pris un peu au pif en trouvant des valeurs qui correspondait dans l'ordre de grandeur.
 *  Mais de meilleurs coefficients pourrait probablement rendre l'algo plus generique et plus fort.
 * 
 *  S'interesser a des combinaisons de coup interessant.
 *  Pour l'instant, l'algorithme ne considere que les coups directement effectue par 1 seul agent. 
 *  Par exemple, quand un agent capture un alien il pourrait tres bien utiliser ses PA pour aider le deplacement d'un autre agent,
 *  mais cela n'est pas pris en compte.
 * 
 *  Trouver d'autres score pour la fonction de cout.
 *  Il y a encore beaucoup d'idee manquante pour ameliorer cette fonction de cout, et le jeu d'echec (et sa communeaute
 *  d'IA autour) montre bien qu'une IA n'est jamais parfaite meme quand elle parait jouer de maniere optimale.
 * 
 * ** 8. Details technique **
 * positions
 *  Dans mon cas, les positions sont encodes en tant qu'entier entre 0 et 625 plutot qu'un couple ligne colonne,
 *  car cela est beaucoup plus facile a gerer, et idem pour iterer sur toutes les possibilitees.
 * 
 * plateau
 *  un plateau est simplement un tableau d'entier pouvant avoir comme valeur
 *  0 -> libre
 *  1 -> MUR
 *  3 -> agent moi
 *  4 -> agent enemi
 * 
 * gamestate
 *  un gamestate contient toutes les informations necessaire pour faire evoluer un plateau
 *  l'idee etait de l'utiliser dans un systeme similaire a un algorithme minimax. Mais manque de temps pour cela.
 *  il a donc un plateau, un plateau pour les points de cap, les coordonnes des agents et le tour actuel.
 * 
 * aliens
 *  les aliens sont contenues dans 2 structures de donnees
 *  aliens_list pour iterer sur ceux ci
 *  aliens_pos permettant de faire le lien position -> alien car celui ci est unique (un alien par position)
 * 
 * move
 *  un move permet de mettre a jour un gamestate et contient 4 informations
 *   start -> position de depart
 *   pos -> position d'arrivee
 *   cout -> prix en PA d'effectuer ce coup
 *   flags -> peut contenir des flags et un payload 
 *     par exemple pour encoder le fait de pousser on va mettre le flag POUSSER et on va ajouter la position 
 *     de l'agent a pousser dans les 10 premiers bits
 * 
 */  



#include "prologin.hh"
#include <ctime>
#include <iostream>

#define AGENT_MOI 3
#define AGENT_LUI 4
// Flags pour move_t
#define POUSSER_FLAG 2048
#define NULLMOVE_FLAG 4096
#define CAP_1_FLAG 8192
#define CAP_2_FLAG 16384
#define CAP_3_FLAG 32768

// Flags pour le djisktra
#define GLISSER_FLAG 2048

typedef int plateau[625];
typedef struct {
	int pos;
	int start;
	int cout;
	int flags; // Les premiers bits du flag peuvent contenir de l'information, comme la direction a pousser
} move_t;

alien_info aliens_pos[625];
std::vector<alien_info> aliens_list;

typedef struct {
	plateau p;
	
	int agents_moi[4];
	int agents_lui[4];
	int agents_PA[4];
	
	plateau cap;
	int tour;
} gamestate;

direction directions[4] = {NORD, EST, SUD, OUEST};

void dist_to_cases(plateau& pla, int pos_start, plateau& result, plateau& dual_graph);

/// Fonctions pratiques

int from_position(position& pos) {
	return pos.ligne * 25 + pos.colonne;
}

int abs(int x) {
	return x < 0 ? -x : x;
}

float fabs(float x) {
	return x < 0 ? -x : x;
}

int l1_dist(int pos, int pos2) {
	return abs(pos%25 - pos2%25) + abs(pos/25-pos2/25);
}

bool est_capturable(alien_info& alien, int tour, int cap) {
	return cap < NB_TOURS_CAPTURE && (alien.tour_invasion <= tour && alien.duree_invasion + alien.tour_invasion + cap - 3 >= tour);
}

void cpy_plateau(plateau& src, plateau& dst) {
	for(int i = 0 ; i < 625 ; i++) {
		dst[i] = src[i];
	}
}

bool cmp_plateau(plateau& p1, plateau& p2) {
	for(int i = 0 ; i < 625 ; i++) {
		if(p1[i] != p2[i])
			return false;
	}
	return true;
}

direction diff_to_direction(int diff) {
	if(diff > 0) {
		if(diff < 25)
			return EST;
		return SUD;
	} else {
		if(diff > -25) {
			return OUEST;
		}
		return NORD;
	}
}

// permet de savoir si une position est dans un coin
// par exemple 
//                        x
//  ox renvoie true mais  o renvoie false
//  x                     x
//
bool est_safe(gamestate& g, int pos, int friends) {
	int c = pos%25;
	int l = pos/25;
	int v = 0;
	int h = 0;
	// positions adjaceantes
	if(c != 0) {
		h += g.p[pos-1] == MUR || g.p[pos-1] == friends;
	}
	if(c != 24) {
		h += g.p[pos+1] == MUR || g.p[pos+1] == friends ;
	}
	if(l != 0) {
		v += g.p[pos-25] == MUR || g.p[pos-25] == friends;
	}
	if(l != 24) {
		v += g.p[pos+25] == MUR || g.p[pos+25] == friends;
	}
	return h > 0 && v > 0;
}

// permet de connaitre l'emplacement d'arrivee lorsqu'on on pousse un objet depuis pos vers la direction dir
int pousser_res(plateau& p, int pos, direction dir) {
	int c = pos % 25;
	int l = pos / 25;
	if(dir == EST) {
		int dec = 1;
		while(c+dec < 25 && p[pos+dec] <= 0) {
			dec += 1;
		}
		dec -= 1;
		return pos+dec;
	}
	if(dir == OUEST) {
		int dec = 1;
		while(c-dec >= 0&& p[pos-dec] <= 0) {
			dec += 1;
		}
		dec -= 1;
		return pos-dec;
	}
	if(dir == NORD) {
		int dec = 1;
		while(l-dec >= 0 && p[pos-25*dec] <= 0) {
			dec += 1;
		}
		dec -= 1;
		return pos-25*dec;
	}
	if(dir == SUD) {
		int dec = 1;
		while(l+dec < 25 && p[pos+25*dec] <= 0) {
			dec += 1;
		}
		dec -= 1;
		return pos+25*dec;
	}
	return -156;
}

std::vector<int> gen_sides(plateau& p, int pos) {
	std::vector<int> mv = std::vector<int>();
	int c = pos%25;
	int l = pos/25;
	
	// positions adjaceantes
	if(c != 0) {
		mv.push_back(pos-1);
	}
	if(c != 24) {
		mv.push_back(pos+1);
	}
	if(l != 0) {
		mv.push_back(pos-25);
	}
	if(l != 24) {
		mv.push_back(pos+25);
	}
	return mv;
}

void gen_mouvements(plateau& p, int pos, int* moves) {
	std::vector<int> mv = std::vector<int>();
	int c = pos%25;
	int l = pos/25;
	int ptr = 0;
	
	// positions adjaceantes
	if(c != 0) {
		moves[ptr++] = pos-1;
	}
	if(c != 24) {
		moves[ptr++] = pos+1;
	}
	if(l != 0) {
		moves[ptr++] = pos-25;
	}
	if(l != 24) {
		moves[ptr++] = pos+25;
	}
	//glissade a droite
	int dec = 1;
	while(c+dec < 25 && p[pos+dec] <= 0) {
		dec += 1;
	} 
	dec -= 1;
	if(dec >= 3) {
		moves[ptr++] = (pos+dec) | GLISSER_FLAG;
	}
	
	//glissade a gauche
	dec = 1;
	while(c-dec >= 0 && p[pos-dec] <= 0) {
		dec += 1;
	} 
	dec -= 1;
	if(dec >= 3) {
		moves[ptr++] = (pos-dec) | GLISSER_FLAG;
	}
	
	//glissade en haut
	dec = 1;
	while(l-dec >= 0 && p[pos-25*dec] <= 0) {
		dec += 1;
	} 
	dec -= 1;
	if(dec >= 3) {
		moves[ptr++] = (pos-25*dec) | GLISSER_FLAG;
	}
	
	//glissade en bas
	dec = 1;
	while(l+dec < 25 && p[pos+25*dec] <= 0) {
		dec += 1;
	} 
	dec -= 1;
	if(dec >= 3) {
		moves[ptr++] = (pos+25*dec) | GLISSER_FLAG;
	}
	moves[ptr] = -1;
}

/// Fonctions d'affichage

void afficher_coup(move_t mv) {
	printf("%d-%d %d-%d %d %d\n", mv.start%25, mv.start/25, mv.pos%25, mv.pos/25, mv.cout, mv.flags);
}

void afficher_raw(plateau& p) {
	for(int y = 0 ; y < 25 ; y++) {
		for(int x = 0 ; x < 25 ; x++) {
			int v = p[y*25+x];
			if(v < 10)
				printf("%d   ", v);
			else if(v < 100)
				printf("%d  ", v);
			else if(v < 1000)
				printf("%d ", v);
			else
				printf("%d..", v/100);
		}
		printf("\n");
	}
}

void afficher_plateau(plateau& p, bool afficher_aliens, int tour) {
	for(int y = 0 ; y < 25 ; y++) {
		for(int x = 0 ; x < 25 ; x++) {
			char c;
			int v = p[y*25+x];
			switch(v) {
				case LIBRE:
					c = '.';
					break;
				case MUR:
					c = '*';
					break;
				case AGENT_LUI:
					c = 'E';
					break;
				case AGENT_MOI:
					c = 'M';
					break;
				default:
					c = ' ';
					break;
			}
			if(c == ' ') {
				printf("ERREUR: Carte corrompue\n");
			}
			if(afficher_aliens) {
				alien_info alien = aliens_pos[y*25+x];
				if(est_capturable(alien, tour, alien.capture_en_cours)) {
					if(c == '.') {
						c = 'a';
					}
				}
			}
			printf("%c", c);
		}
		printf("\n");
	}
}

void afficher_state(gamestate& state) {
	afficher_plateau(state.p, true, state.tour);
	for(int i = 0 ; i < 4 ; i++) {
		printf("%d ", state.agents_moi[i]);
	}
	printf("\n");
	for(int i = 0 ; i < 4 ; i++) {
		printf("%d ", state.agents_lui[i]);
	}
	printf("\n");
	for(int i = 0 ; i < 4 ; i++) {
		printf("%d ", state.agents_PA[i]);
	}
	printf("\n");
	for(int i = 0 ; i < 625 ; i++) {
		if(state.cap[i] != 0) {
			printf("cap %d -> %d\n", i, state.cap[i]);
		}
	}
	printf("%d\n", state.tour);
}

/**
 * Permet d'appliquer un coup "move_t" a un etat. Cela etait initialiement prevu dans l'optique
 * d'appliquer plusieurs coup puis de les annuler, mais au final il fut impossible d'aller plus loin
 * que 1 de profondeur. (due a la lenteur de la fonction de score).
 */
void appliquer_move(gamestate& g, move_t& mv) {
	// Deplacement de l'agent sur la carte
	int val = g.p[mv.start];
	g.p[mv.start] = 0;
	g.p[mv.pos] = val;
	// Deplacement de l'agent dans les variables de positions
	if(val == AGENT_MOI) {
		for(int i = 0 ; i < 4 ; i++) {
			if(g.agents_moi[i] == mv.start) {
				g.agents_moi[i] = mv.pos;
				g.agents_PA[i] -= mv.cout;
			}
		}
	}
	if(val == AGENT_LUI) {
		for(int i = 0 ; i < 4 ; i++) {
			if(g.agents_lui[i] == mv.start) {
				g.agents_lui[i] = mv.pos;
				g.agents_PA[i] -= mv.cout;
			}
		}
	}
	
	// Si on a le flag "pousser", on recupere la position a deplacer, puis on calcule la direction et on effectue le poussement.
	if((mv.flags & POUSSER_FLAG) > 0) {
		int to_push = mv.flags & 1023; // Les premiers bits correspondent a la position de l'objet a pousser
		int res = pousser_res(g.p, to_push, diff_to_direction(to_push - mv.pos)); // On calcule la destination de l'objet pousse
		int pushed_id = g.p[to_push];
		g.p[to_push] = 0;
		g.p[res] = pushed_id;
		if(res != to_push) { // Si l'objet a effectivement ete deplace, la capture est remis a zero.
			if(g.cap[to_push] == 1) {// On va enregistrer l'ancien etat de la capture (1 ou 2) dans des flags correspondant.
				mv.flags |= CAP_1_FLAG;
			} else if(g.cap[to_push] == 2) {
				mv.flags |= CAP_2_FLAG;
			} else if(g.cap[to_push] == 3) {
				mv.flags |= CAP_3_FLAG;
			} else if(g.cap[to_push] > 2) {
				printf("AH ! Cap corrupted.\n");
			}
			g.cap[to_push] = 0;
		}
		
		if(pushed_id == AGENT_MOI) { // On deplace aussi la variable de l'agent affecte
			for(int i = 0 ; i < 4 ; i++) {
				if(g.agents_moi[i] == to_push) {
					g.agents_moi[i] = res;
				}
			}
		}
		if(pushed_id == AGENT_LUI) {
			for(int i = 0 ; i < 4 ; i++) {
				if(g.agents_lui[i] == to_push) {
					g.agents_lui[i] = res;
				}
			}
		}
	}
}

/**
 * Fonction reciproque de "appliquer_move", elle effectue precisemment le code inverse, en remettant les choses a leur places.
 * On remarque que cela signifie que "move" doit contenir toute les informations concernant le changement d'etat.
 */
void annuler_move(gamestate& g, move_t& mv) {
	// Deplacement de l'agent
	int val = g.p[mv.pos];
	g.p[mv.pos] = 0;
	g.p[mv.start] = val;
	if(val == AGENT_MOI) {
		for(int i = 0 ; i < 4 ; i++) {
			if(g.agents_moi[i] == mv.pos) {
				g.agents_moi[i] = mv.start;
				g.agents_PA[i] += mv.cout;
			}
		}
	}
	if(val == AGENT_LUI) {
		for(int i = 0 ; i < 4 ; i++) {
			if(g.agents_lui[i] == mv.pos) {
				g.agents_lui[i] = mv.start;
				g.agents_PA[i] += mv.cout;
			}
		}
	}
	// On remet l'agent poussee
	if((mv.flags & POUSSER_FLAG) > 0) {
		int to_push = mv.flags & 1023;
		direction dir = diff_to_direction(to_push - mv.pos);
		int res = pousser_res(g.p, to_push, dir); // Calcul de la localisation de l'objet precedemment poussee
		if((mv.flags & CAP_1_FLAG) > 0) { // Si l'agent etait en train de cap, on remet les variables de cap.
			g.cap[to_push] = 1;
		}
		if((mv.flags & CAP_2_FLAG) > 0) {
			g.cap[to_push] = 2;
		}
		if((mv.flags & CAP_3_FLAG) > 0) {
			g.cap[to_push] = 3;
		}
		if(g.p[res] > 2) {
			return;
		}
		// Etant donne qu'on a envoye un agent fantome pour voir ou il atterirait, 
		// il touchera l'agent precedemment deplace, il faut donc effectuer un mouvement 
		// de plus dans cette direction.
		if(dir == EST) {
			res += 1;
		}
		if(dir == OUEST) {
			res -= 1;
		}
		if(dir == NORD) {
			res -= 25;
		}
		if(dir == SUD) {
			res += 25;
		}
		int pushed_id = g.p[res]; // Enchangement des coordonnees
		g.p[res] = 0;
		g.p[to_push] = pushed_id;
		if(pushed_id == AGENT_MOI) {
			for(int i = 0 ; i < 4 ; i++) {
				if(g.agents_moi[i] == res) {
					g.agents_moi[i] = to_push;
				}
			}
		}
		if(pushed_id == AGENT_LUI) {
			for(int i = 0 ; i < 4 ; i++) {
				if(g.agents_lui[i] == res) {
					g.agents_lui[i] = to_push;
				}
			}
		}
	}
}

/**
 * Cette fonction permet d'appeller l'API pour le deplacement d'un agent a l'aide des graphs donne par le djisktra.
 * (n'inclue donc pas l'action de pousser par exemple, qui est gere par jouer_coup).
 */
int deplacer_agent_api(int agent_id, int start, int dst, plateau& dual_graph) {
	int len = 0;
	std::vector<int> pos_succ = std::vector<int>();
	
	// On traverse le graph dual pour trouver le chemin le plus court
	while(dst != start) {
		pos_succ.push_back(dst);
		len += 1;
		dst = dual_graph[dst];
	}
	
	// On effectue les deplacements en appellant l'API
	for(int i = 0 ; i < len ; i++) {
		int next = pos_succ.back();
		pos_succ.pop_back();
		int diff = next-start;
		//printf("I should be at %d-%d --> %d-%d diff: %d\n", start%25, start/25, next%25, next/25, diff);
		if(diff == 1) {
			int err = deplacer(agent_id, EST);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else if(diff == -1) {
			int err = deplacer(agent_id, OUEST);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else if(diff == 25) {
			int err = deplacer(agent_id, SUD);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else if(diff == -25) {
			int err = deplacer(agent_id, NORD);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else if(diff > 0 && diff < 25) {
			int err = glisser(agent_id, EST);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else if(diff < 0 && diff > -25) {
			int err = glisser(agent_id, OUEST);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else if(diff > 0) {
			int err = glisser(agent_id, SUD);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		} else  {
			int err = glisser(agent_id, NORD);
			if(err != OK) { printf("Err trying to move err code: %d\n", err); break; }
		}
		start = next;
	}
	// On renvoie la position effective (et non celle demandee). Pratique pour debugger.
	return start;
}

/**
 * Cette fonction est appelle lorsque le coup finale a ete decide. Elle effectue le lien entre
 * les structures de donnees utilise et l'API officielle.
 */
void jouer_coup(gamestate& g, move_t& mv) {
	// On regarde quel agent est cense effectuer l'action
	int agent_id = -1;
	for(int i = 0 ; i < 4 ; i++) {
		if(g.agents_moi[i] == mv.start) {
			agent_id = i;
			break;
		}
	}
	if(agent_id == -1) { // Si l'action est MOVE_NULL, il n'y aura pas d'agent associe
		return;
	}
	// On calcule les informations pour trouver le chemin le plus court afin de deplacer l'agent.
	plateau res, dual; 
	dist_to_cases(g.p, mv.start, res, dual);
	// On effectue le deplacement
	int end = deplacer_agent_api(agent_id, mv.start, mv.pos, dual);
	
	// Si l'agent n'est pas arrivee a destination.. Quelque chose s'est mal passe. On affiche des informations pour debug.
	if(end != mv.pos) {
		afficher_plateau(g.p, true, g.tour);
		afficher_coup(mv);
		for(int i = 0 ; i < 4 ; i++) {
			printf("%d ", g.agents_PA[i]);
		}
		printf(" %d\n", agent_id);
		printf("Something is wrong.\n");
	}

	if((mv.flags & POUSSER_FLAG) > 0) { // Si l'action inclue le fait de pousser, on le fait.
		int to_push = mv.flags & 1023; // Position de l'objet a pousser
		int res = pousser(agent_id, diff_to_direction(to_push - mv.pos));
		if(res != OK) { // Pousser a rate..
			printf("Something is wrong with POUSSER. err code: %d\n", res);
		}
	}
}

/**
 * L'algorithme principal de recherche de chemin. C'est donc simplement un djikstra avec quelques subtilitees...
 * 
 * 1. Il calcule la distance en prenant en compte les 8 PA par tour. 
 *    En effet, cela ne coute pas 3 PA de faire un glissement quand on a 1 PA restants mais 4 
 *    (car il faut attendre la fin du tour).
 * 
 * 2. La file de priorite qu'il utilise sacrifie de la memoire au prix des performances. 
 *    En effet, on appellera plusieurs milliers de fois cette fonction qui recalcule a chaque fois les distances vers toute les cases de la map.
 *    Toute la memoire est donc prealloue en faisant 2 suppositions.
 *     - Un chemin ne demandera jamais plus de 350 PA (raisonnable, car < 25*25/2) --> MAX_DIST
 *     - Il n'y aura jamais plus de 200 positions ayant la meme distance a l'agent initial. --> MAX_SAME_DIST
 *       Aussi raisonnable car la distance est similaire a manhattan (pas exact a cause des glissades).
 *       et MAX_SAME_DIST correspond a la taille maximale du perimetre, qui est autour de ~4*25/2 au maximum.
 *    Cela permet d'initialiser toute la memoire des le debut, et ne necessite donc pas de mettre a jour+allouer des listes, 
 *    mais simplement des pointeurs vers la queue des listes. (situee dans pointers)
 * 
 *    A noter que grace a la magie de cette methode, il n'y a pas besoin de liberer/nettoyer tout le tableau a chaque fois,
 *    memes les pointeurs sont tous a zero quand l'algorithme se fini par definition ! (toutes les files sont vides)
 * 
 *  Concernant les arguments, result contiendra les distances d'un point "pos_start" a chaque point atteignable de la map.
 *  Les murs/agents contiendront aussi une valeur qui correspond au min des valeurs autour d'eux. 
 *  (pratique pour avoir la distance a un alien qui a deja un agent sur lui)
 */

#define MAX_DIST 350
#define MAX_SAME_DIST 200

int prio_queue[MAX_DIST*MAX_SAME_DIST]; // Stack de priorite (revient au meme qu'une file dans notre cas)
int pointers[MAX_DIST]; // Pointeurs vers la queue

// Permet d'ajouter un element a la fin d'une queue 
inline void prio_push_back(int queue, int pos) {
	prio_queue[queue+pointers[queue]*MAX_DIST] = pos;
	pointers[queue] += 1;
}

// Permet d'enlever + recuperer un element a la fin de la queue
inline int prio_pop_back(int queue) {
	pointers[queue] -= 1;
	return prio_queue[queue+pointers[queue]*MAX_DIST];
}

// Simple djistra, comme explique plus haut.
void dist_to_cases(plateau& pla, int pos_start, plateau& result, plateau& dual_graph) {
	// On initialise le tableau de resultat a +inf (ici represente par 1000)
	for(int i = 0 ; i < 625 ; i++) {
		result[i] = 1000;
	}
	
	result[pos_start] = 0;
	
	prio_push_back(0, pos_start);
	int t = 0;
	int left = 1;
	
	// Tableau qui contiendra les mouvements, plus efficace qu'une liste (m'a fait gagner 2x en perfs)
	int moves[9];
		
	while(left > 0) {
		int p = prio_pop_back(t);
		gen_mouvements(pla, p, moves);
	
		// On itere sur les mouvements
		for(int m_id = 0 ; m_id < 8 ; m_id++) {
			int move = moves[m_id];
			if(move < 0)
				break;
			int t_arrival = 1 + result[p];
			if((move & GLISSER_FLAG) > 0) { // Glissade.. coute un peu plus cher, et depends des PA restants.
				int reste = 8-(result[p]%8);
				if(reste >= 3)
					reste = 0;
				t_arrival += 2 + reste;
				move -= GLISSER_FLAG;
			}
			if(pla[move] > 0) { // Deja qqun sur l'arrivee, on mets quand meme la distance (ainsi a la fin, cela correspondra au min des distances autour)
				if(t_arrival - 1 < result[move]) {
					result[move] = t_arrival - 1;
					dual_graph[move] = p;
				}
				continue;
			}
			if(t_arrival < result[move] && t_arrival < MAX_DIST) { // On met a jour la file de prio
				result[move] = t_arrival;
				dual_graph[move] = p;
				prio_push_back(t_arrival, move);
			}
		}
		left -= 1; // On a consomme l'element
		while(left == 0 && t < MAX_DIST - 1) { // On avance dans la file de prio pour recuperer les prochains elements a traiter
			t += 1;
			left = pointers[t];
		}
	}
}

/**
 * Fonction principale donc, qui calcule le score d'un etat de jeu donnee (positions des agents, points de capture etc.)
 * Voir description en haut du fichier pour son fonctionnement detaille, etant donne que c'est un des points majeurs de 
 * la strategie
 */

// Coefficient de points associe a la capture d'un alien (coefficient majeur, donne 95% du score d'un plateau)
float coeff_capture = 0.4f;
// Coefficient de distances a l'agent le plus proche
float coeff_dist_score = 0.001f;
// Coefficient de presence sur le plateau
float coeff_presence = 0.0006f;
// Pourcentage de points perdue si la position est poussable
float coeff_kickable = 0.7f;
// Coefficient de prediction d'arrivee des aliens
float coeff_oracle = 0.025f;

float score_state(gamestate& g) {
	// Ce qui prends la majorite du temps de la fonction de cout
	// Calcul des distances les plus courtes sur tout le plateau pour tout les agents de la map
	plateau dists_moi[4]; // distances pour mes agents
	plateau dists_lui[4]; // distances pour ses agents
	plateau dual; // inutilise apres, simplement necessaire pour le calcul
	for(int i = 0 ; i < 4 ; i++) {
		dist_to_cases(g.p, g.agents_moi[i], dists_moi[i], dual);
		dist_to_cases(g.p, g.agents_lui[i], dists_lui[i], dual);
	}
	
	// Calcul du coefficient de presence
	float presence = 0;
	for(alien_info& al : aliens_list) {
		int i = from_position(al.pos);
		int u = 0;
		int val = g.p[i];
		if(val == AGENT_LUI) { // La case lui appartient deja 
			u = -1;
		} else if(val == AGENT_MOI) {// La case m'appartient deja
			u = 1;
		} else { // La persone la plus proche possede cette case
			int min = 1000;
			u = 1;
			for(int j = 0 ; j < 4 ; j++) {
				if(dists_moi[j][i] < min) {
					min = dists_moi[j][i];
				}
			}
			for(int j = 0 ; j < 4 ; j++) {
				if(dists_lui[j][i] == min) {
					u = 0;
				}
				if(dists_lui[j][i] < min) {
					u = -1;
					break;
				}
			}
		}
		
		// Pondere par les points de capture et la distance a l'apparition
		if(al.tour_invasion + al.duree_invasion >= g.tour) {
			presence += u * al.points_capture / (1 + abs(al.tour_invasion + al.duree_invasion / 2 - g.tour));
		}
	}
	
	float capture_score = 0;
	// Calcul du coefficient de capture pour moi
	for(int i = 0 ; i < 4 ; i++) {
		alien_info al = aliens_pos[g.agents_moi[i]];
		
		if(est_capturable(al, g.tour, g.cap[g.agents_moi[i]])) { // l'alien est capturable
			#ifdef DEBUG
			printf("%d-%d ", g.agents_moi[i]%25, g.agents_moi[i]/25);
			#endif
			
			capture_score += al.points_capture * (1 + g.cap[g.agents_moi[i]]*g.cap[g.agents_moi[i]]); // On ajoute au score poids * cap^2
			bool coin = est_safe(g, from_position(al.pos), AGENT_MOI); // Si je suis safe avec l'aide d'un agent alie et des murs, je ne pourrais pas etre pousse
			
			#ifdef DEBUG
			printf("safe: %d gives %d", coin, al.points_capture * (1 + g.cap[g.agents_moi[i]]*g.cap[g.agents_moi[i]]));
			#endif
			
			if(!coin) { // Je ne suis pas safe.. mais il ne peut pas forcement me pousser
				for(int j = 0 ; j < 4 ; j++) {
					// Eh bien si.. un des agents adverses peut me pousser avant que je finisse de capturer
					// A noter que je verifie que l'agent pouvant me pousser ne vient pas d'un alien vallant plus que moi, auquel cas l'operation est negative pour lui,
					// donc on se fiche qu'il vienne.
					if(aliens_pos[g.agents_lui[j]].points_capture <= al.points_capture && dists_lui[j][g.agents_moi[i]] <= 3+8*(2-g.cap[g.agents_moi[i]])) { 
						
						#ifdef DEBUG
						//printf(" can be kicked -> -%f", al.points_capture * (1 + g.cap[g.agents_moi[i]]*g.cap[g.agents_moi[i]]) * coeff_kickable);
						#endif
						
						// Je peut etre pousse, je perds donc coeff_kickable pourcents des points de la capture
						capture_score -= al.points_capture * (1 + g.cap[g.agents_moi[i]]*g.cap[g.agents_moi[i]]) * coeff_kickable;
						break;
					}
				}
			}
			#ifdef DEBUG 
			printf("\n");
			#endif
			
		}
		#ifdef DEBUG 
		else {
			//printf("no alien %d %d %d cap %d\n", al.tour_invasion, al.duree_invasion, g.tour, g.cap[g.agents_moi[i]]);
		}
		#endif
	}
	
	// Calcul du score associe a l'apparition proche d'aliens
	float oracle_score = 0.0f;
	for(int i = 0 ; i < 4 ; i++) {
		alien_info al = aliens_pos[g.agents_moi[i]];
		if(al.tour_invasion > g.tour) {
			oracle_score += al.points_capture / (float)(al.tour_invasion - g.tour);
			bool coin = est_safe(g, from_position(al.pos), AGENT_MOI);
			//printf("%d-%d safe: %d\n", g.agents_moi[i]%25, g.agents_moi[i]/25, coin);
			if(!coin) {
				for(int j = 0 ; j < 4 ; j++) {
					if(dists_lui[j][g.agents_moi[i]] <= 3) {
						oracle_score -= (al.points_capture * coeff_kickable) / (float)(al.tour_invasion - g.tour);
						break;
					}
				}
			}
		}
		al = aliens_pos[g.agents_lui[i]];
		if(al.tour_invasion > g.tour) {
			oracle_score -= al.points_capture / (float)(al.tour_invasion - g.tour);
			bool coin = est_safe(g, from_position(al.pos), AGENT_LUI);
			if(!coin) {
				for(int j = 0 ; j < 4 ; j++) {
					if(dists_moi[j][g.agents_lui[i]] <= 3) {
						oracle_score += (al.points_capture * coeff_kickable) / (float)(al.tour_invasion - g.tour);
						break;
					}
				}
			}
		}
	}
	// Calcul du score de capture pour l'enemi
	for(int i = 0 ; i < 4 ; i++) {
		alien_info al = aliens_pos[g.agents_lui[i]];
		
		if(est_capturable(al, g.tour, g.cap[g.agents_lui[i]])) {
			#ifdef DEBUG
			int pos = from_position(al.pos);
			//printf("enemy.. %d-%d substracts %d\n", pos%25, pos/25, al.points_capture * (1 + g.cap[g.agents_lui[i]]));
			#endif
			capture_score -= al.points_capture * (1 + g.cap[g.agents_lui[i]]);
		}
	}
	
	float dist_score = 0;
	// Calcul de score de distances aux aliens importants (plus proche = mieux)
	for(alien_info& al : aliens_list) {
		int al_pos = from_position(al.pos);
		if(est_capturable(al, g.tour, 0) && !(g.p[al_pos] > 2 && g.cap[al_pos] == 2)) {
			int min_dist = 100;
			// On regarde l'agent le plus proche pour lui et moi, et le score correspond a la difference de la distance.
			for(int i = 0 ; i < 4 ; i++) { 
				if(g.agents_moi[i] != al_pos && est_capturable(aliens_pos[g.agents_moi[i]], g.tour, g.cap[g.agents_moi[i]]))
					continue;
				int t_d = dists_moi[i][al_pos];
				if(t_d < min_dist) {
					min_dist = t_d;
				}
			}
			int min_dist_e = 100;
			for(int i = 0 ; i < 4 ; i++) {
				if(g.agents_lui[i] != al_pos && est_capturable(aliens_pos[g.agents_lui[i]], g.tour, g.cap[g.agents_lui[i]]))
					continue;
				int t_d = dists_lui[i][al_pos];
				if(t_d < min_dist_e) {
					min_dist_e = t_d;
				}
			}
			// On ajoute aussi un tout petit score qui indique simplement d'etre proche de l'alien, sans histoire de comparaison
			if(min_dist != 100) {
				dist_score -= min_dist * 0.001f;
			}
			
			if(min_dist_e == 100 || min_dist == 100) {
				continue;
			}
			
			dist_score += ((min_dist_e/8 - min_dist / 8)*2 + (min_dist_e - min_dist)) * al.points_capture;
		}
	}
	#ifdef DEBUG
	printf("%f %f %f %f \n", oracle_score * coeff_oracle, capture_score * coeff_capture, dist_score * coeff_dist_score, presence * coeff_presence);
	#endif
	// Si le coeff de distance est trop grand, c'est probablement qu'il n'est pas tres pertinent. 
	// La fonction suit donc une courbe affine avant de se pencher.
	//           |  _____/
	//           | /|
	//           |/ |
	// ----------+--4-------> x
	//          /|
	//   ______/ |
	//  /        |
	//--
	
	dist_score *= coeff_dist_score;
	if(fabs(dist_score) > 4) {
		if(dist_score > 0)
			dist_score = 4 + (dist_score - 4 ) / 10.0f;
		else
			dist_score = -4 + (dist_score + 4 ) / 10.0f;
	}
	
	// On effectue simplement la somme pondere de ces scores
	return oracle_score * coeff_oracle + capture_score * coeff_capture + dist_score + presence * coeff_presence;
}

#ifdef DEBUG
/** A chaque tour, l'IA va afficher toutes les informations necessaires
 *  pour reconstituer l'etat du jeu (afin de comprendre ce qui ne vas pas, en allant dans les details).
 *  
 *  Cette fonction est donc tachee de reconstituer l'etat a partir d'un plateau ascii.
 *  Evidemment, tout ne peut pas etre contenu dans un tel plateau, c'est pour cela que les informations restantes
 *  sont ajoutes a la main, dans jouer_tour (voir plus bas).
 * 
 *  Ainsi, l'idee est de copier coller le plateau si besoin. et avec qqs etapes de plus automatisees dans un script python, 
 *  on peut facilement reconstituer l'etat voulu et afficher beaucoup plus d'info.
 */
void parse_ascii(gamestate& g) {
	int cnt = 0;
	int cnt2 = 0;
	
const char* a = ".........................\
..*...................*..\
.........................\
.........M**.**E.........\
........**.....**........\
.........................\
...M..*...........*..E...\
...*..**..*...*..**..*...\
......a...........a......\
.....*..*.......*..*.....\
*.......*.**.**.*.......*\
*..*...*...*.*...*...*..*\
............*............\
.....*...*.....*...*.....\
........*...*...*........\
........*.......*........\
...*.................*...\
M.......................E\
*...*...............*...*\
........***...***........\
*..*........*........*..*\
.**...................**.\
..*...................*..\
......*....*.*....*......\
......*M.........E*......";

	for(int i = 0 ; i < 625 ; i++) {
		char v = a[i];
		if(v == '.') {
			g.p[i] = LIBRE;
		}
		if(v == 'E') {
			g.p[i] = AGENT_LUI;
			g.agents_lui[cnt++] = i;
		}
		if(v == 'M') {
			g.p[i] = AGENT_MOI;
			g.agents_moi[cnt2++] = i;
		}
		if(v == '*') {
			g.p[i] = MUR;
		}
		g.cap[i] = 0;
	}
}
#endif

/**
 * Une des fonctions majeurs de l'IA.
 * Cette fonction va s'occuper de generer tout les coups potentiels de tout les agents.
 * La liste de coups potentiels est :
 *  - ne rien faire
 *  - deplacement vers une case avec un alien dessus (ou qui va apparaitre bientot).
 *  - deplacement vers les bords d'une case avec un alien ET un agent QUELCONQUE dessus 
 *  - deplacement vers les bords d'une case avec un alien ET un agent ENNEMI dessus, suivie d'une action POUSSER.
 *  - deplacement vers les bords d'une case avec un agent ENNEMI dessus seulement, suvie d'une action POUSSER.
 *  side correspond au cote duquel on doit generer des coups (le sien ou le cote ennemi).
 */
std::vector<move_t> gen_interesting_moves(gamestate& state, int side) {
	std::vector<move_t> good_moves = std::vector<move_t>();
	// Ne rien faire
	move_t null_move;
	null_move.start = 0;
	null_move.pos = 0;
	null_move.flags = 0;
	null_move.cout = NULLMOVE_FLAG;
	good_moves.push_back(null_move);
	
	int LUI = (side == api_moi()) ? AGENT_LUI : AGENT_MOI;
	
	// On boucle sur les 4 agents
	for(int i = 0 ; i < 4 ; i++) {
		//printf("%d -> %d\n", i, state.agents_PA[i]);
		if(state.agents_PA[i] == 0) // On verifie qu'on peut bouger
			continue;
		int pos = (side == api_moi()) ? state.agents_moi[i] : state.agents_lui[i]; // On recupere la position de l'agent
		// On calcule les distances minimales a toutes les cases a partir de la position de l'agent
		plateau res, dual;
		dist_to_cases(state.p, pos, res, dual);
		
		// On genere les coups consistant a aller sur la position d'un alien
		for(alien_info& al : aliens_list) {
			int al_pos = from_position(al.pos);
			
			//printf("%d -> %d\n", al_pos, state.p[al_pos]);
			
			// On verifie que l'alien existe (sinon, points capture vaut 0), 
			// et qu'il n'y a pas deja qqun sur la case.
			if(al.points_capture == 0 || state.p[al_pos] > 0)
				continue;
			
			// On regarde la distance en tour jusqu'a l'alien
			int t_dist = res[al_pos]/8;
			// Si on peut encore le capturer une fois atteint
			if(est_capturable(al, state.tour+t_dist, state.cap[al_pos])) { 
				// alors on ajoute le coup a la liste
				int to_add = al_pos;
				// A noter qu'on ajoute en realite le plus proche rapprochement de cette case (on a que 8 PA en tout, apres tout)
				while(res[to_add] > state.agents_PA[i]) { 
					to_add = dual[to_add];
				}
				
				move_t mv;
				mv.pos = to_add;
				mv.start = pos;
				mv.flags = 0;
				mv.cout = res[to_add];
				if(mv.pos != mv.start) {
					good_moves.push_back(mv);
				}
			}
			// sinon si on ne peut pas le capturer (en effet, il n'est pas encore sur le terrain) 
			// mais qu'il arrive sur le terrain dans moins de cinq tours, on considere quand meme le deplacement.
			else if(al.tour_invasion > state.tour && al.tour_invasion < state.tour + 5 && res[al_pos] <= state.agents_PA[i]) {
				int to_add = al_pos;
				move_t mv;
				mv.pos = to_add;
				mv.start = pos;
				mv.flags = 0;
				mv.cout = res[to_add];
				if(mv.pos != mv.start) {
					good_moves.push_back(mv);
				}
			}
		}
		
		// On considere les aliens ayant quelqu'un dessus
		for(alien_info& al : aliens_list) { 
			if(al.points_capture == 0)
				continue;
			int al_pos = from_position(al.pos);
			int t_dist = res[al_pos]/8; 
			
			// Si on peut le capturer une fois arrive (a cote), et qu'il Y A quelqu'un dessus
			if(est_capturable(al, state.tour+t_dist, state.cap[al_pos]) && state.p[al_pos] > 0) {
				// On considere les cotes de la case
				for(int& m : gen_sides(state.p, al_pos)) { 
					
					// On verifie que la case est atteignable et 
					// qu'il n'y a personne dessus (excepte si c'est de la ou je part)
					if(res[m] == 1000 || (state.p[m] > 0 && m != pos))
						continue;
					
					int to_add = m;
					
					// Comme avant, on rajoute en realite le plus grand rapprochement de la case.
					while(res[to_add] > state.agents_PA[i]) {
						to_add = dual[to_add];
					}
					move_t mv;
					mv.flags = 0;
					
					// On ne vient pas pour rien, on vient pour pousser si on est suffisamment pres
					// et que l'agent sur la case est bien un ennemi (LUI = enemi)
					if(state.p[al_pos] == LUI && res[m] <= 3-8+state.agents_PA[i]) {
						// auquel cas, on indique qu'on va effectuer un deplacement et on mets
						// le flag "POUSSER" ainsi que la position de la case dans le payload.
						mv.flags = POUSSER_FLAG | al_pos;
					}
					
					mv.pos = to_add;
					mv.start = pos;
					mv.cout = res[to_add] + (((mv.flags & POUSSER_FLAG) > 0) ? 5 : 0);
					
					// On verifie qu'on a bien fait un deplacement (le non deplacement est deja gere par "NULL MOVE")
					// excepte si c'est pour pousser l'enemi
					if(mv.pos != mv.start || (mv.flags & POUSSER_FLAG) > 0) {
						good_moves.push_back(mv);
					}
				}
			}
		}
		// Cette fois ci, on va considerer le fait de pousser un agent enemi a un emplacement lambda
		// (pas forcement sur un alien). 
		for(int j = 0 ; j < 4 ; j++) {
			int lui_pos = (side == api_moi()) ? state.agents_lui[j] : state.agents_moi[j];
			// On regarde les bords, et on verifie qu'ils sont atteignables (personne dessus)
			// on verifie aussi qu'il nous restera au moins 3 PA apres avoir efffectue l'action
			for(int& m : gen_sides(state.p, lui_pos)) { 
				if(res[m] > 3-8+state.agents_PA[i] || (state.p[m] > 0 && m != pos))
					continue;
				int to_add = m;
				
				move_t mv;
				mv.flags = POUSSER_FLAG | lui_pos;
				mv.pos = to_add;
				mv.start = pos;
				mv.cout = res[to_add] + 5;
				good_moves.push_back(mv);
			}
		}
	}
	// Si l'on a genere tres peu de coup, on va aussi considerer des mouvements pas forcements interessants.
	// Ceux-ci sont simplement les glissades ou les deplacements de 1 case.
	if(good_moves.size() < 10) { 
		for(int i = 0 ; i < 4 ; i++) {
			int pos = (side == api_moi()) ? state.agents_moi[i] : state.agents_lui[i];
			if(state.agents_PA[i] == 0)
				continue;
			// il nous reste >=3 PA, on genere les glissades
			if(state.agents_PA[i] >= 3) {
				for(direction& dir : directions) {
					int res = pousser_res(state.p, pos, dir);
					if(res != pos) {
						move_t mv;
						mv.flags = 888;
						mv.pos = res;
						mv.start = pos;
						mv.cout = 3;
						good_moves.push_back(mv);
					}
				}
			} 
			// sinon, on genere les deplacements directs.
			else {
				for(int& m : gen_sides(state.p, pos)) {
					if(state.p[m] <= 0) {
						move_t mv;
						mv.flags = 888;
						mv.pos = m;
						mv.start = pos;
						mv.cout = 1;
						good_moves.push_back(mv);
					}
				}
			}
		}
	}
	return good_moves;
}

/**
 * Fonction reduite de "gen_interesting_moves" qui va generer des petits coups au cas ou il y aurait trop de possibilitees.
 * Elle est utilise dans le cas ou l'on manque de temps ou qu'il y a trops de calcul a effectuer.
 * 
 *  Ceux-ci incluent :
 *   - ne rien faire
 *   - glisser
 *   - se deplacer d'une case
 *   - glisser + pousser agent enemi
 */
std::vector<move_t> gen_fallback_moves(gamestate& state, int agent) {
	std::vector<move_t> good_moves = std::vector<move_t>();
	
	// ne rien faire
	move_t null_move;
	null_move.start = 0;
	null_move.pos = 0;
	null_move.flags = 0;
	null_move.cout = NULLMOVE_FLAG;
	good_moves.push_back(null_move);
	
	int pos = state.agents_moi[agent];
	
	// glissades
	if(state.agents_PA[agent] >= 3) {
		for(direction& dir : directions) {
			int res = pousser_res(state.p, pos, dir);
			if(res != pos) {
				move_t mv;
				mv.flags = 888;
				mv.pos = res;
				mv.start = pos;
				mv.cout = 3;
				good_moves.push_back(mv);
			}
		}
	}
	
	// deplacements
	for(int& m : gen_sides(state.p, pos)) {
		if(state.p[m] <= 0) {
			move_t mv;
			mv.flags = 888;
			mv.pos = m;
			mv.start = pos;
			mv.cout = 1;
			good_moves.push_back(mv);
		}
	}
	
	// pousser les ennemis
	plateau res, dual;
	dist_to_cases(state.p, pos, res, dual);
	for(int j = 0 ; j < 4 ; j++) {
		int lui_pos = state.agents_lui[j];
		
		for(int& m : gen_sides(state.p, lui_pos)) {
			if(res[m] > 3-8+state.agents_PA[agent] || (state.p[m] > 0 && m != pos))
				continue;
			int to_add = m;
			
			move_t mv;
			mv.flags = POUSSER_FLAG | lui_pos;
			mv.pos = to_add;
			mv.start = pos;
			mv.cout = res[to_add] + 5;
			good_moves.push_back(mv);
		}
	}
	return good_moves;
}

/** Fonction qui s'occupe de transformer les informations contenues dans l'API dans le modele de donnee local. (gamestate)
 *  Comme par exemple la position des agents, le cap des aliens etc...
 */
void charger_jeu(gamestate& g) {
	g.tour = api_tour_actuel();
	
	// cases (LIBRE ou MUR)
	for(int i = 0 ; i < 625 ; i++) {
		position pos;
		pos.ligne = i/25;
		pos.colonne = i%25;
		case_type info = type_case(pos);
		g.p[i] = info;
		g.cap[i] = 0;
	}
	
	// capture en cours des aliens
	for(alien_info& inf : aliens_list) {
		if(alien_sur_case(inf.pos)) {
			g.cap[from_position(inf.pos)] = info_alien(inf.pos).capture_en_cours;
		} else if(g.tour >= inf.tour_invasion && g.tour < inf.tour_invasion+inf.duree_invasion) {
			g.cap[from_position(inf.pos)] = 3;
			debug_afficher_drapeau(inf.pos, DRAPEAU_VERT);
		}
	}
	
	// position des agents allies
	for(int i = 0 ; i < 4 ; i++) {
		position agent_pos = position_agent(api_moi(), i);
		g.agents_moi[i] = from_position(agent_pos);
		g.p[g.agents_moi[i]] = AGENT_MOI;
		g.agents_PA[i] = 8;
	}
	
	// position des agents enmemis
	for(int i = 0 ; i < 4 ; i++) {
		position agent_pos = position_agent(api_adversaire(), i);
		g.agents_lui[i] = from_position(agent_pos);
		g.p[g.agents_lui[i]] = AGENT_LUI;
	}
	
}

/// Fonction appelée au début de la partie.
void partie_init()
{
	printf("Init start..\n");
	// Les aliens qui n'existe pas vraiment apparaissent au tour -1000 pendant 0 tours et rapportent 0
	for(int i = 0 ; i < 625 ; i++) {
		aliens_pos[i].tour_invasion = -1000;
		aliens_pos[i].duree_invasion = 0;
		aliens_pos[i].points_capture = 0;
		aliens_pos[i].duree_invasion = 0;
	}
	
	// Initialisation pour le djisktra
	for(int i = 0 ; i < MAX_DIST ; i++) {
		pointers[i] = 0;
	}
	
	// On charge ainsi les deux variables contenant les aliens (on a besoin de le faire qu'une seule fois)
	aliens_list = liste_aliens();
	for(alien_info& alien : aliens_list) {
		aliens_pos[from_position(alien.pos)] = alien;
	}
	printf("Init OK\n");
}

/// Fonction appelée à chaque tour.
/// voir recapitulatif du fonctionnement dans le paragraphe en haut
void jouer_tour()
{
	// Etat qui va contenir toutes les informations
	gamestate state;
	#ifndef DEBUG
	// Si on est dans une vraie partie, on charge les informations
	charger_jeu(state);
	#endif
	
	// Sinon, on ajoute les informations manquantes comme la position des agents et le tour actuel.
	// A noter qu'il n'etait pas necessaire de reecrire ces donnees a chaque fois, 
	// un code python generait le code a partir de l'output de afficher_state et il ne restait plus qu'a copier/coller
	#ifdef DEBUG
	parse_ascii(state);
	state.agents_moi[0] = 425 ;
	state.agents_moi[1] = 153 ;
	state.agents_moi[2] = 84 ;
	state.agents_moi[3] = 607 ;
	state.agents_lui[0] = 449 ;
	state.agents_lui[1] = 171 ;
	state.agents_lui[2] = 90 ;
	state.agents_lui[3] = 617 ;

	state.agents_PA[0] = 8;
	state.agents_PA[1] = 8;
	state.agents_PA[2] = 8;
	state.agents_PA[3] = 8;
	state.tour = 0;
	#endif
	
	// Pour verifier la prediction d'apparition des aliens, des drapeaux bleu quand ils apparaitront dans 2 tours,
	// et des drapeaux rouge quand ils apparaitront dans 1 tour.
	for(alien_info& al : aliens_list) {
		if(al.tour_invasion == state.tour + 2) {
			debug_afficher_drapeau(al.pos, DRAPEAU_BLEU);
		}
		if(al.tour_invasion == state.tour + 1) {
			debug_afficher_drapeau(al.pos, DRAPEAU_ROUGE);
		}
	}
	
	printf("Tour %d --- \n", state.tour);
	// On va simplement utiliser une std::clock pour verifier qu'il nous reste du temps.
	// Etant donne que le programme est run sur un seul coeur, il n'y a pas de multithreading et 
	// std::clock devrait etre realiste.
	auto time = std::clock();
	auto time2 = std::clock();
	
	// on compte les PA restants, pour ne pas demander a generer les coups si on a deja tout epuise.
	// A noter que cela date du tout debut du code.. je ne sais pas si cette variable est vraiment utilise.
	int total = 32;
	
	// variable qui va contenir le meilleur coup succesivement
	move_t best_move;
	best_move.start = -1;

	float max_score = -1000000;
	
	// Compteur de coups generes au total (dont on a appele la fonction de score).
	// pratique pour pour obtenir le temps par coup.
	int coups = 0;
	
	// On affiche l'etat actuel pour la reproducibilite
	afficher_state(state);
	
	// Variable indiquant si on doit utiliser le fallback (gen_fallback_moves)
	bool use_fallback = false;
	do {
		// En mode debug, on va afficher le plateau apres chaque coup
		#ifdef DEBUG
		afficher_plateau(state.p, true, state.tour);
		#endif
		
		
		/// On va ici lister les coups et chercher le score maximum
		
		// on genere les coups
		std::vector<move_t> moves = gen_interesting_moves(state, api_moi());
		
		
		// Si l'on est pas en mode debug, on verifie qu'il n'y a pas trop de coup avec le temps restant
		int max_moves = (850 - 1000*(time2-time)/CLOCKS_PER_SEC) * 2;
		int mv_count = 0;
		
		#ifndef DEBUG
		if(moves.size() > max_moves && moves.size() > 200) {
			use_fallback = true;
			break;
		}
		#endif
		for(move_t& mv : moves) {
			#ifdef DEBUG
			plateau tmp, tmp2;
			cpy_plateau(state.p, tmp);
			#endif
			
			// On applique le coup et on regarde son score
			appliquer_move(state, mv);
			float score_mv = score_state(state);
			
			#ifdef DEBUG
			printf("score %f for ", minscore);
			afficher_coup(mv);
			cpy_plateau(state.p, tmp2);
			#endif
			
			// On annule le coup qui vient d'etre joue
			annuler_move(state, mv);
			
			// On verifie que le plateau est reste le meme apres avoir appliquer + annuler le coup
			#ifdef DEBUG
			if(!cmp_plateau(tmp, state.p)) {
				printf("Something is wrong. Annuler n'a pas fonctionne correctement. \n");
				afficher_coup(mv);
				afficher_plateau(tmp, true, state.tour);
				printf("-\n");
				afficher_plateau(tmp2, true, state.tour);
				printf("-\n");
				afficher_plateau(state.p, true, state.tour);
			}
			#endif
			
			if(score_mv > max_score) {
				max_score = score_mv;
				best_move = mv;
			}
			mv_count++;
		}
		
		// Si on a bien itere sur des coups..
		// A noter que cela devrait toujours etre vrai.
		if(best_move.start != -1) {
			// On joue le coup d'un point de vue de l'API
			jouer_coup(state, best_move);
			
			// On applique aussi le coup sur notre structure de donnee interne
			appliquer_move(state, best_move);
			
			// On enleve les PA necessaires
			total -= best_move.cout;
			
			// On affiche differentes informations pratique
			printf("%d coups analyse\n", mv_count);
			coups += mv_count;
			printf("score - %f ", max_score);
			afficher_coup(best_move);
			
			// On mets le temps a jour
			time2 = std::clock();
		} else {
			break;
		}
		printf("---\n");
	
	// On verifie que ce que l'on vient de jouer n'est pas "ne rien faire".
	// On verifie qu'il nous reste des PA
	// On verifie qu'il nous reste plus de 150 ms
	} while((best_move.flags & NULLMOVE_FLAG) == 0 && total > 0 && (1000*(time2-time)/CLOCKS_PER_SEC) < 850);
	
	// Si jamais on a du faire un fallback, pour une raison de temps, on applique l'algorithme secondaire.
	if(use_fallback) {
		// On genere les mini coups pour les agents et on applique le meilleur sequentiellement
		for(int i = 0 ; i < 4 ; i++) {
			// Meme idee que pour la boucle principale, simplement avec une fnction de generation de coup un peu different.
			// Aussi, on effectue les mouvement sequentiellement (agent 0 avant agent 1)
			if(state.agents_PA[i] == 8 && !est_capturable(aliens_pos[state.agents_moi[i]], state.tour, state.cap[state.agents_moi[i]])) {
				std::vector<move_t> moves = gen_fallback_moves(state, i);
				float max_score = -10000;
				
				move_t best_move;
				best_move.start = -1;
				// recherche du meilleur coup..
				
				for(move_t& mv : moves) {
					appliquer_move(state, mv);
					float minscore = score_state(state);
					
					#ifdef DEBUG
					printf("score %f for ", minscore);
					afficher_coup(mv);
					#endif
					
					annuler_move(state, mv);
					
					if(minscore > max_score) {
						max_score = minscore;
						best_move = mv;
					}
				}
				
				// application du dis coup..
				if(best_move.start != -1) {
					jouer_coup(state, best_move);
					appliquer_move(state, best_move);
					printf("fallback move -- score - %f ", max_score);
					afficher_coup(best_move);
				} else {
					break;
				}
			}
		}
	}
	
	// On affiche le temps pris au total, parce que c'est pratique.
	std::cout << 1000*(std::clock()-time)/CLOCKS_PER_SEC << "ms taken, so thats " << (1000*(std::clock()-time)/CLOCKS_PER_SEC) / (float)(coups) << "ms per eval" << std::endl;
}

/// Fonction appelée à la fin de la partie.
void partie_fin()
{

}

int main() {
	partie_init();
	return 0;
}
