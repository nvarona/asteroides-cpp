// .....###....##.....##.####..#######..##....##.########..######...
// ....##.##...##.....##..##..##.....##.###...##.##.......##....##..
// ...##...##..##.....##..##..##.....##.####..##.##.......##........
// ..##.....##.##.....##..##..##.....##.##.##.##.######....######...
// ..#########..##...##...##..##.....##.##..####.##.............##..
// ..##.....##...##.##....##..##.....##.##...###.##.......##....##..
// ..##.....##....###....####..#######..##....##.########..######...

// Introducción a C++, Natxo Varona
// Pruebas 2023-2024:
// Pruebas de un juego 2D en ASCII de Aviones

#include <stdio.h>
#include <stdlib.h>
#include <list>			// Para crear Listas dinamicas
#include <sys/ioctl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h> 		// Para nanosleep
#include <time.h>   		// Para timespec
#include <ncurses.h>

using namespace std;

#define ESC 27

// Función para agregar un retraso de 30 milisegundos
void sleep30ms(){
   struct timespec delay;
   delay.tv_sec = 0;
   delay.tv_nsec = 30000000; // 30 ms
   nanosleep(&delay, NULL);
}

// Creamos la Clase de Avion donde creamos los metodos y constructor de nuestra nave
// Vamos a retornar los valores de X, Y para poder controlar las colisiones
class Avion {
	int posX, posY;
	int corazones;
	int vidas;
public:
	Avion(int _posX, int _posY, int _corazones, int _vidas): posX(_posX), posY(_posY), corazones(_corazones), vidas(_vidas) {}
	int X() { return posX; }
	int Y() { return posY; }
	int V() { return vidas; }
	void HEART() { corazones--; }
	void pintar();
	void borrar();
	void mover();
	void pintar_corazones();
	void pintar_vidas();
	void morir();
};

void Avion::pintar() {
    mvprintw(posY, posX, "  %c", ACS_UARROW);
    mvprintw(posY+1, posX, " (%c)", ACS_DIAMOND);
    mvprintw(posY+2, posX, "%c%c %c%c", ACS_UARROW, ACS_CKBOARD, ACS_CKBOARD, ACS_UARROW);
    refresh();  // Refresca la pantalla
}

void Avion::borrar() {			//  12345678
	move(posY, posX);		printw("       ");
	move(posY+1, posX);		printw("       ");
	move(posY+2, posX);		printw("       ");
	refresh();	// Refresca la pantalla
}

void Avion::mover() {
	int tecla = getch();
	borrar();
	if ((tecla == KEY_UP || tecla == 'w') && posY > 4) posY--;			// Arriba	
	if ((tecla == KEY_DOWN || tecla == 's') && posY+3 < 34) posY++;		// Abajo
	if ((tecla == KEY_LEFT || tecla == 'k') && posX > 3) posX--;		// Izquierda
	if ((tecla == KEY_RIGHT || tecla == 'l') && posX+6 < 79) posX++; 	// Derecha
	if (tecla == 'e' && corazones > 1) corazones--;						// Prueba para saber que se estan borrando corazones
	pintar();
	pintar_corazones();
	pintar_vidas();
	refresh();	// Refresca la pantalla
}

void Avion::pintar_corazones(){
	move(2, 64); printw("Salud:");
	move(2, 71); printw("      ");
	for (int i = 0;i < corazones; i++){
		mvprintw(2, 71 + i, "%c", ACS_HLINE);
	}
}

void Avion::pintar_vidas(){
	move(2, 50); printw("Vidas: %d", vidas);
}

void Avion::morir(){
	if (corazones == 0){
		borrar();
		move(posY, posX);	printw("   **   ");
		move(posY+1, posX);	printw("  ****  ");
		move(posY+2, posX);	printw("   **   ");
		sleep30ms();
		move(posY, posX);	printw(" * ** * ");
		move(posY+1, posX);	printw("   **   ");
		move(posY+2, posX);	printw(" * ** * ");
		sleep30ms();			//  12345678
		borrar();
		vidas--;
		corazones = 3;
		pintar_corazones();
		pintar_vidas();
		pintar();
		// Sonido de una exploxion !!!
	}
}

class Balas{
	int posX, posY;
public:
	Balas(int _posX, int _posY) : posX(_posX), posY(_posY){}
	int X() { return posX; }
	int Y() { return posY; }
	void mover();
	bool fuera();
};

void Balas::mover(){
	move(posY, posX); printw(" ");
	if (posY > 4) posY--;
	move(posY, posX); printw("|");
}

bool Balas::fuera(){
	if (posY == 4) return true;
	return false;
}

class Asteroide{
	int posX, posY;
public:
	Asteroide(int _posX, int _posY) : posX(_posX), posY(_posY){}
	int X() { return posX; }
	int Y() { return posY; }
	void pintar();
	void mover();
	void colision(class Avion &Avo);
};

void Asteroide::colision(class Avion &Avo){
	if(posX >= Avo.X() && posX < Avo.X()+6 && posY >= Avo.Y() && posY <= Avo.Y()+2){
		Avo.HEART();
		Avo.borrar();
		Avo.pintar();
		Avo.pintar_corazones();
		Avo.pintar_vidas();
		posX = rand()%71 + 4;		// para que salga un valor dentro de los limites.
		posY = 4;
	}
}

