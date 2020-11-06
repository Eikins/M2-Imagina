// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <functional>

#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Camera.h"
#include "src/jmkdtree.h"




std::vector< Vec3 > positions;
std::vector< Vec3 > normals;

std::vector< Vec3 > positions2;
std::vector< Vec3 > normals2;
std::vector<unsigned int> triangles;


// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 640;
static unsigned int SCREENHEIGHT = 480;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static bool fullScreen = false;




// ------------------------------------------------------------------------------------------------------------
// i/o and some stuff
// ------------------------------------------------------------------------------------------------------------
void loadPN (const std::string & filename , std::vector< Vec3 > & o_positions , std::vector< Vec3 > & o_normals ) {
    unsigned int surfelSize = 6;
    FILE * in = fopen (filename.c_str (), "rb");
    if (in == NULL) {
        std::cout << filename << " is not a valid PN file." << std::endl;
        return;
    }
    size_t READ_BUFFER_SIZE = 1000; // for example...
    float * pn = new float[surfelSize*READ_BUFFER_SIZE];
    o_positions.clear ();
    o_normals.clear ();
    while (!feof (in)) {
        unsigned numOfPoints = fread (pn, 4, surfelSize*READ_BUFFER_SIZE, in);
        for (unsigned int i = 0; i < numOfPoints; i += surfelSize) {
            o_positions.push_back (Vec3 (pn[i], pn[i+1], pn[i+2]));
            o_normals.push_back (Vec3 (pn[i+3], pn[i+4], pn[i+5]));
        }

        if (numOfPoints < surfelSize*READ_BUFFER_SIZE) break;
    }
    fclose (in);
    delete [] pn;
}
void savePN (const std::string & filename , std::vector< Vec3 > const & o_positions , std::vector< Vec3 > const & o_normals ) {
    if ( o_positions.size() != o_normals.size() ) {
        std::cout << "The pointset you are trying to save does not contain the same number of points and normals." << std::endl;
        return;
    }
    FILE * outfile = fopen (filename.c_str (), "wb");
    if (outfile == NULL) {
        std::cout << filename << " is not a valid PN file." << std::endl;
        return;
    }
    for(unsigned int pIt = 0 ; pIt < o_positions.size() ; ++pIt) {
        fwrite (&(o_positions[pIt]) , sizeof(float), 3, outfile);
        fwrite (&(o_normals[pIt]) , sizeof(float), 3, outfile);
    }
    fclose (outfile);
}
void scaleAndCenter( std::vector< Vec3 > & io_positions ) {
    Vec3 bboxMin( FLT_MAX , FLT_MAX , FLT_MAX );
    Vec3 bboxMax( FLT_MIN , FLT_MIN , FLT_MIN );
    for(unsigned int pIt = 0 ; pIt < io_positions.size() ; ++pIt) {
        for( unsigned int coord = 0 ; coord < 3 ; ++coord ) {
            bboxMin[coord] = std::min<float>( bboxMin[coord] , io_positions[pIt][coord] );
            bboxMax[coord] = std::max<float>( bboxMax[coord] , io_positions[pIt][coord] );
        }
    }
    Vec3 bboxCenter = (bboxMin + bboxMax) / 2.f;
    float bboxLongestAxis = std::max<float>( bboxMax[0]-bboxMin[0] , std::max<float>( bboxMax[1]-bboxMin[1] , bboxMax[2]-bboxMin[2] ) );
    for(unsigned int pIt = 0 ; pIt < io_positions.size() ; ++pIt) {
        io_positions[pIt] = (io_positions[pIt] - bboxCenter) / bboxLongestAxis;
    }
}

void applyRandomRigidTransformation( std::vector< Vec3 > & io_positions , std::vector< Vec3 > & io_normals ) {
    srand(time(NULL));
    Mat3 R = Mat3::RandRotation();
    Vec3 t = Vec3::Rand(1.f);
    for(unsigned int pIt = 0 ; pIt < io_positions.size() ; ++pIt) {
        io_positions[pIt] = R * io_positions[pIt] + t;
        io_normals[pIt] = R * io_normals[pIt];
    }
}

