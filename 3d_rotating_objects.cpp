python --versionpython --version/*
 * ============================================================
 *   3D Rotating Objects - Computer Graphics Project
 *   Using OpenGL + GLUT (freeglut)
 *   Platform : Windows (Code::Blocks / Visual Studio)
 *   Author   : Mahir
 *
 *   Objects   : Cube, Sphere, Cone, Torus
 *   Features  : Lighting, Materials, Fog, Keyboard Control,
 *               Mouse Drag Rotation, Animation
 * ============================================================
 *
 *  SETUP (Code::Blocks):
 *   1. Download freeglut for Windows from:
 *      https://www.transmissionzero.co.uk/software/freeglut-devel/
 *   2. Copy freeglut.dll  -> your project folder (or C:\Windows\System32)
 *   3. Settings > Compiler > Search directories > Add:  freeglut\include
 *   4. Settings > Linker   > Add libraries: freeglut, opengl32, glu32
 *   5. Compile & Run!
 *
 *  CONTROLS:
 *   Arrow Keys      - Rotate scene
 *   W / S           - Zoom In / Out
 *   1 / 2 / 3 / 4  - Select object (Cube/Sphere/Cone/Torus)
 *   L               - Toggle lighting on/off
 *   F               - Toggle fog on/off
 *   SPACE           - Pause/Resume rotation
 *   R               - Reset camera
 *   ESC             - Exit
 *   Mouse Drag      - Rotate scene freely
 * ============================================================
 */

#include <GL/glut.h>
#include <cmath>
#include <cstring>
#include <cstdio>

/* ── Constants ─────────────────────────────────────────────── */
#define PI 3.14159265358979323846f

/* ── Camera / scene state ───────────────────────────────────── */
static float rotX      = 20.0f;
static float rotY      = 0.0f;
static float zoom      = -6.0f;
static int   mouseLastX = 0;
static int   mouseLastY = 0;
static bool  mouseDown  = false;

/* ── Animation ──────────────────────────────────────────────── */
static float angle       = 0.0f;
static bool  paused      = false;
static int   selectedObj = 0;   /* 0=all, 1=Cube, 2=Sphere, 3=Cone, 4=Torus */

/* ── Toggles ────────────────────────────────────────────────── */
static bool lightingOn = true;
static bool fogOn      = false;

/* ── Window ─────────────────────────────────────────────────── */
static int winW = 900, winH = 650;

/* ══════════════════════════════════════════════════════════════
 *  MATERIAL PRESETS
 * ══════════════════════════════════════════════════════════════ */
struct Material {
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
};

static const Material MAT_RUBY = {
    {0.17f, 0.01f, 0.01f, 1.0f},
    {0.61f, 0.04f, 0.04f, 1.0f},
    {0.73f, 0.63f, 0.63f, 1.0f},
    76.8f
};
static const Material MAT_GOLD = {
    {0.25f, 0.20f, 0.07f, 1.0f},
    {0.75f, 0.61f, 0.23f, 1.0f},
    {0.63f, 0.56f, 0.37f, 1.0f},
    51.2f
};
static const Material MAT_CYAN_PLASTIC = {
    {0.00f, 0.10f, 0.06f, 1.0f},
    {0.00f, 0.51f, 0.51f, 1.0f},
    {0.50f, 0.70f, 0.70f, 1.0f},
    32.0f
};
static const Material MAT_SILVER = {
    {0.19f, 0.19f, 0.19f, 1.0f},
    {0.51f, 0.51f, 0.51f, 1.0f},
    {0.51f, 0.51f, 0.51f, 1.0f},
    51.2f
};

static void applyMaterial(const Material& m) {
    glMaterialfv(GL_FRONT, GL_AMBIENT,   m.ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   m.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  m.specular);
    glMaterialf (GL_FRONT, GL_SHININESS, m.shininess);
}

/* ══════════════════════════════════════════════════════════════
 *  DRAW HELPERS
 * ══════════════════════════════════════════════════════════════ */