void Asteroide::pintar(){
	move(posY, posX);	printw("%c", 184);
}

void Asteroide::mover(){
	move(posY, posX);	printw(" ");
	posY++;
	if (posY > 32){
		posX = rand()%71 + 4;		// para que salga un valor dentro de los limites.
		posY = 4;
	}
	pintar();
}

// Funcion que nos pinta el cuadro de juego y los limites en la pantalla
void pintarLimite() {
  	// Líneas horizontales
  	for (int i = 3; i <= 33; i++) {
  		attron(COLOR_PAIR(1));
    	mvprintw(i, 2, "|");
    	mvprintw(i, 78, "|");
    	attroff(COLOR_PAIR(1));
  	}

  	// Líneas verticales
  	for (int i = 2; i <= 78; i++) {
  		attron(COLOR_PAIR(1));
    	mvprintw(3, i, "-");
    	mvprintw(34, i, "-");
    	attroff(COLOR_PAIR(1));
  	}

  	// Esquinas
  	attron(COLOR_PAIR(2));
  	mvprintw(3, 2, "+");
  	mvprintw(34, 2, "+");
  	mvprintw(3, 78, "+");
  	mvprintw(34, 78, "+");
  	attroff(COLOR_PAIR(2));
}

// Funcion principal del proyecto de AVIONES en ASCII
int main(){
	// Inicializar la pantalla de ncurses
	int max_x, max_y, pos_x, pos_y;
	clear();
  	initscr();
  	keypad(stdscr, TRUE);			// Configurar para leer teclas de flechas
  	raw();
  	cbreak(); 					// Deshabilita el modo de línea
  	noecho(); 					// Deshabilita el eco de caracteres
  	nonl();
    timeout(0);					// Habilitar el reconocimiento de tecla de escape
    curs_set(0);				// Ocultar el cursor
    erase();
    getmaxyx(stdscr, max_y, max_x);
    pos_x = max_x / 2;
  	pos_y = max_y / 2;

    // Inicializar colores para los limites
  	start_color();
  	init_pair(1, COLOR_WHITE, COLOR_BLACK);
  	init_pair(2, COLOR_RED, COLOR_BLACK);

  	// creamos una lista dinamica para poder almacenar los punteros de los objetos Balas.
  	list<Balas*> Bal;
  	list<Balas*>::iterator it; 

  	list<Asteroide*> Ast;
  	list<Asteroide*>::iterator itAst;
  	for (int i=0;i<5;i++){
  		Ast.push_back(new Asteroide(rand()%75 + 3, rand()%5 + 4));
  	}

    // Creamos el objecto contructor Avion, le ponemos un nombre "Avo"
    // Y con los parametros para los metodos
    // Posición inicial del Avion (X,Y), Corazones y Vidas
    Avion Avo(15, 10, 3, 5);
    Avo.pintar();
    Avo.pintar_corazones();
    Avo.pintar_vidas();

    // Pintar los límites del juego
  	pintarLimite();

	bool game_over = false;
	int puntos = 0;
	while(!game_over){
		move(2,4);	printw("Puntos: %d", puntos);
		// Miramos que hemos pulsado una tecla
		// int tecla = getch();
		//if (tecla == 'b' || tecla == KEY_SPACE ){
			Bal.push_back(new Balas(Avo.X() + 2, Avo.Y() - 1));
			// Sonido de disparo de la bala
			//disparo();
		//}
		//if (tecla == 'q' || tecla == ESC ){
		//		game_over = true;
		//}

		// Recorremos la lista de balas para moverlas y verificar si han llegado al limite
		for (it = Bal.begin(); it != Bal.end(); it++){
			(*it)->mover(); 
			if ((*it)->fuera()){
				move((*it)->Y(),(*it)->X());	printw(" ");
				delete(*it);
				it = Bal.erase(it);
			}
		}
		// Recorremos la lista de Asteroides para moverlos y verificar las colisiones
		for (itAst = Ast.begin(); itAst != Ast.end(); itAst++){
			(*itAst)->mover(); 
			(*itAst)->colision(Avo); 
		}
		// Recorremos las listas de Asteroides, Balas y verificar las colisiones entre ellas !!!
		for (itAst = Ast.begin(); itAst != Ast.end(); itAst++){
			for (it = Bal.begin(); it != Bal.end(); it++){
				if ((*itAst)->X() == (*it)->X() && (*itAst)->Y() + 1 == (*it)->Y() || (*itAst)->Y() == (*it)->Y()){
					move((*it)->Y(),(*it)->X());	printw(" ");
					delete(*it);
					it = Bal.erase(it);
					Ast.push_back(new Asteroide(rand()%74 + 3, rand()%5 + 4));
					move((*itAst)->Y(),(*itAst)->X());	printw(" ");
					delete(*itAst);
					itAst = Ast.erase(itAst);
					// Incrementos de 5 puntos si tenemos una colision
					puntos+=5;					
				}
			} 
		}
		if (Avo.V() <= 0){
			game_over = true;
			// Hacer musica de que se acabo el juego ...
			move(18, 25);
			printw("Game OVER ;-((");
		}
		Avo.morir();
		Avo.mover();
    	// Agregar un retraso de 30 ms
    	sleep30ms();
	}
    curs_set(1);		// Mostrar el cursor nuevamente
  	endwin();			// Terminar la configuracion de la terminal por ncurses
  	getch();
  	return 0;
}