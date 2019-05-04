#include <GL/glut.h>
#include <iostream>
#include <iomanip>

using namespace std;

#include "CameraDistante.h"
#include "Desenha.h"

#include "gui.h"

#include "stanfordbunny.h"
#include "model3ds.h"

GLuint gBunnyWireList = NULL;
GLuint gBunnySolidList = NULL;

string str;
Model3DS* modelo = new Model3DS("3ds/wolf.3ds");

//variaveis globais
int width = 800;
int height = 600;

int slices = 16;
int stacks = 16;

float trans_obj = false;
float trans_luz = false;

float tx = 0.0;
float ty = 0.0;
float tz = 0.0;

float ax = 0.0;
float ay = 0.0;
float az = 0.0;

float delta = 5.0;

float sx = 1.0;
float sy = 1.0;
float sz = 1.0;

//ponto em coords locais, a ser calculado em coords de mundo
float pl[4] = { 0.0, 0.0, 0.0, 1.0 };
//ponto em coords globais, resultado da conversao a partir de pl
float pg[4] = { 0.0, 0.0, 0.0, 1.0 };

bool lbpressed = false;
bool mbpressed = false;
bool rbpressed = false;

float last_x = 0.0;
float last_y = 0.0;

Camera* cam = new CameraDistante();
float savedCamera[9];

Camera* cam2 = new CameraDistante(-3,2,-5, 0,0,0, 0,1,0);
//Camera* cam2 = new CameraDistante(2,1,0, 2,1,-5, -1,0,0);
bool manual_cam = false;
bool change_manual_cam = false;

GLfloat light_position[] = { 1.5f, 1.5f, 1.5f, 1.0f };

//projecao
Vetor3D objProj;
bool drawGround = false;
bool drawGrid = true;
bool drawShadow = false;
bool drawMesh = false;
bool normProj = false;
enum {PERSP = 0, ORTHO, OBLIQ};
int normProjType = PERSP;

//volume de visualizacao
float s = 0.5; //1.0;
float x = -s;
float X =  s;
float y = -s;
float Y =  s;
float near = 1.5;
float far = 2.5;
//matriz de cisalhamento (projecao obliqua)
    float alfaG = 75; //60; //45; //30 //90
    float phiG = -75; //-60; //-45; //-30 //-90
bool visVolEdit = false;

//subrotinas extras
void mult_matriz_vetor(float res[4], float matriz[16], float entr[4]) {
    for (int i = 0; i < 4; i++) {
        res[i] = 0.0;
        for (int j = 0; j < 4; j++) {
            //res[i] += matriz[4*i+j] * entr[j];
            res[i] += matriz[4*j+i] * entr[j]; //matriz^T.entr
        }
    }
}

void mostra_matriz_transform(float matriz[16], bool transposta = true) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (!transposta) {
                cout << setiosflags (ios::fixed) << setprecision(2) << matriz[4*i+j] << "  ";
            } else {
                cout << setiosflags (ios::fixed) << setprecision(2) << matriz[4*j+i] << "  "; //matriz^T
            }
        }
        cout << "\n";
    }
    //cout << "\n";
}

void imprime_coords_locais_globais()
{
    //imprimindo coords locais e coords globais
      //locais
        cout << "Coords locais: " << pl[0] << ", " << pl[1] << ", " << pl[2] << "\n";
      //globais
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
            glLoadIdentity();
              //composicao de transformacoes
              glTranslated(tx,ty,tz);
              glRotated(az,0,0,1);
              glRotated(ay,0,1,0);
              glRotated(ax,1,0,0);
            float mudanca_sist_coords[16];
            glGetFloatv(GL_MODELVIEW_MATRIX,mudanca_sist_coords);
            cout << "Matriz mudanca sist coords local=>global (T.Rz.Ry.Rx):\n";
            mostra_matriz_transform(mudanca_sist_coords);
            mult_matriz_vetor(pg,mudanca_sist_coords,pl);
            cout << "Coords globais: " << pg[0] << ", " << pg[1] << ", " << pg[2] << "\n\n";
        glPopMatrix();
}

void desenha_camera(float tam) {
    GLUquadricObj *quad = gluNewQuadric();

    glPushMatrix();
      Desenha::drawBox( -tam,-tam,0.0, tam,tam,2*tam );
      glTranslated(0,0,-tam);
      Desenha::gluClosedCylinder(quad, tam, tam/2, tam, slices,stacks);
    glPopMatrix();

    gluDeleteQuadric( quad );
}

void transformacao_camera_2_global(Vetor3D e, Vetor3D c, Vetor3D u, bool mostra_matriz = false)
{
    //z'
    Vetor3D z_ = e - c;
    z_.normaliza();
    //x'
    Vetor3D x_ = u ^ z_;
    x_.normaliza();
    //y'
    Vetor3D y_ = z_ ^ x_;
    //y_.normaliza();

    //matriz de transformacao
        float transform[16] = {
                                x_.x,   y_.x,   z_.x,   e.x,
                                x_.y,   y_.y,   z_.y,   e.y,
                                x_.z,   y_.z,   z_.z,   e.z,
                                0.0,    0.0,    0.0,    1.0
                             };
        glMultTransposeMatrixf( transform );

    if (mostra_matriz) {
        cout << "Matriz mudanca sist coords camera2=>global (R t = x' y' z' e):\n";
        mostra_matriz_transform(transform,false);
        cout << "\n";
    }
}