/* Draw a wire grid floor */
static void drawFloor() {
    glDisable(GL_LIGHTING);
    glColor3f(0.30f, 0.30f, 0.35f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -5; i <= 5; i++) {
        glVertex3f((float)i, -2.0f, -5.0f);
        glVertex3f((float)i, -2.0f,  5.0f);
        glVertex3f(-5.0f, -2.0f, (float)i);
        glVertex3f( 5.0f, -2.0f, (float)i);
    }
    glEnd();
    if (lightingOn) glEnable(GL_LIGHTING);
}

/* Axis indicator (bottom-left corner, small) */
static void drawAxes(float len) {
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
        /* X - Red */
        glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(len,0,0);
        /* Y - Green */
        glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,len,0);
        /* Z - Blue */
        glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,len);
    glEnd();
    if (lightingOn) glEnable(GL_LIGHTING);
}

/* ── Draw a smooth custom cube with normals ───────────────── */
static void drawCubeWithNormals(float s) {
    float h = s * 0.5f;
    /* 6 faces */
    static const float normals[6][3] = {
        { 0, 0, 1}, { 0, 0,-1},
        { 0, 1, 0}, { 0,-1, 0},
        { 1, 0, 0}, {-1, 0, 0}
    };
    static const float verts[6][4][3] = {
        /* front */
        {{-1,-1, 1},{ 1,-1, 1},{ 1, 1, 1},{-1, 1, 1}},
        /* back */
        {{ 1,-1,-1},{-1,-1,-1},{-1, 1,-1},{ 1, 1,-1}},
        /* top */
        {{-1, 1, 1},{ 1, 1, 1},{ 1, 1,-1},{-1, 1,-1}},
        /* bottom */
        {{-1,-1,-1},{ 1,-1,-1},{ 1,-1, 1},{-1,-1, 1}},
        /* right */
        {{ 1,-1, 1},{ 1,-1,-1},{ 1, 1,-1},{ 1, 1, 1}},
        /* left */
        {{-1,-1,-1},{-1,-1, 1},{-1, 1, 1},{-1, 1,-1}}
    };
    glBegin(GL_QUADS);
    for (int f = 0; f < 6; f++) {
        glNormal3fv(normals[f]);
        for (int v = 0; v < 4; v++) {
            glVertex3f(verts[f][v][0]*h,
                       verts[f][v][1]*h,
                       verts[f][v][2]*h);
        }
    }
    glEnd();
}

/* ══════════════════════════════════════════════════════════════
 *  LIGHTING SETUP
 * ══════════════════════════════════════════════════════════════ */
static void setupLighting() {
    glEnable(GL_LIGHTING);

    /* Light 0 - main white light (top-right) */
    float pos0[] = { 4.0f, 6.0f, 4.0f, 1.0f };
    float amb0[] = { 0.15f, 0.15f, 0.15f, 1.0f };
    float dif0[] = { 1.0f,  1.0f,  1.0f,  1.0f };
    float spe0[] = { 1.0f,  1.0f,  1.0f,  1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spe0);
    glEnable(GL_LIGHT0);

    /* Light 1 - blue fill light (left) */
    float pos1[] = {-5.0f, 2.0f, 2.0f, 1.0f };
    float dif1[] = { 0.20f, 0.20f, 0.60f, 1.0f };
    float spe1[] = { 0.10f, 0.10f, 0.30f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  dif1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spe1);
    glEnable(GL_LIGHT1);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}

/* ══════════════════════════════════════════════════════════════
 *  HUD TEXT
 * ══════════════════════════════════════════════════════════════ */