void subsample( std::vector< Vec3 > & i_positions , std::vector< Vec3 > & i_normals , float minimumAmount = 0.1f , float maximumAmount = 0.2f ) {
    std::vector< Vec3 > newPos , newNormals;
    std::vector< unsigned int > indices(i_positions.size());
    for( unsigned int i = 0 ; i < indices.size() ; ++i ) indices[i] = i;
    srand(time(NULL));
    std::random_shuffle(indices.begin() , indices.end());
    unsigned int newSize = indices.size() * (minimumAmount + (maximumAmount-minimumAmount)*(float)(rand()) / (float)(RAND_MAX));
    newPos.resize( newSize );
    newNormals.resize( newSize );
    for( unsigned int i = 0 ; i < newPos.size() ; ++i ) {
        newPos[i] = i_positions[ indices[i] ];
        newNormals[i] = i_normals[ indices[i] ];
    }
    i_positions = newPos;
    i_normals = newNormals;
}

bool save( const std::string & filename , std::vector< Vec3 > & vertices , std::vector< unsigned int > & triangles ) {
    std::ofstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    myfile << "OFF" << std::endl;

    unsigned int n_vertices = vertices.size() , n_triangles = triangles.size()/3;
    myfile << n_vertices << " " << n_triangles << " 0" << std::endl;

    for( unsigned int v = 0 ; v < n_vertices ; ++v ) {
        myfile << vertices[v][0] << " " << vertices[v][1] << " " << vertices[v][2] << std::endl;
    }
    for( unsigned int f = 0 ; f < n_triangles ; ++f ) {
        myfile << 3 << " " << triangles[3*f] << " " << triangles[3*f+1] << " " << triangles[3*f+2];
        myfile << std::endl;
    }
    myfile.close();
    return true;
}



// ------------------------------------------------------------------------------------------------------------
// rendering.
// ------------------------------------------------------------------------------------------------------------

void initLight () {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
}

void drawTriangleMesh( std::vector< Vec3 > const & i_positions , std::vector< unsigned int > const & i_triangles ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_triangles.size() / 3 ; ++tIt) {
        Vec3 p0 = i_positions[i_triangles[3*tIt]];
        Vec3 p1 = i_positions[i_triangles[3*tIt+1]];
        Vec3 p2 = i_positions[i_triangles[3*tIt+2]];
        Vec3 n = Vec3::cross(p1-p0 , p2-p0);
        n.normalize();
        glNormal3f( n[0] , n[1] , n[2] );
        glVertex3f( p0[0] , p0[1] , p0[2] );
        glVertex3f( p1[0] , p1[1] , p1[2] );
        glVertex3f( p2[0] , p2[1] , p2[2] );
    }
    glEnd();
}

void drawPointSet( std::vector< Vec3 > const & i_positions , std::vector< Vec3 > const & i_normals ) {
    glBegin(GL_POINTS);
    for(unsigned int pIt = 0 ; pIt < i_positions.size() ; ++pIt) {
        glColor3f( i_normals[pIt][0] , i_normals[pIt][1] , i_normals[pIt][2] );
        glVertex3f( i_positions[pIt][0] , i_positions[pIt][1] , i_positions[pIt][2] );
    }
    glEnd();
}

void draw () {
    glColor3f(1,0.5,0.5);
    drawTriangleMesh(positions2, triangles);
    glPointSize(3);
    drawPointSet(positions2, normals2);
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    glutPostRedisplay ();
}

void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;

    case 'w':
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if(polygonMode[0] != GL_FILL)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        break;

    default:
        break;
    }
    idle ();
}

void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle ();
}

void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}

