#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

// Maria Eugenia Reis da Fonseca

const float FPS = 100;

const int SCREEN_W = 960;
const int SCREEN_H = 540;

const int NAVE_W = 24;
const int NAVE_H = 34;
const int VELOCIDADE_NAVE = 2;

#define NUM_INIMIGOS 1500
#define VELOCIDADE_CIR 2
#define RAIOMAXIMO 1

#define NUM_BALAS 1
#define RAIOESPECIAL 10

#define NUM_ESTRELAS 300

ALLEGRO_COLOR COR_CENARIO;
BITMAP *buffer;

//_______________________________________________________________________

//------------------------------OBJETOS----------------------------------


typedef struct Nave
{
	int x, y;
	int vel;
	int dir_x, dir_y;
	ALLEGRO_COLOR cor;
} Nave;

typedef struct Bloco
{
	int x, y;
	int w, h;
	bool ativo;
	ALLEGRO_COLOR cor;
} Bloco;

typedef struct Ponto
{
	int x, y;
} Ponto;

typedef struct Inimigo
{
	Ponto centro;
	int raio;
	int vel;
	bool ativo;
	ALLEGRO_COLOR cor;
} Inimigo;

typedef struct Balas
{
	Ponto centro;
	float raio;
	int vel;
	bool atirou;
	ALLEGRO_COLOR cor;
} Balas;


//__________________________________________________________________________

//-------------------------ASSINATURAS DE FUNCOES----------------------------

int colisaoBlocos(Bloco a, Bloco b);

int colisaoInimigos(Inimigo *c1, Inimigo *c2);

int colisaoInimigosAtivos(int i, Inimigo inimigo[]);

float distancia(Ponto p1, Ponto p2);

//___________________________________________________________________________

//-------------------------DEFINICAO DE FUNCOES------------------------------

//------------RECORDE------------

int novoRecorde(int pontuacao, int *recorde)
{
	FILE *arq = fopen("recorde.txt", "r");
	*recorde = -1;
	if (arq != NULL)
	{
		fscanf(arq, "%d", recorde);
		fclose(arq);
	}
	if (*recorde < pontuacao)
	{
		arq = fopen("recorde.txt", "w");
		fprintf(arq, "%d", pontuacao);
		fclose(arq);
		return 1;
	}
	return 0;
}

//--------FUNCOES RAND---------

float randf()
{
	return (float)rand() / RAND_MAX; // como o maior valor possivel gerado por rand eh 32767, o maior valor possivel
} //  obtido nessa divisao eh 1

float randFloat(float min, float max)
{
	return min + randf() * (max - min);
}

//---------TRANSFORMACOES-----------

void trNaveEmBloco(Bloco *ret, Nave nave)
{
	ret->h = NAVE_H;
	ret->w = NAVE_W;
	ret->x = nave.x - NAVE_W;
	ret->y = nave.y - NAVE_H / 2;
	ret->cor = nave.cor;
	ret->ativo = true;
}

void trInimigoEmBloco(Bloco *ret, Inimigo *c)
{
	ret->ativo = c->ativo;
	ret->h = c->raio + c->raio;
	ret->w = c->raio + c->raio;
	ret->x = c->centro.x - c->raio;
	ret->y = c->centro.y - c->raio;
}

void trInimigosEmBloco(Bloco ret[], int tamanho, Inimigo c[])
{
	for (int i = 0; i < tamanho; i++)
	{
		trInimigoEmBloco(ret + i, c + i);
	}
}

// *p = ret;    ------> o endereco para o comeco do array
// ret[0] == *p
// ret[i] == p[i] == *(p + i)   ------> acessar o elemento na posicao i do array
// ret == &ret[0]
// &ret[i] == (ret + i)

void trBalaEmBloco(Bloco *ret, Balas *bala)
{
	ret->ativo = bala->atirou;
	ret->h = bala->raio + bala->raio;
	ret->w = bala->raio + bala->raio;
	ret->x = bala->centro.x - bala->raio;
	ret->y = bala->centro.y - bala->raio;
}