void transformacao_global_2_camera(Vetor3D e, Vetor3D c, Vetor3D u, bool mostra_matriz = false)
{
    //z'
    Vetor3D z_ = e - c;
    z_.normaliza();
    //x'
    Vetor3D x_ = u ^ z_;
    x_.normaliza();
    //y'
    Vetor3D y_ = z_ ^ x_;
    //y_.normaliza();

    //t'=-R^T.t=R^T.(-e)
    Vetor3D t_;
        t_.x = x_ * ( e * (-1) );
        t_.y = y_ * ( e * (-1) );
        t_.z = z_ * ( e * (-1) );

    //matriz de transformacao
        float transform[16] = {
                                x_.x,   x_.y,   x_.z,   t_.x,
                                y_.x,   y_.y,   y_.z,   t_.y,
                                z_.x,   z_.y,   z_.z,   t_.z,
                                0.0,    0.0,    0.0,    1.0
                             };
        glMultTransposeMatrixf( transform );

    if (mostra_matriz) {
        cout << "Matriz mudanca sist coords global=>camera2 (R^T -R^T.t):\n";
        mostra_matriz_transform(transform,false);
        cout << "\n";
    }
}


/* GLUT callback Handlers */

void resize(int w, int h)
{
    width = w;
    height = h;
}

void displayInit()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float ar = height>0 ? (float) width / (float) height : 1.0;

    glViewport(0, 0, width, height);

//------------------projecao------------------
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.,ar,0.1,1000.);
    //gluPerspective(150.,ar,0.1,1000.); //pontos de fuga (rotaciona o objeto em x, depois em y)

    //testes
    //glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    //glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);
    //glOrtho(-ar, ar, -1.0, 1.0, 2.0, 100.0);
    //glOrtho(-10*ar, 10*ar, -10.0, 10.0, 2.0, 100.0);
    //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
    //gluPerspective(30.,ar,0.1,1000.);

    //obliqua
    //    float s = 10;
    //    glOrtho(-s, s, -s, s, -s, s);
    //    //matriz de cisalhamento (projecao obliqua)
    //        float alfa = 45; //60; //30 //90
    //        alfa = alfa*(PI/180); //grau2rad
    //        float phi = -45; //-60; //-30 //-90
    //        phi = phi*(PI/180); //grau2rad
    //        //float d = 1.0; //1.0/2.0;
    //        float transform[16] = {
    //            //1.0,    0.0,    -d*cos(alfa),    0.0,
    //            //0.0,    1.0,    -d*cos(phi),     0.0,
    //            1.0,    0.0,    1.0/tan(alfa),   0.0,
    //            0.0,    1.0,    1.0/tan(phi),    0.0,
    //            0.0,    0.0,    1.0,             0.0,
    //            0.0,    0.0,    0.0,             1.0
    //                             };
    //        glMultTransposeMatrixf( transform );

    //testes
    //perspectiva
        //gluPerspective(30.,1.0,1.5,10.0); //2.5);
        //float s = 0.5; //1.0;
        //glFrustum(-s, s, -s, s, 1.5, 10.0); //2.5);
    //ortografica
        //float s = 0.5; //1.0;
        //glOrtho(-s, s, -s, s, 1.5, 10.0); //2.5);
    //obliqua
        //    float s = 0.5; //1.0;
        //    float near = 1.5;
        //    glOrtho(-s, s, -s, s, near, 10.0); //2.5);
        //    glTranslatef(0.0,0.0,-near); //translada -near em z de volta
        //    //matriz de cisalhamento (projecao obliqua)
        //        float alfa = 45; //30 //90
        //        alfa = alfa*(PI/180); //grau2rad
        //        float phi = 45; //30 //90
        //        phi = phi*(PI/180); //grau2rad
        //        //float d = 1.0; //1.0/2.0;
        //        float transform[16] = {
        //            //1.0,    0.0,    -d*cos(alfa),    0.0,
        //            //0.0,    1.0,    -d*cos(phi),     0.0,
        //            1.0,    0.0,    1.0/tan(alfa),   0.0,
        //            0.0,    1.0,    1.0/tan(phi),    0.0,
        //            0.0,    0.0,    1.0,             0.0,
        //            0.0,    0.0,    0.0,             1.0
        //                             };
        //        glMultTransposeMatrixf( transform );
        //    glTranslatef(0.0,0.0,near); //translada near em z
//------------------projecao------------------
}

void displayEnd()
{
    glutSwapBuffers();
}