static void drawBitmapString(float x, float y, const char* str) {
    glRasterPos2f(x, y);
    for (const char* c = str; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
}

static void drawHUD() {
    /* Switch to 2D orthographic projection */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* Semi-transparent black background strip at top */
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glVertex2f(0,     (float)winH);
        glVertex2f((float)winW, (float)winH);
        glVertex2f((float)winW, (float)winH - 68);
        glVertex2f(0,     (float)winH - 68);
    glEnd();
    glDisable(GL_BLEND);

    /* Title */
    glColor3f(1.0f, 0.85f, 0.20f);
    drawBitmapString(10, winH - 20, "3D ROTATING OBJECTS  |  Computer Graphics Project");

    /* Controls row 1 */
    glColor3f(0.75f, 0.95f, 1.0f);
    drawBitmapString(10, winH - 40,
        "Arrows:Rotate  W/S:Zoom  1-4:Select  SPACE:Pause  L:Light  F:Fog  R:Reset  ESC:Exit");

    /* Status */
    char status[128];
    const char* objName[] = {"ALL", "Cube", "Sphere", "Cone", "Torus"};
    sprintf(status, "Selected: %s  |  Lighting: %s  |  Fog: %s  |  Zoom: %.1f",
            objName[selectedObj],
            lightingOn ? "ON" : "OFF",
            fogOn      ? "ON" : "OFF",
            -zoom);
    glColor3f(0.60f, 1.0f, 0.65f);
    drawBitmapString(10, winH - 58, status);

    glEnable(GL_DEPTH_TEST);
    if (lightingOn) glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* ══════════════════════════════════════════════════════════════
 *  DRAW SCENE
 * ══════════════════════════════════════════════════════════════ */
static void drawScene() {
    /* ── Cube (top-left) ─────────────────────────────────────── */
    glPushMatrix();
        glTranslatef(-2.5f, 0.5f, 0.0f);
        if (selectedObj == 0 || selectedObj == 1)
            glRotatef(angle, 1.0f, 1.0f, 0.3f);
        applyMaterial(MAT_RUBY);
        drawCubeWithNormals(1.4f);

        /* Wireframe overlay */
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.4f, 0.4f);
        glLineWidth(1.2f);
        glutWireCube(1.41f);
        if (lightingOn) glEnable(GL_LIGHTING);

        /* Label */
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.8f, 0.8f);
        glRasterPos3f(-0.3f, -1.1f, 0.0f);
        const char* cubeLabel = "CUBE";
        for (const char* c = cubeLabel; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        if (lightingOn) glEnable(GL_LIGHTING);
    glPopMatrix();

    /* ── Sphere (top-right) ──────────────────────────────────── */
    glPushMatrix();
        glTranslatef(2.5f, 0.5f, 0.0f);
        if (selectedObj == 0 || selectedObj == 2)
            glRotatef(angle * 1.3f, 0.5f, 1.0f, 0.5f);
        applyMaterial(MAT_GOLD);
        glutSolidSphere(0.9, 48, 48);

        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.85f, 0.30f);
        glLineWidth(0.8f);
        glutWireSphere(0.91, 16, 16);
        if (lightingOn) glEnable(GL_LIGHTING);

        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.95f, 0.70f);
        glRasterPos3f(-0.42f, -1.1f, 0.0f);
        const char* sphereLabel = "SPHERE";
        for (const char* c = sphereLabel; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        if (lightingOn) glEnable(GL_LIGHTING);
    glPopMatrix();

    /* ── Cone (bottom-left) ──────────────────────────────────── */
    glPushMatrix();
        glTranslatef(-2.5f, -1.5f, 0.0f);
        if (selectedObj == 0 || selectedObj == 3)
            glRotatef(angle * 0.8f, 0.2f, 1.0f, 0.4f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);  /* point upward */
        applyMaterial(MAT_CYAN_PLASTIC);
        glutSolidCone(0.75, 1.6, 32, 16);

        glDisable(GL_LIGHTING);
        glColor3f(0.20f, 0.80f, 0.80f);
        glLineWidth(0.8f);
        glutWireCone(0.76, 1.61, 16, 8);
        if (lightingOn) glEnable(GL_LIGHTING);

        glDisable(GL_LIGHTING);
        glColor3f(0.60f, 1.0f, 1.0f);
        glRasterPos3f(-0.35f, 1.8f, 0.0f);
        const char* coneLabel = "CONE";
        for (const char* c = coneLabel; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        if (lightingOn) glEnable(GL_LIGHTING);
    glPopMatrix();

    /* ── Torus (bottom-right) ────────────────────────────────── */
    glPushMatrix();
        glTranslatef(2.5f, -1.5f, 0.0f);
        if (selectedObj == 0 || selectedObj == 4)
            glRotatef(angle * 1.1f, 1.0f, 0.5f, 1.0f);
        applyMaterial(MAT_SILVER);
        glutSolidTorus(0.28, 0.70, 32, 64);

        glDisable(GL_LIGHTING);
        glColor3f(0.75f, 0.75f, 0.80f);
        glLineWidth(0.8f);
        glutWireTorus(0.285, 0.705, 12, 24);
        if (lightingOn) glEnable(GL_LIGHTING);

        glDisable(GL_LIGHTING);
        glColor3f(0.85f, 0.85f, 0.95f);
        glRasterPos3f(-0.42f, -1.1f, 0.0f);
        const char* torusLabel = "TORUS";
        for (const char* c = torusLabel; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        if (lightingOn) glEnable(GL_LIGHTING);
    glPopMatrix();
}

/* ══════════════════════════════════════════════════════════════
 *  GLUT CALLBACKS
 * ══════════════════════════════════════════════════════════════ */

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Camera */
    gluLookAt(0, 0, -zoom,
              0, 0, 0,
              0, 1, 0);

    /* Scene rotation from keyboard/mouse */
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    /* Setup lighting after camera */
    if (lightingOn)
        setupLighting();

    /* Fog */
    if (fogOn) {
        glEnable(GL_FOG);
        float fogColor[] = {0.05f, 0.05f, 0.10f, 1.0f};
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_START, 4.0f);
        glFogf(GL_FOG_END,   12.0f);
        glFogi(GL_FOG_MODE,  GL_LINEAR);
    } else {
        glDisable(GL_FOG);
    }

    /* Floor grid */
    drawFloor();

    /* Axis indicator */
    glPushMatrix();
        glTranslatef(-4.5f, -1.8f, 0.0f);
        drawAxes(0.6f);
    glPopMatrix();

    /* Main 3D objects */
    drawScene();

    /* HUD (2D overlay — drawn last) */
    drawHUD();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW = w; winH = (h == 0 ? 1 : h);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)winW / winH, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int) {
    if (!paused) {
        angle += 0.6f;
        if (angle >= 360.0f) angle -= 360.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);   /* ~60 fps */
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'w': case 'W': zoom += 0.3f; if (zoom > -2.0f) zoom = -2.0f; break;
        case 's': case 'S': zoom -= 0.3f; if (zoom < -15.0f) zoom = -15.0f; break;
        case ' ':           paused = !paused; break;
        case 'l': case 'L':
            lightingOn = !lightingOn;
            if (!lightingOn) glDisable(GL_LIGHTING);
            break;
        case 'f': case 'F': fogOn = !fogOn; break;
        case 'r': case 'R': rotX = 20.0f; rotY = 0.0f; zoom = -6.0f; break;
        case '1': selectedObj = 1; break;
        case '2': selectedObj = 2; break;
        case '3': selectedObj = 3; break;
        case '4': selectedObj = 4; break;
        case '0': selectedObj = 0; break;
        case 27:            exit(0); break;  /* ESC */
    }
    glutPostRedisplay();
}

