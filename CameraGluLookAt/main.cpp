#include <iostream>

using namespace std;

#include <gui.h>
#include <vector>

void global2cameraAlternativa( Vetor3D olho, Vetor3D centro, Vetor3D up ) {
    Vetor3D Oc = olho; //origem do sist local da camera
    Vetor3D kc = olho - centro; //z local da camera
    !kc; //normaliza kc (torna unitario)
    Vetor3D ic = up ^ kc; //x local da camera
    !ic; //normaliza ic (torna unitario)
    Vetor3D jc = kc ^ ic; //j local da camera
    !jc; //normaliza jc (torna unitario)

    float Tcam[16] = {
        ic.x, ic.y, ic.z, 0,
        jc.x, jc.y, jc.z, 0,
        kc.x, kc.y, kc.z, 0,
           0,    0,    0, 1
    };

    glMultTransposeMatrixf(Tcam);
    glTranslatef(-Oc.x,-Oc.y,-Oc.z);
}

void global2camera( Vetor3D olho, Vetor3D centro, Vetor3D up ) {
    Vetor3D Oc = olho; //origem do sist local da camera
    Vetor3D kc = olho - centro; //z local da camera
    !kc; //normaliza kc (torna unitario)
    Vetor3D ic = up ^ kc; //x local da camera
    !ic; //normaliza ic (torna unitario)
    Vetor3D jc = kc ^ ic; //j local da camera
    !jc; //normaliza jc (torna unitario)

    float Tcam[16] = {
        ic.x, ic.y, ic.z, ic * (Oc*-1), //t = R^T.-Oc = [ ic ]
        jc.x, jc.y, jc.z, jc * (Oc*-1), //              [ jc ].-Oc
        kc.x, kc.y, kc.z, kc * (Oc*-1), //              [ kc ]
           0,    0,    0,       1
    };

    glMultTransposeMatrixf(Tcam);
}

void camera2global( Vetor3D olho, Vetor3D centro, Vetor3D up ) {
    Vetor3D Oc = olho; //origem do sist local da camera
    Vetor3D kc = olho - centro; //z local da camera
    !kc; //normaliza kc (torna unitario)
    Vetor3D ic = up ^ kc; //x local da camera
    !ic; //normaliza ic (torna unitario)
    Vetor3D jc = kc ^ ic; //j local da camera
    !jc; //normaliza jc (torna unitario)

    float Tcam[16] = {
        ic.x, jc.x, kc.x, Oc.x,
        ic.y, jc.y, kc.y, Oc.y,
        ic.z, jc.z, kc.z, Oc.z,
           0,    0,    0,    1
    };

    glMultTransposeMatrixf(Tcam);
}

void desenha() {
    GUI::displayInit();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //efeito 3a pessoa
        //gluLookAt(0,4,7, 0,0,0, 0,1,0);
    gluLookAt(glutGUI::cam->e.x,glutGUI::cam->e.y,glutGUI::cam->e.z, glutGUI::cam->c.x,glutGUI::cam->c.y,glutGUI::cam->c.z, glutGUI::cam->u.x,glutGUI::cam->u.y,glutGUI::cam->u.z);
        //global2camera(glutGUI::cam->e,glutGUI::cam->c,glutGUI::cam->u);
        //global2cameraAlternativa(glutGUI::cam->e,glutGUI::cam->c,glutGUI::cam->u);
        camera2global(glutGUI::cam->e,glutGUI::cam->c,glutGUI::cam->u);
    //brincando com parametros do gluLookAt
        //gluLookAt(0,1,10, 0,1,0, 1,0,0);
        //gluLookAt(-5,3.5,5, -1,0,-1, 0,1,0);

    GUI::setLight(1,1,3,5,true,false);
    GUI::setLight(2,-1.5,0.5,-1,true,false);
    //GUI::setLight(3,-5,3,5,true,false);

    GUI::drawOrigin(1);

    GUI::setColor(1,0,0);
    GUI::drawFloor();

    //objeto transformado
    glPushMatrix();
        glTranslatef(glutGUI::tx,glutGUI::ty,glutGUI::tz);
        glRotatef(glutGUI::az,0,0,1);
        glRotatef(glutGUI::ay,0,1,0);
        glRotatef(glutGUI::ax,1,0,0);
        GUI::drawOrigin(1);
        glScalef(glutGUI::sx,glutGUI::sy,glutGUI::sz);
        GUI::setColor(0,0,1);
        GUI::drawBox(0,0,0, 1,1,1);
    glPopMatrix();

    //camera
    //glPushMatrix();
    //    camera2global(glutGUI::cam->e,glutGUI::cam->c,glutGUI::cam->u);
    //    GUI::drawOrigin(1);
    //    GUI::setColor(0,0,1);
    //    //GUI::drawBox(-0.5,-0.5,0, 0.5,0.5,1);
    //    GUI::drawCamera(0.5);
    //glPopMatrix();

    GUI::displayEnd();
}

void teclado(unsigned char key, int x, int y) {
    GUI::keyInit(key,x,y);

    switch (key) {
    case 't':
        glutGUI::trans_obj = !glutGUI::trans_obj;
        break;
    case 'l':
        glutGUI::trans_luz = !glutGUI::trans_luz;
        break;

    default:
        break;
    }
}

int main()
{
    cout << "Hello World!" << endl;

    GUI gui = GUI(800,600,desenha,teclado);
}


//while(true) {
//    desenha();
//    interacaoUsuario();
//}