//-------------------sombra-------------------
//desenha todos os objetos que possuem sombra
void desenhaObjetosComSombra() {
    //sistema local 1
    glPushMatrix();
        //composicao de transformacoes
        glTranslatef(objProj.x,objProj.y,objProj.z);
        glTranslated(tx,ty,tz);
        glRotated(az,0,0,1);
        glRotated(ay,0,1,0);
        glRotated(ax,1,0,0);
        //desenhando ponto, dado em coords do sistema local 1
//          glPushMatrix();
//            glTranslated(pl[0],pl[1],pl[2]);
//            glutSolidSphere(0.1,slices,stacks);
//          glPopMatrix();
        //retomando as transformacoes do objeto do sistema local 1
        glScaled(sx,sx,sx);
        //glScaled(sx,sy,sz);
        //desenhando eixos do sistema de coordenadas local 1
          //Desenha::drawEixos( 0.5 );
        //desenhando objeto
        //glColor3d(0.3,0.3,0.3);
        //glColor3d(0.0,0.5,0.0);
        //glutSolidTorus(0.2,0.8,slices,stacks);
        //glCallList (gBunnySolidList);
        if (drawMesh) {
            glTranslatef(0.5,0.0,0.5);
            glRotatef(-90,1,0,0);
            modelo->draw();
        } else {
            //Desenha::drawBox( 0.0,0.0,0.0, 1.0,1.0,1.0 );
            GUI::drawQuadBox( 0.0,0.0,0.0, 1.0,1.0,1.0 );
        }
    glPopMatrix();
}
//-------------------sombra-------------------