void HPSS(Vec3 inputPoint, Vec3 &outputPoint, Vec3 &outputNormal, std::vector<Vec3> const &positions,
    std::vector<Vec3> const &normals, BasicANNkdTree const &kdtree, int kernel_type, float radius, 
    unsigned int nbIterations = 10, unsigned int knn = 20)
{
    const std::array<std::function<float(float, float)>, 3> kKernels = {
        // Singular
        [] (float r, float h) -> float {
            return (h * h) / (r * r);
        },
        // Gaussian
        [] (float r, float h)  -> float {
            return std::exp(- (r * r) / (h * h));
        },
        // Wendland
        [] (float r, float h)  -> float {
            return std::pow(1.0f - r / h, 4) * (1 + 4 * r / h);
        }
    };

    std::vector<ANNidx>  neighbourIds(knn);
    std::vector<ANNdist> neighbourSqDist(knn);
    std::vector<float>   weights(knn);
    float weightSum;
    Vec3 weightedCentroid;
    Vec3 weightedNormal;

    // Variable gaussian kernel
    float maxDist;

    outputPoint = inputPoint;

    for (unsigned int i = 0; i < nbIterations; i++)
    {
        kdtree.knearest(inputPoint, knn, neighbourIds.data(), neighbourSqDist.data());

        weightSum = 0.0F;
        weightedCentroid = { 0.0F, 0.0F, 0.0F };
        weightedNormal = { 0.0F, 0.0F, 0.0F };

        // Get the maximum distance to adapt the kernel sizes
        maxDist = std::sqrt(neighbourSqDist[knn - 1]);

        // Compute the weights, and the sums
        for (unsigned int k = 0; k < knn; k++)
        {
            Vec3 position = positions[neighbourIds[k]];
            Vec3 normal = normals[neighbourIds[k]];
            // Not optimal but needed for wendland support
            // For gaussian and singular, we can directly use the squared distance
            float r = std::sqrt(neighbourSqDist[k]);

            weights[k] = kKernels[kernel_type](r, maxDist * radius);
            weightSum += weights[k];

            // Project the point on the plane defined by (point, normal)
            weightedCentroid += weights[k] * (outputPoint - Vec3::dot(outputPoint - position, normal) * normal);
            weightedNormal += weights[k] * normal;
        }
        
        weightedNormal.normalize();

        // We can create a project function but we can just inline here because
        // We don't have a lot of usages
        outputPoint = weightedCentroid / weightSum;
        outputNormal = weightedNormal;
    }
}

void APSS(Vec3 inputPoint, Vec3 &outputPoint, Vec3 &outputNormal, std::vector<Vec3> const &positions,
    std::vector<Vec3> const &normals, BasicANNkdTree const &kdtree, int kernel_type, float radius, 
    unsigned int nbIterations = 10, unsigned int knn = 20)
{
}

void DisplaceWithNoise(std::vector<Vec3> &positions, const std::vector<Vec3> &normals, float magnitude)
{
    for (unsigned int i = 0; i < positions.size(); i++)
    {
        // Generate a displacement in [0, 1]
        float displacement = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        // Remap to [-mag, mag]
        displacement = magnitude * (2 * displacement - 1);
        positions[i] += displacement * normals[i]; 
    }
}

float SurfaceSDFWithHPSS(
    Vec3 inputPoint, 
    std::vector<Vec3> const &positions, 
    std::vector<Vec3> const &normals, 
    BasicANNkdTree const &kdtree, 
    int kernel_type = 1.0, 
    float radius = 15.0F, 
    unsigned int nbIterations = 10, 
    unsigned int knn = 20
)
{
    Vec3 projPos, projNormal;
    HPSS(inputPoint, projPos, projNormal, positions, normals, kdtree, kernel_type, radius, nbIterations, knn);
    return Vec3::dot(inputPoint - projPos, projNormal);
}

#define POINT_INDEX(res, x, y, z) (res) * (res) * (x) + (res) * (y) + (z)

void PointGrid(unsigned res, Vec3 start, Vec3 end, std::vector<Vec3> &outputPoints)
{
    outputPoints.resize(res * res * res, Vec3());
    Vec3 boxSize = end - start;

    for (unsigned x = 0; x < res; x++)
    {
        for (unsigned y = 0; y < res; y++)
        {
            for (unsigned z = 0; z < res; z++)
            {
                Vec3 offset(
                    x * boxSize[0] / res,
                    y * boxSize[1] / res,
                    z * boxSize[2] / res
                );
                outputPoints[POINT_INDEX(res, x, y, z)] = start + offset;
            }   
        } 
    }
}

