#include <iostream>
#include <cmath>
#include <GL/glut.h>

#define PI 3.1415926535898f	// Définition d'une valeur de approchée de PI
#define PHI 1.61803398875f	// Définition d'une valeur apporchée de PHI

/* ------------------------------------------------------------------- */
/*		Les variables globales de la classe		       */
/* ------------------------------------------------------------------- */

char presse;						// Le caractère lu au clavier dans la gestion des entrées clavier

GLfloat minx {-6.0f}, miny {-3.0f}, minz {-6.0f};	// Les paramètres min de la fenêtre de vue ...
GLfloat maxx {+6.0f}, maxy {+9.0f}, maxz {+6.0f};	// Les paramètres max de la fenêtre de vue ...

GLint xold, yold, anglex, angley;			// Les contrôles de position de la souris

GLfloat boule[3][4][4]
{
	/* boule Base */	 
	{ 0.0f, 0.0f, 0.0f, 0.0f,		/* -------------------------------------------------------- */
	  1.0f, 1.0f, 1.0f, 1.0f,		/*							    */
 	  0.0f, 0.0f, 0.0f, 60.0f,		/*	J'utilise des matrices 4x4 pour stocker		    */
	  1.0f, 0.0f, 0.0f, 0.0f },		/* 	les paramètres de chaque élément de la pile GL 	    */
	/* boule Inter */		  	/*	selon un schema commun :			    */
	{ 0.0f, 0.0f, 0.0f, 0.0f,		/*	première ligne 	: les paramètres de glRotatef	    */
	  1.0f, 1.0f, 1.0f, 0.7f,		/*	seconde ligne	: les paramètres de glScalef	    */
 	  0.0f, 0.0f, 0.0f, 36.0f,		/*			  et le rayon de la sphère	    */
	  0.0f, 1.0f, 0.0f, 0.0f },		/*	troisième ligne	: les paramètres de glTranslatef    */	
	/* boule Tête */			/*		et le nombre de méridiens & tropiques	    */
	{ 0.0f, 0.0f, 0.0f, 0.0f,		/*	quatrième ligne	: les paramètres de glColor3f	    */
	  1.0f, 1.0f, 1.0f, 0.5f,		/*			  et le gap avec la précedente	    */
	  0.0f, 0.0f, 0.0f, 24.0f,		/*						  	    */
	  0.0f, 0.0f, 1.0f, 0.0f }		/* -------------------------------------------------------- */
};
					  							
const GLfloat sFactor {1.0151515f},		// Le facteur de scale
	      sFactorMax {PHI},			// La limite max qui crée le décrochage
	      sFactorMin {PHI-1.0f},		// La limite min qui bloque l'écrasement
	      deltaY {0.1f},			// L'incrément de translation qui remplace le SS en cas de décrochage
	      deltaZ {0.25f};			// l'incrément de zoom de la fenêtre de vue

GLfloat angleBI {0.0f}, angleIT {0.0f};		// Les angles de rotation des boules les unes par rapport aux autres

/* ------------------------------------------------------------------- */
/*			Prototype des fonctions		   	       */
/* ------------------------------------------------------------------- */

void drawBoule (int i);			//----------------------- dessin d'une boule
void bonHomme();			//----------------------- structure du bonhomme

void idle();				//----------------------- IDLE
void reshape(int width, int height);	//----------------------- Redimensionnement

void zoomIO(GLfloat d);			//----------------------- Zoom
GLfloat sqr(GLfloat x);			//----------------------- fonction carré (pour AL Kashi)
GLfloat getAlKashiAngle();		//----------------------- Fonction de calcul d'angle pour éviter le chevauchement
GLfloat getGap(int i, int j);		//----------------------- Renvoie l'espace en y entre deux boules
GLfloat getCenterDist(int i, int j);	//----------------------- Renvoie la distance entre deux centres de scènes
GLfloat getSSRadius(int i);		//----------------------- Renvoie le rayon étiré en y d'une boule
void updateSSParams(int i, GLfloat f);		//--------------- Met à jour les paramètres Squash & Stretch
bool isBottomConnected(int i);		//----------------------- Dit si une boule est connectée à celle du dessous
bool isTopConnected(int i);		//----------------------- Dit si une boule est connectée à celle du dessus
void reScaleAtOne(int i);		//----------------------- Remet les coefficients d'échelle à 1 pour une boule
void clavier(unsigned char touche, int x, int y);	//------- Gère les touches classiques du clavier
void specialkeys(int key, int x, int y);	//--------------- Gère les touches spéciales du clavier
void mouse(int button, int state, int x, int y);	//------- Gère les clics souris
void mousemotion(int x, int y);		//----------------------- Gère les mouvements de la souris

