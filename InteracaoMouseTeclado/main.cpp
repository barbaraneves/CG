#include <iostream>
#include <gui.h>

using namespace std;

void Desenha()
{
    //Define a cor de fundo da tela como branca.
    glClearColor(1.0, 1.0, 1.0, 1.0);

    //Limpa a janela e pinta com a cor de fundo selecionada.
    glClear(GL_COLOR_BUFFER_BIT);

    //Altera a cor para amarelo.
    glColor3f(1, 2, 0);

    //Define o conjunto de vértices pertencentes a um quadrado.
    glBegin(GL_QUADS);
        glVertex2d(1.3, 1.3);
        glVertex2d(1.7, 1.3);
        glVertex2d(1.7, 1.7);
        glVertex2d(1.3, 1.7);
    glEnd();

    //Executa os códigos OpenGL.
    glFlush();
}

void Teclado(unsigned char tecla, int x, int y)
{
    if (tecla == 'd') {
        exit(0);
    }
}

void Mouse(int botao, int estado, int x, int y)
{
    if (botao == GLUT_LEFT_BUTTON) {
        if (estado == GLUT_DOWN) {
            cout << "O botão esquerdo foi pressionado!" << endl;
        }
    }
}

void Inicializa()
{
    //Matrix de projeção.
    glMatrixMode(GL_PROJECTION);

    //Modo de projeção (2D).
    gluOrtho2D(0.0, 3.0, 0.0, 3.0);

    //Matrix de modelagem.
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
    //Inicializa a GLUT.
    glutInit(&argc, argv);

    //Função de inicialização da GLUT que define o modo de operação.
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

    //Função que especifica a posição inicial da janela.
    glutInitWindowPosition(40, 40);

    //Função que define o tamanho e a altura da janela (em pixels).
    glutInitWindowSize(600, 600);

    //Função que cria a janela e usa como argumento o nome da janela.
    glutCreateWindow("Interacao com mouse e teclado");

    //Função de callback que desenha e redesenha a janela.
    glutDisplayFunc(Desenha);

    //Função de callback que trata de eventos do teclado.
    glutKeyboardFunc(Teclado);

    //Função de callback que trata de eventos do mouse.
    glutMouseFunc(Mouse);

    Inicializa();

    //Aguarda as intereções com o usuário.
    glutMainLoop();

    return 0;
}
