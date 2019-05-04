#include <iostream>
#include <gui.h>

using namespace std;

void Desenha()
{
    //Define a cor de fundo da tela como branca.
    glClearColor(1.0, 1.0, 1.0, 1.0);

    //Limpa a janela e pinta com a cor de fundo selecionada.
    glClear(GL_COLOR_BUFFER_BIT);

    //Executa os códigos OpenGL.
    glFlush();
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
    glutCreateWindow("Primeira janela OpenGL");

    //Função de callback que desenha e redesenha a janela.
    glutDisplayFunc(Desenha);

    Inicializa();

    //Aguarda as intereções com o usuário.
    glutMainLoop();

    return 0;
}