void specialKeys(int key, int, int) {
    const float step = 2.5f;
    switch (key) {
        case GLUT_KEY_UP:    rotX -= step; break;
        case GLUT_KEY_DOWN:  rotX += step; break;
        case GLUT_KEY_LEFT:  rotY -= step; break;
        case GLUT_KEY_RIGHT: rotY += step; break;
    }
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        mouseDown = (state == GLUT_DOWN);
        mouseLastX = x;
        mouseLastY = y;
    }
    /* Scroll wheel zoom */
    if (button == 3) { zoom += 0.3f; if (zoom > -2.0f) zoom = -2.0f; glutPostRedisplay(); }
    if (button == 4) { zoom -= 0.3f; if (zoom < -15.0f) zoom = -15.0f; glutPostRedisplay(); }
}

void mouseMotion(int x, int y) {
    if (mouseDown) {
        rotY += (x - mouseLastX) * 0.5f;
        rotX += (y - mouseLastY) * 0.5f;
        mouseLastX = x;
        mouseLastY = y;
        glutPostRedisplay();
    }
}

/* ══════════════════════════════════════════════════════════════
 *  MAIN
 * ══════════════════════════════════════════════════════════════ */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("3D Rotating Objects  |  Computer Graphics Project  |  Mahir");

    /* OpenGL global state */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    /* Register callbacks */
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