void displayInner(bool manual_cam)
{
    //tempo
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    //cam->e.z = 10+t;
    //tx = t;
    //ay = 100*t;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //transformacao de camera
    //definindo posicao e orientacao da camera
    gluLookAt(cam->e.x,cam->e.y,cam->e.z, cam->c.x,cam->c.y,cam->c.z, cam->u.x,cam->u.y,cam->u.z);

    //transformacao de camera
    //if (!manual_cam) {
    //    //definindo posicao e orientacao da camera
    //    gluLookAt(cam->e.x,cam->e.y,cam->e.z, cam->c.x,cam->c.y,cam->c.z, cam->u.x,cam->u.y,cam->u.z);
    //} else {
    //    //definindo posicao e orientacao da camera 2 "manualmente"
    //    transformacao_global_2_camera(cam2->e,cam2->c,cam2->u);
    //}

    //sistema global
    glPushMatrix();
        //posicao da luz
            glutGUI::trans_luz = trans_luz;
            GUI::setLight(0,light_position[0],light_position[1],light_position[2],false,false);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        //desenhando eixos do sistema de coordenadas global
            Desenha::drawEixos( 0.5 );
        //chao
            if (drawGround) {
                glColor3d(0.6,0.4,0.0);
                glTranslated(0.0,-0.001,0.0);
                GUI::drawFloor(15.0,15.0,0.5,0.5);
            }
            if (drawGrid) {
                glColor3d(0.0,0.0,0.0);
                Desenha::drawGrid( 10, 0, 10, 1 );
            }
    glPopMatrix();

//-------------------sombra-------------------
    //sistema local 1 com sombra
    if (!normProj) {
        glPushMatrix();
            //glColor3d(0.3,0.3,0.3);
            glColor3d(0.0,0.5,0.0);
            desenhaObjetosComSombra();
        glPopMatrix();
        glPushMatrix();
            //matriz p multiplicar tudo por -1
                //float neg[16] = {
                //                   -1.0, 0.0, 0.0, 0.0,
                //                    0.0,-1.0, 0.0, 0.0,
                //                    0.0, 0.0,-1.0, 0.0,
                //                    0.0, 0.0, 0.0,-1.0
                //                };
                //glMultTransposeMatrixf( neg );
            //matriz de projecao para gerar sombra no plano y=0
                float sombra[16] = {
                                     light_position[1], -light_position[0],                0.0,                0.0,
                                                   0.0,                0.0,                0.0,                0.0,
                                                   0.0, -light_position[2],  light_position[1],                0.0,
                                                   0.0, -light_position[3],                0.0,  light_position[1]
                                   };
                glMultTransposeMatrixf( sombra );
            glDisable(GL_LIGHTING);
            glColor3d(0.0,0.0,0.0);
            if (drawShadow) desenhaObjetosComSombra();
            glEnable(GL_LIGHTING);
        glPopMatrix();
    }
//-------------------sombra-------------------

//----------normalizacao da projecao----------
    //sistema local 1 com distorção de projeção (normalização da projeção)
    if (normProj) {
        //variaveis
            //float s = 0.5; //1.0;
            //float x = -s;
            //float X =  s;
            //float y = -s;
            //float Y =  s;
            //float near = 4.5;
            //float far = 5.5;
            //matriz de cisalhamento (projecao obliqua)
                //float alfa = 75; //60; //45; //30 //90
                float alfa = alfaG*(PI/180); //grau2rad
                //float phi = -75; //-60; //-45; //-30 //-90
                float phi = phiG*(PI/180); //grau2rad
                //float d = 1.0; //1.0/2.0;
                float transform[16] = {
                    //1.0,    0.0,    -d*cos(alfa),    0.0,
                    //0.0,    1.0,    -d*cos(phi),     0.0,
                    1.0,    0.0,    1.0/tan(alfa),   0.0,
                    0.0,    1.0,    1.0/tan(phi),    0.0,
                    0.0,    0.0,    1.0,             0.0,
                    0.0,    0.0,    0.0,             1.0
                                     };
        //marcacoes didaticas
            glPushMatrix();
                //camera
                glColor3d(0.3,0.3,0.3);
                desenha_camera(0.1);
                //cubo 2x2x2
                glColor3d(1.0,0.0,0.0);
                Desenha::drawSelectedBox(-1,1,-1,1,-1,1);
                //imagem
                glBegin(GL_LINE_STRIP);
                    glVertex3f(-1,-1,1);
                    glVertex3f( 1,-1,1);
                    glVertex3f( 1, 1,1);
                    glVertex3f(-1, 1,1);
                    glVertex3f(-1,-1,1);
                glEnd();
                //volume de visualizacao
                glColor3d(1.0,0.5,0.0);
                    float xfp = x*far/near;
                    float Xfp = X*far/near;
                    float yfp = y*far/near;
                    float Yfp = Y*far/near;
                    float xfo = x + (1.0/tan(alfa))*(far-near);
                    float Xfo = X + (1.0/tan(alfa))*(far-near);
                    float yfo = y + (1.0/tan(phi))*(far-near);
                    float Yfo = Y + (1.0/tan(phi))*(far-near);
                    float x0o = x - (1.0/tan(alfa))*(near);
                    float X0o = X - (1.0/tan(alfa))*(near);
                    float y0o = y - (1.0/tan(phi))*(near);
                    float Y0o = Y - (1.0/tan(phi))*(near);
                switch (normProjType) {
                case PERSP:
                    glBegin(GL_LINE_STRIP);
                        glVertex3f(x,y,-near);
                        glVertex3f(X,y,-near);
                        glVertex3f(X,Y,-near);
                        glVertex3f(x,Y,-near);
                        glVertex3f(x,y,-near);
                    glEnd();
                    glBegin(GL_LINE_STRIP);
                        glVertex3f(xfp,yfp,-far);
                        glVertex3f(Xfp,yfp,-far);
                        glVertex3f(Xfp,Yfp,-far);
                        glVertex3f(xfp,Yfp,-far);
                        glVertex3f(xfp,yfp,-far);
                    glEnd();
                    glBegin(GL_LINES);
                        glVertex3f(x ,y ,-near);
                        glVertex3f(xfp,yfp,-far );
                        glVertex3f(X ,y ,-near);
                        glVertex3f(Xfp,yfp,-far );
                        glVertex3f(X ,Y ,-near);
                        glVertex3f(Xfp,Yfp,-far );
                        glVertex3f(x ,Y ,-near);
                        glVertex3f(xfp,Yfp,-far );
                    glEnd();
                    glBegin(GL_LINES);
                        glVertex3f(0  ,0  , 0   );
                        glVertex3f(xfp,yfp,-far );
                        glVertex3f(0  ,0  , 0   );
                        glVertex3f(Xfp,yfp,-far );
                        glVertex3f(0  ,0  , 0   );
                        glVertex3f(Xfp,Yfp,-far );
                        glVertex3f(0  ,0  , 0   );
                        glVertex3f(xfp,Yfp,-far );
                    glEnd();
                    break;
                case ORTHO:
                    glBegin(GL_LINE_STRIP);
                        glVertex3f(x,y,-near);
                        glVertex3f(X,y,-near);
                        glVertex3f(X,Y,-near);
                        glVertex3f(x,Y,-near);
                        glVertex3f(x,y,-near);
                    glEnd();
                    glBegin(GL_LINE_STRIP);
                        glVertex3f(x,y,-far);
                        glVertex3f(X,y,-far);
                        glVertex3f(X,Y,-far);
                        glVertex3f(x,Y,-far);
                        glVertex3f(x,y,-far);
                    glEnd();
                    glBegin(GL_LINES);
                        glVertex3f(x ,y ,-near);
                        glVertex3f(x ,y ,-far);
                        glVertex3f(X ,y ,-near);
                        glVertex3f(X ,y ,-far);
                        glVertex3f(X ,Y ,-near);
                        glVertex3f(X ,Y ,-far);
                        glVertex3f(x ,Y ,-near);
                        glVertex3f(x ,Y ,-far);
                    glEnd();
                    glBegin(GL_LINES);
                        glVertex3f(x ,y , 0);
                        glVertex3f(x ,y ,-far);
                        glVertex3f(X ,y , 0);
                        glVertex3f(X ,y ,-far);
                        glVertex3f(X ,Y , 0);
                        glVertex3f(X ,Y ,-far);
                        glVertex3f(x ,Y , 0);
                        glVertex3f(x ,Y ,-far);
                    glEnd();
                    break;
                case OBLIQ:
                    glBegin(GL_LINE_STRIP);
                        glVertex3f(x,y,-near);
                        glVertex3f(X,y,-near);
                        glVertex3f(X,Y,-near);
                        glVertex3f(x,Y,-near);
                        glVertex3f(x,y,-near);
                    glEnd();
                    glBegin(GL_LINE_STRIP);
                        glVertex3f(xfo,yfo,-far);
                        glVertex3f(Xfo,yfo,-far);
                        glVertex3f(Xfo,Yfo,-far);
                        glVertex3f(xfo,Yfo,-far);
                        glVertex3f(xfo,yfo,-far);
                    glEnd();
                    glBegin(GL_LINES);
                        glVertex3f(x ,y ,-near);
                        glVertex3f(xfo,yfo,-far );
                        glVertex3f(X ,y ,-near);
                        glVertex3f(Xfo,yfo,-far );
                        glVertex3f(X ,Y ,-near);
                        glVertex3f(Xfo,Yfo,-far );
                        glVertex3f(x ,Y ,-near);
                        glVertex3f(xfo,Yfo,-far );
                    glEnd();
                    glBegin(GL_LINES);
                        glVertex3f(x0o,y0o, 0   );
                        glVertex3f(xfo,yfo,-far );
                        glVertex3f(X0o,y0o, 0   );
                        glVertex3f(Xfo,yfo,-far );
                        glVertex3f(X0o,Y0o, 0   );
                        glVertex3f(Xfo,Yfo,-far );
                        glVertex3f(x0o,Y0o, 0   );
                        glVertex3f(xfo,Yfo,-far );
                    glEnd();
                    break;
                default:
                    break;
                }
            glPopMatrix();
        //objeto original
            glPushMatrix();
                //glColor3d(0.3,0.3,0.3);
                glColor3d(0.0,0.5,0.0);
                glTranslatef(-0.5,-0.5,-0.5-0.5*(far+near));
                desenhaObjetosComSombra();
            glPopMatrix();
        //objeto distorcido pela normalizacao da projecao
            glPushMatrix();
                //glColor4d(0.3,0.3,0.3,0.4); //transparente
                glColor4d(0.0,0.5,0.0,0.4); //transparente
                //glTranslatef(2.,0,0);
                //matriz p multiplicar z por -1
                    float zNeg[16] = {
                                        1.0, 0.0, 0.0, 0.0,
                                        0.0, 1.0, 0.0, 0.0,
                                        0.0, 0.0,-1.0, 0.0,
                                        0.0, 0.0, 0.0, 1.0
                                     };
                    glMultTransposeMatrixf( zNeg );
                //escolhe tipo da projecao
                switch (normProjType) {
                case PERSP:
                    //matriz de projecao perspectiva
                        //gluPerspective(30.,1.,near,far);
                        glFrustum(x,X, y,Y, near,far);
                    break;
                case ORTHO:
                    //matriz de projecao ortografica
                        glOrtho(x,X, y,Y, near,far);
                    break;
                case OBLIQ:
                    //matriz de projecao obliqua
                        glOrtho(x,X, y,Y, near,far);
                        glTranslatef(0.0,0.0,-near); //translada -near em z de volta
                        //matriz de cisalhamento (projecao obliqua)
                            glMultTransposeMatrixf( transform );
                        glTranslatef(0.0,0.0,near); //translada near em z
                    break;
                default:
                    break;
                }
                //desenha objeto
                glTranslatef(-0.5,-0.5,-0.5-0.5*(far+near));
                desenhaObjetosComSombra();
            glPopMatrix();
    }

    //matriz de projecao perspectiva (gluPerspective)
    //    float far = 2.5;
    //    float near = 1.5;
    //    float esq = (far+near)/(far-near);
    //    float dir = (2*far*near)/(far-near);
    //    float persp[16] = {
    //                        1.0, 0.0, 0.0, 0.0,
    //                        0.0, 1.0, 0.0, 0.0,
    //                        0.0, 0.0, esq, dir,
    //                        0.0, 0.0,-1.0, 0.0
    //                      };
    //glMultTransposeMatrixf( persp );
    //    float fovy = 30.;
    //    float aspect = 1.;
    //    float f = 1./tan((fovy/2.)*(PI/180.));
    //    float pri = f/aspect;
    //    float seg = f;
    //    float Escala[16] = {
    //                          pri, 0.0, 0.0, 0.0,
    //                          0.0, seg, 0.0, 0.0,
    //                          0.0, 0.0, 1.0, 0.0,
    //                          0.0, 0.0, 0.0, 1.0
    //                       };
    //glMultTransposeMatrixf( Escala );
//----------normalizacao da projecao----------

    //sistema local 2
    //glPushMatrix();
    //    //composicao de transformacoes
    //    //glTranslated(0,1.5,0);
    //    glTranslated(2,1,-3);
    //    glRotated(30,0,0,1);
    //    glRotated(10,0,1,0);
    //    glRotated(-20,1,0,0);
    //    //glScaled(sx,sy,sz);
    //    //desenhando eixos do sistema de coordenadas local 2
    //      Desenha::drawEixos( 0.5 );
    //    //matriz de transformacao
    //        //float transform[16] = {
    //        //                        1.0,     sy,     sz,    0.0,
    //        //                        0.0,    1.0,    0.0,    0.0,
    //        //                        0.0,    0.0,    1.0,    0.0,
    //        //                        0.0,    0.0,    0.0,    1.0
    //        //                     };
    //        //glMultTransposeMatrixf( transform );
    //    //desenhando objeto
    //    glColor3d(1,0,0);
    //    Desenha::drawBox( 0.0,0.0,0.0, 1.0,1.0,1.0 );
    //    //Desenha::drawBox( -1.0,-1.0,-1.0, 1.0,1.0,1.0 );
    //glPopMatrix();

    //sistema local da camera
    //if (!manual_cam) {
    //    glPushMatrix();
    //        //definindo sistema de coords atraves do glulookat (eye,center,up)
    //        transformacao_camera_2_global(cam2->e,cam2->c,cam2->u);
    //        //desenhando eixos do sistema de coordenadas local da camera
    //          Desenha::drawEixos( 0.5 );
    //        //desenhando camera
    //        glColor3d(0.5,0.5,0.0);
    //        desenha_camera(0.2);
    //    glPopMatrix();
    //} else {
    //    glPushMatrix();
    //        //definindo sistema de coords atraves do glulookat (eye,center,up)
    //        transformacao_camera_2_global(cam->e,cam->c,cam->u);
    //        //desenhando eixos do sistema de coordenadas local da camera
    //          Desenha::drawEixos( 0.5 );
    //        //desenhando camera
    //        glColor3d(0.5,0.5,0.5);
    //        desenha_camera(0.2);
    //    glPopMatrix();
    //}

}