void DualContouring(
    unsigned res,
    Vec3 start,
    Vec3 end,
    std::vector<Vec3> &outputPoints,
    std::vector<Vec3> &outputNormals,
    std::vector<unsigned int> &outputTriangles,
    const std::vector<Vec3> &positions,
    const std::vector<Vec3> &normals,
    const BasicANNkdTree &kdtree
)
{
    outputPoints.clear();
    outputNormals.clear();
    outputTriangles.clear();

    std::vector<Vec3> grid;
    std::vector<float> sdf;
    
    PointGrid(res, start, end, grid);

    // Contains indices of surface points
    // An empty cell will have an index of -1
    // This will be used for second pass when we will create faces
    std::vector<int> cells;
    cells.resize(grid.size(), -1);

    // Fill with SDF
    sdf.reserve(grid.size());
    for (unsigned i = 0; i < grid.size(); i++)
    {
        sdf.push_back(SurfaceSDFWithHPSS(grid[i], positions, normals, kdtree));
    }

    // First pass, compute vertices
    for (unsigned x = 0; x < res - 1; x++)
    {
        for (unsigned y = 0; y < res - 1; y++)
        {
            for (unsigned z = 0; z < res - 1; z++)
            {
                // For each grid corner
                std::array<Vec3, 8> cornerPositions = 
                {
                    grid[POINT_INDEX(res, x, y, z)],
                    grid[POINT_INDEX(res, x + 1, y, z)],
                    grid[POINT_INDEX(res, x, y + 1, z)],
                    grid[POINT_INDEX(res, x, y, z + 1)],
                    grid[POINT_INDEX(res, x + 1, y + 1, z)],
                    grid[POINT_INDEX(res, x, y + 1, z + 1)],
                    grid[POINT_INDEX(res, x + 1, y, z + 1)],
                    grid[POINT_INDEX(res, x + 1, y + 1, z + 1)]
                };

                std::array<float, 8> cornerDistances = 
                {
                    sdf[POINT_INDEX(res, x, y, z)],
                    sdf[POINT_INDEX(res, x + 1, y, z)],
                    sdf[POINT_INDEX(res, x, y + 1, z)],
                    sdf[POINT_INDEX(res, x, y, z + 1)],
                    sdf[POINT_INDEX(res, x + 1, y + 1, z)],
                    sdf[POINT_INDEX(res, x, y + 1, z + 1)],
                    sdf[POINT_INDEX(res, x + 1, y, z + 1)],
                    sdf[POINT_INDEX(res, x + 1, y + 1, z + 1)]
                };

                // Store corner sdf sign count to check if there is a change
                unsigned negativeCount = 0;
                for (unsigned i = 0; i < 8; i++)
                {
                    if (cornerDistances[i] <= 0.0F) negativeCount++;
                }

                if (negativeCount == 0 || negativeCount == 8)
                {
                    // If there are no sign change, ignore
                    continue;
                }

                Vec3 cellPoint = (cornerPositions[0] + cornerPositions[7]) / 2;
                Vec3 cellNormal; // Used when performing triangle reconstruction
                // Optional : project the point on the MLS surface to have a better precision
                HPSS(cellPoint, cellPoint, cellNormal, positions, normals, kdtree, 1, 15.0F);

                cells[POINT_INDEX(res, x, y, z)] = outputPoints.size();

                outputPoints.push_back(cellPoint);
                outputNormals.push_back(cellNormal);
            }   
        }
    }

    // Second pass, generate triangles
    for (unsigned x = 0; x < res - 1; x++)
    {
        for (unsigned y = 0; y < res - 1; y++)
        {
            for (unsigned z = 0; z < res - 1; z++)
            {
                std::array<int, 8> connectedCells = 
                {
                    cells[POINT_INDEX(res, x, y, z)],
                    cells[POINT_INDEX(res, x + 1, y, z)],
                    cells[POINT_INDEX(res, x, y + 1, z)],
                    cells[POINT_INDEX(res, x, y, z + 1)],
                    cells[POINT_INDEX(res, x + 1, y + 1, z)],
                    cells[POINT_INDEX(res, x, y + 1, z + 1)],
                    cells[POINT_INDEX(res, x + 1, y, z + 1)],
                    cells[POINT_INDEX(res, x + 1, y + 1, z + 1)]
                };

                // On face defines the four neighbours of the edges from the grid
                // The faces comes from our neighbour indexing
                //   5 ----- 7
                //  /|      /|
                // 3 ----- 6 |
                // | 2 ----| 4
                // |/      |/
                // 0 ----- 1
                // 
                // z y
                // |/
                // o-- x

                const std::array<std::array<unsigned int, 4>, 6> faces = 
                {{
                    {0, 1, 6, 3},
                    {3, 6, 7, 5},
                    {2, 4, 7, 5},
                    {1, 4, 7, 6},
                    {0, 2, 5, 3},
                    {0, 1, 4, 2}
                }};

                for (auto face : faces)
                {
                    // For each face, (4 neighbours of an edge)
                    // If there are 4 points, then connect them
                    unsigned neighbourCount = 0;
                    for (unsigned i = 0; i < 4; i++)
                    {
                        if (connectedCells[face[i]] != -1)
                        {
                            neighbourCount++;
                        }
                    }

                    // std::cout << "[" << connectedCells[face[0]] << ", " << connectedCells[face[1]] << ", " << connectedCells[face[2]] << ", " << connectedCells[face[3]] << "]" << std::endl;

                    if (neighbourCount == 4)
                    {
                        // Create two triangles, and check they're facing outside using surface normal
                        unsigned int i0 = connectedCells[face[0]];
                        unsigned int i1 = connectedCells[face[1]];
                        unsigned int i2 = connectedCells[face[2]];
                        unsigned int i3 = connectedCells[face[3]];

                        // First triangle
                        // Check winding order
                        // We will use the first vertex normal, we will assume we won't have
                        // normal discontinuities here
                        Vec3 n1ref = outputNormals[i0] + outputNormals[i1] + outputNormals[i2];
                        Vec3 n2ref = outputNormals[i0] + outputNormals[i2] + outputNormals[i3];

                        Vec3 v0 = outputPoints[i0];
                        Vec3 v1 = outputPoints[i1];
                        Vec3 v2 = outputPoints[i2];
                        Vec3 v3 = outputPoints[i3];

                        // First triangle normal
                        Vec3 n1 = Vec3::cross(v1-v0, v2-v0);
                        // Second triangle normal
                        Vec3 n2 = Vec3::cross(v3-v2, v0-v2);

                        // First triangle
                        if (Vec3::dot(n1, n1ref) >= 0)
                        {
                            // Triangle has a good winding order
                            outputTriangles.push_back(i0);
                            outputTriangles.push_back(i1);
                            outputTriangles.push_back(i2);
                        }
                        else
                        {
                            // Reverse winding order
                            outputTriangles.push_back(i0);
                            outputTriangles.push_back(i2);
                            outputTriangles.push_back(i1);
                        }
                        
                        // Second triangle
                        if (Vec3::dot(n2, n2ref) >= 0)
                        {
                            // Triangle has a good winding order
                            outputTriangles.push_back(i2);
                            outputTriangles.push_back(i3);
                            outputTriangles.push_back(i0);
                        }
                        else
                        {
                            // Reverse winding order
                            outputTriangles.push_back(i2);
                            outputTriangles.push_back(i0);
                            outputTriangles.push_back(i3);
                        }
                    }

                }
            }
        }
    }
}

int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("tp point processing");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);


    {
        // Load a first pointset, and build a kd-tree:
        loadPN("pointsets/igea.pn" , positions , normals);

        BasicANNkdTree kdtree;
        kdtree.build(positions);

        const unsigned gridRes = 16 + 1;
        DualContouring(gridRes, Vec3(-0.6F, -0.6F, -0.6F), Vec3(0.6F, 0.6F, 0.6F), positions2, normals2, triangles, positions, normals, kdtree);
    }

    glutMainLoop ();
    return EXIT_SUCCESS;
}