void affichage();		//------------------------------- Gère l'affichage de la scène


/* ------------------------------------------------------------------- */
/*			Fonctions de Tracé			       */
/* ------------------------------------------------------------------- */

void drawBoule (int i) 	//----------------------------------- Le tracé d'une boule
{
	glPushMatrix();
		glColor3f	(boule[i][3][0], boule[i][3][1], boule[i][3][2]); 			// Sa couleur
		glTranslatef	(boule[i][2][0], boule[i][2][1], boule[i][2][2]);			// Ses translations
		glScalef	(boule[i][1][0], boule[i][1][1], boule[i][1][2]);			// Ses échelles
		glRotatef	(boule[i][0][0], boule[i][0][1], boule[i][0][2], boule[i][0][3]);	// Sa rotation propre
		glutSolidSphere	(boule[i][1][3], round(boule[i][2][3]), round(boule[i][2][3]));		// L'objet lui-même
	glPopMatrix();
}

void bonHomme() 	//--------------------------------------- La structure OpenGL de la scène du bonHomme
{
	glTranslatef(0.0f, getSSRadius(0), 0.0f);		//--- Translation de l'ensemble de la scène pour se poser en O
	glPushMatrix();
		glRotatef(angleBI, 0.0f, 0.0f, 1.0f);		//--- Préparer la rotation de la scene \ premier centre
		drawBoule(0);					//--- Tracé de la boule 0 au premier centre
		glTranslatef(0.0f, getCenterDist(0,1), 0.0f);	//--- Translation vers la sous-scène (boules 1 & 2)
		glPushMatrix();
			glRotatef(angleIT, 0.0f, 0.0f, 1.0f);	//--- Préparer la rotation de la sous-scène \ second centre
			drawBoule(1);				//--- Tracé de la boule 1 au second centre
			glTranslatef(0.0f, getCenterDist(1,2), 0.0f);	//--- Translation vers la sous-sous-scène (boule 2)
			glPushMatrix();
				drawBoule(2);				//--- Tracé de la boule 2
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}


/* ------------------------------------------------------------------- */
/*			Fonction IDLE				       */
/* ------------------------------------------------------------------- */

void idle()	//------------------------------------------- IDLE
{
	if (true) {	
		glutPostRedisplay();
	}
}


/* ------------------------------------------------------------------- */
/*			Fonction de Redimensionnement		       */
/* ------------------------------------------------------------------- */

void reshape(int width, int height)	//------------------- Redimensionnement de la fenêtre
{
	if (width < height) {
		glViewport(0, (height - width) / 2, width, width);	
	}
	else {
		glViewport((width - height)/2, 0, height, height);	
	}
}


/* ------------------------------------------------------------------- */
/*			Fonctions de Gestion du clavier		       */
/* ------------------------------------------------------------------- */

void zoomIO(GLfloat d)	//----------------------------------- Mise à jour des paramètres de zoom
{
	minx += d;	maxx -= d;
	miny += d;	maxy -= d;
	minz += d;	maxz -= d;
	glOrtho(minx, maxx, miny, maxy, minz, maxz);
}

GLfloat sqr(GLfloat x)	//----------------------------------- Une fonction carré
{	
	return x*x;
}

GLfloat getAlKashiAngle() 	//--------------------------- Une limitation de l'angle de rotation de 2 autour de 1
{
	GLfloat r0 = boule[0][1][3]*boule[0][1][0];
	GLfloat r1 = boule[1][1][3]*boule[1][1][0];
	GLfloat r2 = boule[2][1][3]*boule[2][1][0];
	return 180.0f-(acos((sqr(r0+r2)-sqr(r0+r1)-sqr(r1+r2))/(-2.0f*(r0+r1)*(r1+r2)))*180.0f/PI);
}