void display()
{
    displayInit();
    displayInner(manual_cam);
        //glViewport(0, 0, width, height);
        //displayInner(manual_cam);
        //glViewport(0, 3*height/4, width/4, height/4);
        //displayInner(!manual_cam);
    displayEnd();
}

void mouseButton(int button, int state, int x, int y) {
	// if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
		// when the button is pressed
		if (state == GLUT_DOWN) {
            lbpressed = true;
		} else {// state = GLUT_UP
            lbpressed = false;
		}
	}
	// if the middle button is pressed
	if (button == GLUT_MIDDLE_BUTTON) {
		// when the button is pressed
		if (state == GLUT_DOWN) {
            mbpressed = true;
		} else {// state = GLUT_UP
            mbpressed = false;
		}
	}
	// if the left button is pressed
	if (button == GLUT_RIGHT_BUTTON) {
		// when the button is pressed
		if (state == GLUT_DOWN) {
            rbpressed = true;
		} else {// state = GLUT_UP
            rbpressed = false;
		}
	}

    last_x = x;
    last_y = y;
}

void mouseMove(int x_, int y_) {
    float fator = 10.0;
    if (lbpressed && !rbpressed && !mbpressed) {
        if (visVolEdit) {
            x += (x_ - last_x)/100.0;
            y += -(y_ - last_y)/100.0;
        } else
        if (!trans_obj) {
            if (!manual_cam) {
                if (!change_manual_cam) {
                    cam->rotatex(y_,last_y);
                    cam->rotatey(x_,last_x);
                } else {
                    cam2->rotatex(last_y,y_);
                    cam2->rotatey(last_x,x_);
                }
            } else {
                if (!change_manual_cam) {
                    cam->rotatex(last_y,y_);
                    cam->rotatey(last_x,x_);
                } else {
                    cam2->rotatex(y_,last_y);
                    cam2->rotatey(x_,last_x);
                }
            }
        } else {
            ax += (y_ - last_y)/fator;
            ay += (x_ - last_x)/fator;
        }
    }
    fator = 100.0;
    if (!lbpressed && rbpressed && !mbpressed) {
        if (visVolEdit) {
            X += (x_ - last_x)/100.0;
            Y += -(y_ - last_y)/100.0;
        } else
        if (!trans_obj && !trans_luz) {
            if (!manual_cam) {
                if (!change_manual_cam) {
                    cam->translatex(x_,last_x);
                    cam->translatey(y_,last_y);
                } else {
                    cam2->translatex(last_x,x_);
                    cam2->translatey(last_y,y_);
                }
            } else {
                if (!change_manual_cam) {
                    cam->translatex(last_x,x_);
                    cam->translatey(last_y,y_);
                } else {
                    cam2->translatex(x_,last_x);
                    cam2->translatey(y_,last_y);
                }
            }
        } else {
            if (trans_obj) {
                tx += (x_ - last_x)/fator;
                ty += -(y_ - last_y)/fator;
            }
            if (trans_luz) {
                light_position[0] += (x_ - last_x)/fator;
                light_position[1] += -(y_ - last_y)/fator;
            }
        }
    }
    if (lbpressed && rbpressed && !mbpressed) {
        if (visVolEdit) {
            near += (x_ - last_x)/100.0;
            far += -(y_ - last_y)/100.0;
        } else
        if (!trans_obj && !trans_luz) {
            if (!manual_cam) {
                if (!change_manual_cam) {
                    cam->zoom(y_,last_y);
                } else {
                    cam2->zoom(last_y,y_);
                }
            } else {
                if (!change_manual_cam) {
                    cam->zoom(last_y,y_);
                } else {
                    cam2->zoom(y_,last_y);
                }
            }
        } else {
            if (trans_obj) {
                tz += (y_ - last_y)/fator;
                fator = 10.0;
                az += -(x_ - last_x)/fator;
            }
            if (trans_luz) {
                light_position[2] += (y_ - last_y)/fator;
            }
        }
    }
    fator = 100.0;
    if (!lbpressed && !rbpressed && mbpressed) {
        if (visVolEdit) {
            alfaG += (x_ - last_x)/100.0;
            phiG += -(y_ - last_y)/100.0;
        } else
        if (!trans_obj) {
        } else {
            //sx *= 1+(x_ - last_x)/fator;
            sx += (x_ - last_x)/fator;
            sy += -(y_ - last_y)/fator;
        }
    }
    if (lbpressed && !rbpressed && mbpressed) {
        if (!trans_obj) {
        } else {
            sz += (y_ - last_y)/fator;
        }
    }
    if (!lbpressed && rbpressed && mbpressed) {
        if (!trans_obj) {
        } else {
            pl[0] += (x_ - last_x)/fator;
            pl[1] += -(y_ - last_y)/fator;
            imprime_coords_locais_globais();
        }
    }

    last_x = x_;
    last_y = y_;
}

