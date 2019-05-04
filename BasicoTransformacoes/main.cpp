#include <iostream>
#include <gui.h>

using namespace std;

GLfloat Tx = 0;

void Desenha()
{
    //Aplica as transformações sobre a imagem.
    glMatrixMode(GL_MODELVIEW);

    //Transforma a matriz de transformações corrente na matriz identidade.
    glLoadIdentity();

    //Define a cor de fundo da tela como branca.
    glClearColor(1.0, 1.0, 1.0, 1.0);

    //Limpa a janela e pinta com a cor de fundo selecionada.
    glClear(GL_COLOR_BUFFER_BIT);

    //Altera a cor para preto.
    glColor3f(0, 0, 0);

    //Empilha a matriz.
    glPushMatrix();

    //Translada o objeto gráfico em Tx.
    glTranslatef(Tx, 0, 0);

    //Define os vértices do triângulo.
    glBegin(GL_TRIANGLES);
        glVertex2f(0.0, 0.0);
        glVertex2f(0.4, 0.0);
        glVertex2f(0.2, 0.5);
    glEnd();

    //Recupera a matriz de transformações da pilha.
    glPopMatrix();

    //Define o conjunto de vértices pertencentes do quadrado.
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
    else if (tecla == 't') {
        Tx = Tx + 0.1;
    }
    glutPostRedisplay();
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
    glutCreateWindow("Aplicacao de Translacao");

    //Função de callback que desenha e redesenha a janela.
    glutDisplayFunc(Desenha);

    //Função de callback que trata de eventos do teclado.
    glutKeyboardFunc(Teclado);

    Inicializa();

    //Aguarda as intereções com o usuário.
    glutMainLoop();

    return 0;
}