void trBalasEmBloco(Bloco ret[], int tamanho, Balas bala[])
{
	for (int i = 0; i < tamanho; i++)
	{
		trBalaEmBloco(ret + i, bala + i);
	}
}

//----------CENARIO-----------

void initGlobais()
{
	COR_CENARIO = al_map_rgb(0, 0, 0);
}

void desenhaCenario()
{
	al_clear_to_color(COR_CENARIO);
}

void estrelas()
{
	static int x[NUM_ESTRELAS] = {0};
	static int y[NUM_ESTRELAS] = {0};
	static int w[NUM_ESTRELAS] = {2};
	for (int i = 0; i < NUM_ESTRELAS; i++)
	{
		if (x[i]-- < 0)
		{
			x[i] = (rand() % SCREEN_W) + SCREEN_W;
			y[i] = rand() % SCREEN_H;
			w[i] = randFloat(0, 2);
		}
		else if (i % 2) // criando dois planos de estrelas se movimentando
			x[i] -= 5;
		else if (i % 3) // criando tres planos de estrelas se movimentando
			x[i] -= 10; 
		al_draw_filled_circle(x[i], y[i], w[i], al_map_rgb(255, 255, 255));
	}
}

//------------NAVE-------------

void initNave(Nave *nave)
{
	nave->x = 5 + NAVE_W; // um pequeno espaço para afastar da margem + o comprimento da nave
	nave->y = SCREEN_H / 2;
	nave->dir_x = 0;
	nave->dir_y = 0;
	nave->cor = al_map_rgb(255, 255, 255);
	nave->vel = VELOCIDADE_NAVE;
}

void desenhaNave(Nave nave)
{
	al_draw_filled_triangle(nave.x, nave.y, // recebe os 3 vertices do triangulo
							nave.x - NAVE_W, nave.y - NAVE_H / 2,
							nave.x - NAVE_W, nave.y + NAVE_H / 2,
							nave.cor);
	al_draw_filled_rectangle(nave.x + 2, nave.y + 2,
							 nave.x - NAVE_W, nave.y - 2,
							 al_map_rgb(0, 0, 100));
	al_draw_filled_rectangle(nave.x - 10, nave.y - 9,
							 nave.x - 1, nave.y - 7,
							 al_map_rgb(100, 0, 0));
	al_draw_filled_rectangle(nave.x - 10, nave.y + 9,
							 nave.x - 1, nave.y + 7,
							 al_map_rgb(100, 0, 0));
}

void atualizaNave(Nave *nave)
{
	nave->y += nave->dir_y * nave->vel;
	nave->x += nave->dir_x * nave->vel;
}

//------------BLOCOS-----------

void initBloco(Bloco *bloco)
{
	bloco->x = SCREEN_W + rand() % SCREEN_W;
	bloco->y = rand() % (4 * SCREEN_H / 5); // no minimo um quinto da tela
	bloco->w = SCREEN_W + rand() % SCREEN_W;
	bloco->h = SCREEN_H / 5 + rand() % (2 * SCREEN_H / 5); // no maximo tres quintos da tela
	bloco->cor = al_map_rgb(rand(), rand(), rand());
	bloco->ativo = true;
}

void desenhaBloco(Bloco bloco)
{
	al_draw_filled_rectangle(bloco.x, bloco.y,
							 bloco.x + bloco.w, bloco.y + bloco.h,
							 bloco.cor);
}

void atualizaBloco(Bloco *bloco)
{
	bloco->x -= 2;

	if (bloco->x + bloco->w < 0) // quando ultrapassar o limite da tela, cria-se um novo bloco
		initBloco(bloco);
}

//--------(INIMIGOS)----------

void initInimigo(Inimigo inimigo[], int tamanho)
{
	for (int i = 0; i < tamanho; i++)
	{
		inimigo[i].ativo = false;
		inimigo[i].vel = 2 + rand()%5;
	}
}

void inimigoRandom(Inimigo *iniRandom)
{
	iniRandom->raio = 20 + rand() % RAIOMAXIMO;
	iniRandom->centro.x = SCREEN_W + SCREEN_W;
	iniRandom->centro.y = randFloat(0 + iniRandom->raio, SCREEN_H - iniRandom->raio);
	iniRandom->cor = al_map_rgb(rand(), rand(), rand());
	iniRandom->ativo = true;
}