void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 13 :
        case 'q':
            exit(0);
            break;

        case '+':
            slices++;
            stacks++;
            break;

        case '-':
            if (slices>3 && stacks>3)
            {
                slices--;
                stacks--;
            }
            break;

        case 'X':
            //ax+=delta;
            if(trans_obj) ax+=delta;
            else { pl[0] += 0.01; imprime_coords_locais_globais(); };
            break;

        case 'Y':
            //ay+=delta;
            if(trans_obj) ay+=delta;
            else { pl[1] += 0.01; imprime_coords_locais_globais(); };
            break;

        case 'Z':
            //az+=delta;
            if(trans_obj) az+=delta;
            else { pl[2] += 0.01; imprime_coords_locais_globais(); };
            break;

        case 'x':
            //ax-=delta;
            if(trans_obj) ax-=delta;
            else { pl[0] -= 0.01; imprime_coords_locais_globais(); };
            break;

        case 'y':
            //ay-=delta;
            if(trans_obj) ay-=delta;
            else { pl[1] -= 0.01; imprime_coords_locais_globais(); };
            break;

        case 'z':
            //az-=delta;
            if(trans_obj) az-=delta;
            else { pl[2] -= 0.01; imprime_coords_locais_globais(); };
            break;

        case 'i':
            ax=ay=az=0.0;
            tx=ty=tz=0.0;
            sx=sy=sz=1.0;
            break;

        case 't':
            trans_obj = !trans_obj;
            break;

        case 'l':
            trans_luz = !trans_luz;
            break;

        case 'p':
            light_position[3] = 1 - light_position[3];
            break;

        case 'G':
            drawGround = !drawGround;
            break;
        case 'g':
            drawGrid = !drawGrid;
            break;
        case 's':
            drawShadow = !drawShadow;
            break;
        case 'm':
            drawMesh = !drawMesh;
            break;
        case 'n':
            normProj = !normProj;
            break;
        case 'N':
            normProjType = (normProjType+1)%3;
            break;
        case 'v':
            visVolEdit = !visVolEdit;
            break;

        case 'c':
            static int posCam = 0;
            posCam++;
            if (cam->estilo == 1) {
                delete cam;
                if (posCam%5==0) cam = new CameraDistante(); //CameraDistante(0,1,5, 0,1,0, 0,1,0);
                if (posCam%5==1) cam = new CameraDistante(5,1,0, 0,1,0, 0,1,0);
                if (posCam%5==2) cam = new CameraDistante(0,1,-5, 0,1,0, 0,1,0);
                if (posCam%5==3) cam = new CameraDistante(-5,1,0, 0,1,0, 0,1,0);
                if (posCam%5==4) cam = new CameraDistante(savedCamera[0],savedCamera[1],savedCamera[2],savedCamera[3],savedCamera[4],savedCamera[5],savedCamera[6],savedCamera[7],savedCamera[8]);
            } else if (cam->estilo == 2) {
                delete cam;
                cam = new CameraDistante();
            }
            objProj = Vetor3D();
            break;

        case 'C':
            delete cam;
            cam = new CameraDistante(0,0,0, 0,0,-1, 0,1,0);
            objProj = Vetor3D(-0.5,-0.5,-2.5);
            break;