GLfloat getGap(int i, int j)	//--------------------------- Renvoie la mesure de l'espace entre deux boules
{
	if (i<j)
		return boule[j][3][3];
	else
		return boule[i][3][3];
}

GLfloat getCenterDist(int i, int j)	//------------------- Renvoie la distance entre deux centres de scènes
{
	if (i!=j)
		return getSSRadius(i)+getSSRadius(j)+getGap(i,j);
	else 
		return 0.0f;
}

GLfloat getSSRadius(int i)	//--------------------------- Renvoie le rayon déformé de la boule i
{
	return (boule[i][1][3] * boule[i][1][1]);
}

void updateSSParams(int i, GLfloat f)	//------------------- Modifie les paramètres Stretch & Squash de la boule i
{
	boule[i][1][1] *= f;			// on multiplie le facteur en y par f
	boule[i][1][0] /= sqrt(f);		// on ajuste le facteur en x (conservation du volume)
	boule[i][1][2] /= sqrt(f);		// on ajuste le facteur en z (conservation du volume)
}

bool isBottomConnected(int i)	//--------------------------- Dit si la boule i est attachée par le bas
{
	if ((i >= 0) && (i <= 2))
		return ((boule[i][3][3])==0.0f);	// c à d si le gap est à zéro ...
	else
		return false;
}

bool isTopConnected(int i)	//--------------------------- Dit si la boule i est attachée par le haut
{
	if ((i >= 0) && (i < 2))
		return ((boule[i+1][3][3])==0.0f);	// c à d si le gap de la suivante est à zéro
	else
		return false;
}

void reScaleAtOne(int i)	//--------------------------- Ramène les facteur de scale de la boule i à 1
{
	for (int j=0; j<3; j++)	boule[i][1][j] = 1.0f;
}