void soltaInimigo(Inimigo inimigo[], Bloco ret, int tamanho)
{
	Inimigo temp;
	Bloco blocoIni;

	int colidiu = 0;

	if (rand() % 50 != 0)
		return;

	inimigoRandom(&temp);
	trInimigoEmBloco(&blocoIni, &temp);

	colidiu = colisaoBlocos(blocoIni, ret);
	printf("\nA colisao %d", colidiu);

	for (int i = 0; i < tamanho && !colidiu; i++)
	{
		if (inimigo[i].ativo)
		{
			colidiu = colisaoInimigos(&inimigo[i], &temp);
		}
	}
	if (colidiu)
		return;
	for (int i = 0; i < tamanho && !colidiu; i++)
	{
		if (!inimigo[i].ativo)
		{
			inimigo[i].raio = temp.raio;
			inimigo[i].centro.x = temp.centro.x;
			inimigo[i].centro.y = temp.centro.y;
			inimigo[i].cor = temp.cor;
			inimigo[i].ativo = temp.ativo;
			break;
		}
	}
}

void atualizaInimigo(Inimigo inimigo[], int tamanho)
{
	for (int i = 0; i < tamanho; i++)
	{
		if (inimigo[i].ativo) // o inimigo inimigo esta na tela
		{
			inimigo[i].centro.x -= inimigo[i].vel;
			if (inimigo[i].centro.x + 50 < 0 || colisaoInimigosAtivos(i, inimigo))
			{
				inimigo[i].ativo = false;
			}
			
		}
	}
}

void atualizaPosicaoAleatoria(Inimigo inimigo[], int tamanho){     //caso o jogador pressione R os inimigos da tela mudam de posicao no eixo Y
	for (int i = 0; i < tamanho; i++)
	{
		if(inimigo[i].ativo)
		{
			inimigo[i].centro.y = randFloat(0 + inimigo[i].raio, SCREEN_H - inimigo[i].raio);
			if(inimigo[i].centro.x + 50 < 0 || colisaoInimigosAtivos(i, inimigo))
			{
				inimigo[i].ativo = false;
			}
		}
	}
}

void desenhaInimigo(Inimigo inimigo[], int tamanho)
{
	for (int i = 0; i < tamanho; i++)
	{
		if (inimigo[i].ativo)
		{
			al_draw_filled_circle(inimigo[i].centro.x, inimigo[i].centro.y, inimigo[i].raio, inimigo[i].cor);
		}
	}
}

//---------------BALAS----------------

void initBalas(Balas balas[], int tamanho)      //inicializa o array de NUM_BALAS
{
	for (int i = 0; i < tamanho; i++)
	{
		balas[i].raio = 3;
		balas[i].vel = 5;
		balas[i].atirou = false;
		
	}
}

void atiraBalas(Balas balas[], int tamanho, int carregandoTiro) // quando a bala estiver inativa ("morta")
{
	for (int i = 0; i < tamanho; i++)
	{
		if (!balas[i].atirou && carregandoTiro == 0) 
		{
			balas[i].atirou = true;
			break;
		}
	}
}

void atualizaBalasAtivas(Balas balas[], int tamanho) // quando a bala estiver ativa na tela
{
	for (int i = 0; i < tamanho; i++)
	{
		if (balas[i].atirou) 
		{
			balas[i].centro.x += balas[i].vel; // anda 5 pixels a cada quadro
			if (balas[i].centro.x > SCREEN_W)
			{ // assim que a bala ultrapassa W, a bala fica inativa e disponibiliza outra p ser usada
				balas[i].atirou = false;
				balas[i].raio = 3.0;
			}
		}
	}
}

void desenhaBalasAtivas(Balas balas[], int tamanho) // funcao para desenhar a bala a cada frame
{
	for (int i = 0; i < tamanho; i++)
	{
		if (balas[i].atirou)
		{
			al_draw_filled_circle(balas[i].centro.x, balas[i].centro.y, balas[i].raio, balas[i].cor);
		}
	}
}