//        case 'C':
//            static int pos_manual_cam = 0;
//            pos_manual_cam++;
//            if (pos_manual_cam%4==0) {
//                cout << "Mostrando camera 1/Alterando camera 1:\n";
//                change_manual_cam = false;
//                manual_cam = false;
//            }
//            if (pos_manual_cam%4==1) {
//                cout << "Mostrando camera 1/Alterando camera 2:\n";
//                change_manual_cam = true;
//                manual_cam = false;
//            }
//            if (pos_manual_cam%4==2) {
//                cout << "Mostrando camera 2/Alterando camera 2:\n";
//                change_manual_cam = true;
//                manual_cam = true;
//            }
//            if (pos_manual_cam%4==3) {
//                cout << "Mostrando camera 2/Alterando camera 1:\n";
//                change_manual_cam = false;
//                manual_cam = true;
//            }
//            //manual_cam = !manual_cam;
//            if (manual_cam) {
//                glPushMatrix();
//                    transformacao_global_2_camera(cam2->e,cam2->c,cam2->u,true);
//                glPopMatrix();
//            } else {
//                glPushMatrix();
//                    transformacao_camera_2_global(cam2->e,cam2->c,cam2->u,true);
//                glPopMatrix();
//            }
//            break;

//        case 's':
//            //save current camera location
//            savedCamera[0] = cam->e.x;
//            savedCamera[1] = cam->e.y;
//            savedCamera[2] = cam->e.z;
//            savedCamera[3] = cam->c.x;
//            savedCamera[4] = cam->c.y;
//            savedCamera[5] = cam->c.z;
//            savedCamera[6] = cam->u.x;
//            savedCamera[7] = cam->u.y;
//            savedCamera[8] = cam->u.z;
//            break;

        case 'S':
            cout << "Insira o nome do arquivo .3ds a ser lido (ex.: skeleton.3ds): ";
            cin >> str;
            delete modelo;
            modelo = new Model3DS((string("3ds/")+str).c_str());
            break;

        case 'M':
            if (!manual_cam) {
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                    glLoadIdentity();
                      glTranslated(tx,ty,tz);
                      glRotated(az,0,0,1);
                      glRotated(ay,0,1,0);
                      glRotated(ax,1,0,0);
                      glScaled(sx,sy,sz);
                    float transform[16];
                    glGetFloatv(GL_MODELVIEW_MATRIX,transform);
                    cout << "Matriz composicao de transformacoes (T.Rz.Ry.Rx.S):\n";
                    mostra_matriz_transform(transform);
                    cout << "\n";
                glPopMatrix();
            } else {
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                    glLoadIdentity();
                      gluLookAt(cam2->e.x,cam2->e.y,cam2->e.z, cam2->c.x,cam2->c.y,cam2->c.z, cam2->u.x,cam2->u.y,cam2->u.z);
                    float transform[16];
                    glGetFloatv(GL_MODELVIEW_MATRIX,transform);
                    cout << "Matriz gluLookAt:\n";
                    mostra_matriz_transform(transform);
                    cout << "\n";
                glPopMatrix();
            }
            break;
    }

    glutPostRedisplay();
}