void clavier(unsigned char touche, int x, int y)	//--- Gestion des saisies clavier
{
	switch(touche) {
	
		/* Quitter */
		case 'q': 
			exit(0);
			
		/* affichage en mode plein */
		case 'p': 
      		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      		break;
      		
    		/* affichage en mode fil de fer */
    		case 'f': 
      		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      		break;
      		
      		/* Affichage en mode sommets seuls */	
    		case 's' : 
      		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
     		break;
     		
		/* Zoomer */
		case 'Z': 
			zoomIO(deltaZ);
			break;
			
		/* Dézoomer */
		case 'z': 
			zoomIO(-deltaZ);
			break;
		
		/* Etirement en y de la tête du bonHomme */
		case 'T':
			if (isBottomConnected(2)) {				//--- si la tête est connectée au tronc
				if (boule[2][1][1] < sFactorMax) {		//--- et si max n'est pas atteint
					GLfloat old_Pos = getSSRadius(2);	//--- on stocke le rayon déformé actuel	
					updateSSParams(2, sFactor);		//--- on augmente l'étirement
					boule[2][2][1] = getSSRadius(2) - old_Pos; // on ajuste la translation en y
				}
				else {							// sinon (max est atteint)
					boule[2][3][3] += getSSRadius(2)-boule[2][1][3];// on détache la tête du tronc
					reScaleAtOne(2);				// on réinitialise le scale de la boule
				}
			}
			else {					//------- sinon (la tête est détachée)
				boule[2][3][3] += deltaY;	//------- on transforme l'étirement en translation
			}
			break;
		
		/* Ecrasement en y de la tête du bonHomme */
		case 't':
			if (isBottomConnected(2)) {				//--- si la tête est connectée au tronc
				if (boule[2][1][1] > sFactorMin) {		//--- et si le facteur min n'est pas atteint
					GLfloat old_Pos = getSSRadius(2);	//--- on stocke le rayon déformé actuel	
					updateSSParams(2, 1/sFactor);		//--- on augmente l'écrasement
					boule[2][2][1] = (old_Pos-getSSRadius(2)); // on ajuste la translation en y
				}
			}
			else {							//--- sinon (la tête est détachée)
				if (boule[2][3][3] > deltaY) { 			//--- si la distance tête-tronc est suffisante
					boule[2][3][3] -= deltaY;		//--- on transforme l'écrasement en translation
				}
				else {						//--- sinon (distance insuffisante pour translater)
					boule[2][3][3] = 0.0f;			//--- on reconnecte la tête au tronc
					boule[2][2][1] = 0.0f;			//--- on réinitialise la translation
				}
			}
			break;

		/* Etirement en y de la partie intermédiaire du bonHomme */
		case 'I':
			if (isBottomConnected(1)) {				//---si le tronc est connecté à la base
				if (boule[1][1][1] < sFactorMax) {		//--- 	et si le facteur max n'est pas atteint
					GLfloat old_Pos = getSSRadius(1);	//--- on stocke la position
					updateSSParams(1, sFactor);		//--- on augmente l'étirement
					if (!isTopConnected(1)) {		//---	si le tronc n'est pas attaché à la tête
						boule[2][3][3] -= 2*(getSSRadius(1)-old_Pos);	// on compense en rapprochant la tête
						if (boule[2][3][3] <= 0.0f) {			//--- si on rattrape la tête
							boule[2][3][3] = 0.0f;			//--- 	on s'y reconnecte
						}						
					}
				}
				else {							//--- 	sinon (le max est atteint)
					boule[1][3][3] += 2*(getSSRadius(1)-boule[1][1][3]);	// on détache le tronc de la base
					reScaleAtOne(1);					//--- on réinitialise le scale de la boule
				}
			}
			else {							//--- sinon (le tronc n'est pas connecté à la base)
				boule[1][3][3] += deltaY;			//--- on transforme l'étirement en translation
				if (!isTopConnected(1)) {			//--- si le tronc n'est pas attaché à la tête
					boule[2][3][3] -= deltaY;		//--- on compense en rapprochant la tête
					if (boule[2][3][3] <= 0.0f) {		//--- si on rattrape la tête
						boule[2][3][3] = 0.0f;		//--- on s'y reconnecte
					}						
				}
			}
			break;
		
		/* Ecrasement en y de la partie intermédiaire du bonHomme */
		case 'i':
			if (isBottomConnected(1)) {				//------- si le tronc est connecté à la base
				if (boule[1][1][1] > sFactorMin) {		//--- 	et si le facteur min n'est pas atteint
					GLfloat old_Pos = getSSRadius(1);	//--- on stocke la position
					updateSSParams(1, 1/sFactor);		//--- on augmente l'écrasement
					if (!isTopConnected(1)) {		//--- si le tronc n'est pas attaché à la tête
						boule[2][3][3] += 2*(old_Pos-getSSRadius(1));	// on compense en éloignant la tête
					}
				}
			}
			else {						//------- sinon (le tronc n'est pas attaché à la base)
				boule[1][3][3] -= deltaY;		//--- 	on transforme l'écrasement en translation
				if (!isTopConnected(1)) {		//--- 	si le tronc n'est pas connecté à la tête
					boule[2][3][3] += deltaY;	//--- on ajuste sa translation
				}
				if (boule[1][3][3] <= 0.0f) {		//--- 	si on rattrape la base
					boule[1][3][3] = 0.0f;		//--- on s'y reconnecte
				}
			}
			break;
			
		/* Etirement en y de la base du bonHomme */
		case 'B':
			if (boule[0][1][1] < sFactorMax) {		//------- si le facteur max n'est pas atteint
				GLfloat old_Pos = getSSRadius(0);	//--- 	on stocke la position
				updateSSParams(0, sFactor);		//--- 	on augmente l'étirement
				if (!isTopConnected(0)) {		//---	si la base n'est pas connectée au tronc 
					boule[1][3][3] -= 2*(getSSRadius(0)-old_Pos);	// on ajuste la translation du tronc
				}
				if (boule[1][3][3] <= 0.0f) {		//--- 	si on rattrape le tronc
					boule[1][3][3] = 0.0f;		//--- on s'y reconnecte
				}
				if (isTopConnected(0)&!isTopConnected(1)) {	//	si seule la tête n'est pas connectée
					boule[2][3][3] -= 2*(getSSRadius(0)-old_Pos);	// on ajuste sa translation
				}
				if (boule[2][3][3] <= 0.0f) {		//---	si on rattrape la tête
					boule[2][3][3] = 0.0f;		//--- on la reconnecte au tronc
				}
			}
			break;
		
		/* Ecrasement en y de la base du bonHomme */
		case 'b':
			if (boule[0][1][1] > sFactorMin) {		//------- si le facteur min n'est pas atteint
				GLfloat old_Pos = getSSRadius(0);	//--- 	on stocke la position
				updateSSParams(0, 1/sFactor);		//--- 	on augmente l'écrasement
				if (!isTopConnected(0)) {		//--- 	si la base n'est pas connectée au tronc
					boule[1][3][3] += 2*(old_Pos-getSSRadius(0));	// on ajuste la translation du tronc;
				}
				if (isTopConnected(0)&&!isTopConnected(1)) {	// si seule la tête n'est pas connectée
					boule[2][3][3] += 2*(old_Pos-getSSRadius(0));	// on ajuste sa translation
				}
			}
			break;
								
		default :
			break;
	}
	glutPostRedisplay();
}