void posicionaBalasCarregando(Balas balas[], Nave nave, int tamanho, int carregandoTiro)
{
	for (int i = 0; i < tamanho; i++)
	{
		if(!balas[i].atirou && carregandoTiro == 1)
		{
			balas[i].cor = al_map_rgb(255, 255, 0);
			balas[i].centro.x = nave.x + 15;
			balas[i].centro.y = nave.y;
			al_draw_filled_circle(balas[i].centro.x, balas[i].centro.y, balas[i].raio, balas[i].cor);
			break;
		}
	}
}

void atualizaBalasCarregando(Balas balas[], int tamanho, int carregandoTiro)
{ 
	for (int i = 0; i < tamanho; i++)
	{
		if(!balas[i].atirou && carregandoTiro == 1)
		{
			balas[i].cor = al_map_rgb(255, 255, 0); 
			while(carregandoTiro && balas[i].raio < RAIOESPECIAL)
			{
				balas[i].raio += 1.0/3.0;
				if(balas[i].raio > RAIOESPECIAL)
				{
					balas[i].raio = RAIOESPECIAL;
				}
				if(balas[i].raio  == RAIOESPECIAL)
				{
					carregandoTiro = 0;
				}	
				break;
			}

		}
	}
}

//_______________________________________________

//-----------------------FUNCOES DE COLISAO---------------------------

int colisaoBlocos(Bloco a, Bloco b)
{ // entre blocos
	if (a.ativo && b.ativo)
	{
		return ((a.x + a.w > b.x && a.x < b.x + b.w && a.y + a.h > b.y && a.y < b.y + b.h)) ||
			   ((b.x + b.w > a.x && b.x < a.x + a.w && b.y + b.h > a.y && b.y < a.y + a.h));
	}
	return 0;
}