void idle(void)
{
    glutPostRedisplay();
}

/* Program entry point */

int main(int argc, char *argv[])
{
    //chamadas de inicializacao da GLUT
        glutInit(&argc, argv);
        glutInitWindowSize(width,height);
        glutInitWindowPosition(10,10);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

        glutCreateWindow("Sombras");

        glutReshapeFunc(resize);
        glutDisplayFunc(display);
        glutKeyboardFunc(key);
        glutIdleFunc(idle);

        glutMouseFunc(mouseButton);
        glutMotionFunc(mouseMove);

    //chamadas de inicializacao da OpenGL
        //glClearColor(1,1,1,1);
        //glClearColor(0.3,0.3,0.3,1.0);
        glClearColor(0.8,0.8,0.8,1.0);

        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        glEnable(GL_NORMALIZE); //mantem a qualidade da iluminacao mesmo quando glScalef eh usada

        glShadeModel(GL_SMOOTH);
        //glShadeModel(GL_FLAT);

        glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LESS);

        glEnable(GL_BLEND); //habilita a transparencia
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //definindo uma luz
            glEnable(GL_LIGHT0);

            const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
            const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
            const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            //const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

            const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
            const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
            const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
            const GLfloat high_shininess[] = { 100.0f };

            glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
            //glLightfv(GL_LIGHT0, GL_POSITION, light_position);

            glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
            glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

            //glFrontFace(GL_CCW);
            gBunnySolidList = GenStanfordBunnySolidList ();
            gBunnyWireList = GenStanfordBunnyWireList ();

    //iniciando o loop principal da glut
        glutMainLoop();

    return EXIT_SUCCESS;
}




/*
void draw_casa() {
  //parede
  glBegin(GL_QUADS);
    glColor3d(0,0,1);
      glVertex3d( 0.0, 0.0, 0.0 );
      glVertex3d( 4.0, 0.0, 0.0 );
      glVertex3d( 4.0, 3.0, 0.0 );
      glVertex3d( 0.0, 3.0, 0.0 );
  glEnd();
  //porta
  glBegin(GL_POLYGON);
    glColor3d(1,1,1);
      glVertex3d( 1.5, 0.0, 0.01 );
      glVertex3d( 2.5, 0.0, 0.01 );
      glVertex3d( 2.5, 2.0, 0.01 );
      glVertex3d( 1.5, 2.0, 0.01 );
  glEnd();
  //janela
  glBegin(GL_POLYGON);
    glColor3d(1,1,1);
      glVertex3d( 2.6, 1.5, 0.01 );
      glVertex3d( 3.6, 1.5, 0.01 );
      glVertex3d( 3.6, 2.5, 0.01 );
      glVertex3d( 2.6, 2.5, 0.01 );
  glEnd();
  glBegin(GL_LINES);
    glColor3d(0,0,1);
      glVertex3d( 2.6, 2.0, 0.02 );
      glVertex3d( 3.6, 2.0, 0.02 );
      glVertex3d( 3.1, 1.5, 0.02 );
      glVertex3d( 3.1, 2.5, 0.02 );
  glEnd();
  //teto
  glBegin(GL_TRIANGLES);
    glColor3d(1,0,0);
      glVertex3d( 0.0, 3.0, 0.0 );
      glVertex3d( 4.0, 3.0, 0.0 );
      glVertex3d( 2.0, 5.0, 0.0 );
  glEnd();
}
*/