void specialkeys(int key, int x, int y)	//------------------- Gestion des touches spéciales du clavier
{
	switch (key) {
		
		case GLUT_KEY_LEFT :		//----------------------- Rotation de 1 & 2 autour de 0
			angleBI += 1.0f;
			break;
		
		case GLUT_KEY_RIGHT :		//----------------------- Rotation de 1 & 2 autour de 0 (autre sens)
			angleBI -= 1.0f;
			break;
			
		case GLUT_KEY_DOWN :		//----------------------- Rotation de 2 autour de 1
			if ((angleIT+1.0f)<getAlKashiAngle()) angleIT += 1.0f;
			break;
		
		case GLUT_KEY_UP :		//----------------------- Rotation de 2 autour de 1 (autre sens)
			if ((angleIT-1.0f)>-getAlKashiAngle()) angleIT -= 1.0f;
			break;
			
		default :
			break;
	}
	glutPostRedisplay();
}

/* ------------------------------------------------------------------- */
/*		Fonctions de Gestion de la Souris		       */
/* ------------------------------------------------------------------- */

void mouse(int button, int state, int x, int y)	//----------- Gestion du clic de la souris
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		presse = 1;
		xold = x;
		yold = y;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		presse = 0;
}

void mousemotion(int x, int y)	//--------------------------- Gestion du mouvement de la souris
{
	if (presse) {
		anglex = anglex + (x - xold);
		angley = angley + (y - yold);
		glutPostRedisplay();
	}
	xold = x;
	yold = y;
}

/* ------------------------------------------------------------------- */
/*		Fonctions display d'OpenGL & de lancement	       */
/* ------------------------------------------------------------------- */

void affichage()	//--------------------------------------- Gestion de l'affichage
{
	/* Effacement de l'image avec la couleur de fond */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/* Objets à afficher */
	bonHomme();	
	glLoadIdentity();
	glRotatef(anglex, 1.0f, 0.0f, 0.0f);
	glRotatef(angley, 0.0f, 0.0f, 1.0f);
	
	/* Recadrage de l'espace de vue */
	glOrtho(minx, maxx, miny, maxy, minz, maxz);
	glFlush();
	glutSwapBuffers();
}

int main(int argc, char** argv)		//----------------------- Fonction de lancement de l'application
{
	/* Initialisation de glut et creation de la fenêtre */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("The Squashed & Stretched Snowman");

	/* Initialisation des styles */
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glPointSize(3.0f);
	glLineWidth(2.0f);
	glEnable(GL_DEPTH_TEST);

	/* Enregistrement des fonctions de rappel */
	glutDisplayFunc(affichage);
	glutKeyboardFunc(clavier);
	glutSpecialFunc(specialkeys);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemotion);
	glutIdleFunc(idle);

	/* Entree dans la boucle principale glut */
	glutMainLoop();
	return 0;
}