float distancia(Ponto p1, Ponto p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

int colisaoInimigos(Inimigo *c1, Inimigo *c2)
{ // entre inimigos (inimigos)
	return (distancia(c1->centro, c2->centro) < c1->raio + c2->raio);
}

int colisaoInimigosAtivos(int i, Inimigo inimigo[])
{
		if(inimigo[i].ativo)
		{
			for (int j = 0; j < NUM_INIMIGOS; j++)
			{
				if (i != j && inimigo[j].ativo)
				{
					if (colisaoInimigos(inimigo + i, inimigo + j)){
						inimigo[j].ativo = false;
						return 1;
					}	
				}		
			}	
		}
	return 0;
}

int colisaoNaveBloco(Nave nave, Bloco bloco)
{ // entre a nave e um bloco
	Bloco blocoNave;
	trNaveEmBloco(&blocoNave, nave);
	return colisaoBlocos(blocoNave, bloco);
}

int colisaoNaveInimigos(Nave nave, Inimigo inimigo[])
{ // entre a nave e um inimigo
	Bloco blocoNave;
	Bloco blocoIni[NUM_INIMIGOS];
	trNaveEmBloco(&blocoNave, nave);
	trInimigosEmBloco(blocoIni, NUM_INIMIGOS, inimigo);
	for (int i = 0; i < NUM_INIMIGOS; i++)
	{
		if (colisaoBlocos(blocoNave, blocoIni[i]))
			return 1;
	}
	return 0;
}

int colisaoBalaBloco(Balas bala[], Bloco bloco)
{
	Bloco blocoBala; 
	for (int i = 0; i < NUM_BALAS; i++)
	{
		 trBalasEmBloco(&blocoBala, NUM_BALAS, bala);
		if (bala[i].atirou && bloco.ativo)
		{
			if (colisaoBlocos(bloco, blocoBala))
			{
				bala[i].atirou = false;
				bala[i].raio =  3.0;
				printf("teste");
			}
		}
	}
	return 0;
}

int colisaoBalaInimigos(Balas balas[], int bTamanho, Inimigo inimigo[], int iTamanho)
{ // entre as balas e os inimigos
	int pontuacao = 0;
	for (int i = 0; i < bTamanho; i++)
	{
		if (balas[i].atirou) // se a bala estiver ativa
		{
			for (int j = 0; j < iTamanho; j++)
			{
				if (inimigo[j].ativo) // se o inimigo estiver ativo
				{
					if (distancia(balas[i].centro, inimigo[j].centro) < balas[i].raio + inimigo[j].raio 
					&& balas[i].raio < RAIOESPECIAL)
					{
						balas[i].raio = 3.0;
						balas[i].atirou = false;
						inimigo[j].ativo = false;
						if(inimigo[j].raio < RAIOMAXIMO)
							pontuacao = pontuacao + 5;
						else
							pontuacao = pontuacao + 10; 
					}
					else if(distancia(balas[i].centro, inimigo[j].centro) < balas[i].raio + inimigo[j].raio 
					&& balas[i].raio == RAIOESPECIAL)
					{
						inimigo[j].ativo = false;
						if(inimigo[j].raio < RAIOMAXIMO)
							pontuacao = pontuacao + 5;
						else
							pontuacao = pontuacao + 10; 
					}
				}
			}
		}
	}
	return pontuacao;
}

//_______________________________________________________________________________

//---------------------------INICIO DA FUNCAO MAIN--------------------------------

int main(int argc, char **argv)
{

	srand(time(NULL));

	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;

	//------------ROTINAS DE INICIALIZACAO------------

	// inicializa o Allegro
	if (!al_init())
	{
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	// inicializa o módulo de primitivas do Allegro
	if (!al_init_primitives_addon())
	{
		fprintf(stderr, "failed to initialize primitives!\n");
		return -1;
	}

	// inicializa o modulo que permite carregar imagens no jogo
	if (!al_init_image_addon())
	{
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}

	// al_load_bitmap("missile1.png");

	// cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
	timer = al_create_timer(1.0 / FPS);
	if (!timer)
	{
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	// cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if (!display)
	{
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	// instala o teclado
	if (!al_install_keyboard())
	{
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}

	// inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();

	// inicializa o modulo allegro que entende arquivos tff de fontes
	if (!al_init_ttf_addon())
	{
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}

	// carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
	ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);
	if (size_32 == NULL)
	{
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}
	ALLEGRO_FONT *size_24 = al_load_font("arial.ttf", 24, 1);
	if (size_24 == NULL)
	{
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

	// cria a fila de eventos
	event_queue = al_create_event_queue();
	if (!event_queue)
	{
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}

	//____________________________________________________________________________

	//---------------------------REGISTRO DE SOURCES------------------------------

	// registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	// registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	// registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	//____________________________________________________________________________

	//-------------------------INICIALIZACAO DE OBJETOS---------------------------

	initGlobais();

	srand(time(NULL));

	Nave nave;
	initNave(&nave);

	Bloco bloco;
	initBloco(&bloco);

	Inimigo inimigo[NUM_INIMIGOS];
	initInimigo(inimigo, NUM_INIMIGOS);

	Balas balas[NUM_BALAS];
	initBalas(balas, NUM_BALAS);

	// inicia o temporizador
	al_start_timer(timer);

	//____________________________________________________________________________

	//------------------------------LOOP PRINCIPAL-------------------------------

	int playing = 1; // enquanto o jogo estiver rodando
	int pontuacao = 0;
	int carregandoTiro;
	
	while (playing)
	{
		ALLEGRO_EVENT ev;
		// espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		// se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if (ev.type == ALLEGRO_EVENT_TIMER)
		{

			desenhaCenario();

			estrelas();

			atualizaBloco(&bloco);

			desenhaBloco(bloco);

			atualizaNave(&nave);

			desenhaNave(nave);

			soltaInimigo(inimigo, bloco, NUM_INIMIGOS);

			atualizaInimigo(inimigo, NUM_INIMIGOS);

			desenhaInimigo(inimigo, NUM_INIMIGOS);

			for (int i = 0; i < NUM_INIMIGOS; i++)
			{

				Bloco blocoInimigo;
				trInimigoEmBloco(&blocoInimigo, &inimigo[i]);
				if (colisaoBlocos(blocoInimigo, bloco))
				{
					inimigo[i].ativo = false;
				}
			}

			pontuacao += colisaoBalaInimigos(balas, NUM_BALAS, inimigo, NUM_INIMIGOS);
			

			al_draw_textf(size_24, al_map_rgb(0, 255, 0), 5, 20, 0, "PONTOS: %d", pontuacao);

			atualizaBalasCarregando(balas, NUM_BALAS, carregandoTiro);

			posicionaBalasCarregando(balas, nave, NUM_BALAS, carregandoTiro);

			atualizaBalasAtivas(balas, NUM_BALAS);

			desenhaBalasAtivas(balas, NUM_BALAS);

			colisaoBalaBloco(balas, bloco);

			playing = !(colisaoNaveBloco(nave, bloco) || colisaoNaveInimigos(nave, inimigo));

			// atualiza a tela (quando houver algo para mostrar)
			al_flip_display();

			if (al_get_timer_count(timer) % (int)FPS == 0)
				printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer) / FPS));
		}
		// se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			playing = 0;
		}

		// verificando se a nave está nos limites da tela

		if (nave.y - NAVE_H / 2 <= 0)
		{
			nave.y = 0 + NAVE_H / 2;
		}

		if (nave.y + NAVE_H / 2 >= SCREEN_H)
		{
			nave.y = SCREEN_H - NAVE_H / 2;
		}

		if (nave.x - NAVE_W <= 0)
		{
			nave.x = 0 + NAVE_W;
		}

		if (nave.x - NAVE_W >= SCREEN_W)
		{
			nave.x = SCREEN_W;
		}


		// se o tipo de evento for um pressionar de uma tecla
		else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{

			al_set_target_bitmap(al_get_backbuffer(display));

			switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_W:
				nave.dir_y--;
				break;

			case ALLEGRO_KEY_S:
				nave.dir_y++;
				break;

			case ALLEGRO_KEY_A:
				nave.dir_x--;
				break;

			case ALLEGRO_KEY_D:
				nave.dir_x++;
				break;

			case ALLEGRO_KEY_SPACE:
				carregandoTiro = 1;
				posicionaBalasCarregando(balas, nave, NUM_BALAS, carregandoTiro);
				atualizaBalasCarregando(balas, NUM_BALAS, carregandoTiro);
				break;
				
			case ALLEGRO_KEY_R:
				atualizaPosicaoAleatoria(inimigo, NUM_INIMIGOS);
			}
				
			// imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);
		}

		else if (ev.type == ALLEGRO_EVENT_KEY_UP)
		{

			switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_W:
				nave.dir_y++;
				break;

			case ALLEGRO_KEY_S:
				nave.dir_y--;
				break;

			case ALLEGRO_KEY_A:
				nave.dir_x++;
				break;

			case ALLEGRO_KEY_D:
				nave.dir_x--;
				break;

			case ALLEGRO_KEY_SPACE:
				carregandoTiro = 0;
				atiraBalas(balas, NUM_BALAS, carregandoTiro);
				break;
			

			}
		}

	} // fim do while

	al_rest(1);

	char texto[20];
	int recorde;
	al_clear_to_color(al_map_rgb(0, 0, 0));
	sprintf(texto, "Total de pontos: %d", pontuacao);
	al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W / 3, SCREEN_H / 2 - 100, 0, texto);
	if (novoRecorde(pontuacao, &recorde))
	{
		al_draw_text(size_32, al_map_rgb(0, 255, 0), SCREEN_W / 3, 30 + SCREEN_H / 2, 0, "NOVO RECORDE!");
	}
	else
	{
		sprintf(texto, "Recorde: %d", recorde);
		al_draw_text(size_32, al_map_rgb(0, 200, 30), SCREEN_W / 3, 30 + SCREEN_H / 2, 0, texto);
	}

	// reinicializa a tela
	al_flip_display();
	al_rest(2);

	//_______________________________________________________________________________

	//-------PROCEDIMENTOS DE FIM DE JOGO (fecha a tela, limpa a memoria, etc)--------

	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}