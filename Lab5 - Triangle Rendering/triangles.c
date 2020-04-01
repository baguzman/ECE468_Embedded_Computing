#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

//CONSTANTS
#define PI 3.14

//HELPER FUNCTIONS
void readVerticesNFaces(FILE *fin,int **faces,float **vertices, unsigned int v_count, unsigned int f_count);
void bounds(float **vertices_arr,unsigned int v_count, float min[],float max[],float center[], float *E);
void rotate(float v[],float R[][3]);
void subtract(float x[],float y[], float z[]);
void crossProduct(float v1[], float v2[], float z[]);
void scale(float scale, float v[], float prod[]);
void vadd(float v1[], float v2[], float sum[]);
float dotProduct(float v1[], float v2[]);
void vprint(float v[]); //Debugging helper
void dots(void);

int main(int argc, char *argv[])
{
    FILE *fin;
    unsigned int *face_count, *vertex_count;
    char buffer[1000];
    int i = 0;
    float xangle, y, z;

    if(argc != 5)
    {
        printf("Usage: ./triangles <input PLY file> <angle1> <angle2> <angle3>\n");
        exit(1);
    }

    fin = fopen(argv[1],"r");
    if(fin == NULL)
    {
        printf("ERROR: Could not open %s\n",argv[1]);
        exit(1);
    }
    printf("Processing...\n");
    xangle = (M_PI/180) * (-1 * atof(argv[2]));
    y = (M_PI/180) * (-1 * atof(argv[3]));
    z = (M_PI/180) * atof(argv[4]);
    float xaxis[3][3] = {{1, 0, 0}, {0, cos(xangle), -sin(xangle)}, {0, sin(xangle), cos(xangle)}};
    float yaxis[3][3] = {{cos(y), 0, sin(y)}, {0, 1, 0}, {-sin(y), 0, cos(y)}};
    float zaxis[3][3] = {{cos(z), -sin(z),0}, {sin(z), cos(z), 0}, {0, 0, 1}};

    //ALLOCATE MEMORY FOR NUMBER OF FACES AND VERTICES
    face_count = (unsigned int *)calloc(1,sizeof(unsigned int));
    vertex_count = (unsigned int *)calloc(1,sizeof(unsigned int));

    //PARSE HEADER TO GET NUMBER OF VERTICES AND FACES
    while(1)
    {
        if(strstr(buffer,"element vertex"))
        {
            sscanf(buffer,"element vertex %u",vertex_count);
        }
        if(strstr(buffer,"element face"))
        {
            sscanf(buffer,"element face %u",face_count);
        }
        i++;
        if(strstr(buffer,"end_header"))
        {
            break;
        }
        else
        {
            fgets(buffer,sizeof(buffer),fin);
        }
    }
    //DEBUGGING PRINT STATEMENTS
    //printf("%d\n",*vertex_count);
    //printf("%d\n",*face_count);

    unsigned int v_count = *vertex_count;
    unsigned int f_count = *face_count;

    float **vertices_arr;
    int **face_arr;

    vertices_arr = (float **)calloc(1,v_count * sizeof(float *));
    for(i = 0; i < v_count; i++)
    {
        vertices_arr[i] = (float *)calloc(1,3 * sizeof(float));
    }

    face_arr = (int **)calloc(1,f_count * sizeof(int *));
    for(i = 0; i < f_count; i++)
    {
        face_arr[i] = (int *)calloc(1,3 * sizeof(int));
    }

    //READ VERTICES AND FACES INTO SEPARATE ARRAYS
    readVerticesNFaces(fin,face_arr,vertices_arr,v_count,f_count);

    //FIND BOUNDS OF IMAGE
    float min[3],max[3],center[3],E;
    bounds(vertices_arr,v_count,min,max,center,&E);
    //vprint(min);
    //vprint(max);
    //vprint(center);
    //CALCULATE CAMERA VECTOR POSITION
    float cam[3] = {1, 0, 0};

    rotate(cam,xaxis);
    rotate(cam,yaxis);
    rotate(cam,zaxis);
    for(i = 0; i < 3; i++)
    {
        cam[i] = (1.5 * E * cam[i])+ center[i];
    }

    //ROTATE UP VECTOR
    float up[3] = {0, 0, 1};
    rotate(up,xaxis);
    rotate(up,yaxis);
    rotate(up,zaxis);

    //3D BOUNDING COORDINATES
    float left[3], a, right[3], top[3], bottom[3], topleft[3];
    float center_cam_diff[3], prod[3];

    subtract(center, cam, center_cam_diff);
    crossProduct(up, center_cam_diff, left);

    a = sqrt(pow(left[0], 2) + pow(left[1], 2) + pow(left[2], 2));

    scale(E/(2*a), left, prod);
    vadd(prod,center,left);
    subtract(center, cam, center_cam_diff);
    crossProduct(center_cam_diff, up, right);

    scale(E/(2*a), right, prod);
    vadd(prod, center, right);

    scale(E/2, up, prod);
    vadd(prod, center, top);

    scale((-1 * E)/2, up, prod);
    vadd(prod, center, bottom);

    scale(E/2, up, prod);
    vadd(prod, left, topleft);
/*
    printf("left:\t");
    vprint(left);
    printf("right:\t");
    vprint(right);
    printf("top:\t");
    vprint(top);
    printf("bottom:\t");
    vprint(bottom);
    printf("topleft:\t");
    vprint(topleft);
*/
    //6
    int r = 256, c = 256, x, j;
    float zbuffer = 999999;
    float image_beam[3];
    float plane[4];
    float *v, *v1, *v2;
    float temp[3],temp2[3];
    float n, d, dot1, dot2, dot3;
    float rayPlaneIntersection[3],temp3[3],temp4[3];
    float prod2[3];
    unsigned char displayImage[256][256] = {0};
    unsigned short colorMax = 0; //keeping track of max color value
    //int count = 1;
    for(i = 0; i < r; i++)
    {
        for(j = 0; j < c; j++)
        {
            zbuffer = 999999;
            //FIND BEAM COORDINATES
            for(x = 0; x < 3; x++)
            {
                image_beam[x] = topleft[x] + (((float)j / (c-1)) * (right[x] - left[x])) + (((float)i / (r-1)) * (bottom[x] - top[x]));
            }
            for(x = 0; x < f_count; x++)
            {
                v = vertices_arr[face_arr[x][0]];
                v1 = vertices_arr[face_arr[x][1]];
                v2 = vertices_arr[face_arr[x][2]];

                subtract(v1,v,temp);
                subtract(v2,v,temp2);
                crossProduct(temp,temp2,plane);
                scale(-1,plane,prod);
                plane[3] = dotProduct(prod,v);
                scale(-1,plane,prod);
                n = dotProduct(prod,cam);
                n = n - plane[3];
                subtract(image_beam,cam,temp);
                d = dotProduct(plane,temp);
                if(fabs(d) < 0.00000005)
                {
                    continue;
                }

                //CALCULATE DOT1, DOT2 AND DOT3 USING GIVEN EQUATIONS
                subtract(image_beam,cam,temp);
                scale(n/d,temp,prod);
                vadd(cam,prod,rayPlaneIntersection);
                subtract(v2,v,temp);
                subtract(v1,v,temp2);
                subtract(rayPlaneIntersection,v,temp3);
                subtract(v1,v,temp4);
                crossProduct(temp,temp2,prod);
                crossProduct(temp3,temp4,prod2);
                dot1 = dotProduct(prod,prod2);

                subtract(v,v1,temp);
                subtract(v2,v1,temp2);
                subtract(rayPlaneIntersection,v1,temp3);
                subtract(v2,v1,temp4);
                crossProduct(temp,temp2,prod);
                crossProduct(temp3,temp4,prod2);
                dot2 = dotProduct(prod,prod2);

                subtract(v1,v2,temp);
                subtract(v,v2,temp2);
                subtract(rayPlaneIntersection,v2,temp3);
                subtract(v,v2,temp4);
                crossProduct(temp,temp2,prod);
                crossProduct(temp3,temp4,prod2);
                dot3 = dotProduct(prod,prod2);

                if(dot1 < 0 || dot2 < 0 || dot3 < 0)
                {
                    continue;
                }
                else if((n/d) > zbuffer)
                {
                    continue;
                }
                else
                {
                    zbuffer = n/d;
                    displayImage[i][j] = 155 + (x%100);
                    if(displayImage[i][j] > colorMax)
                    {
                        colorMax = displayImage[i][j];
                    }
                }
            }
        }
    }
    //WRITE PPM IMAGE
    FILE *fout;
    fout = fopen("output.ppm","wb");
    fprintf(fout,"P5 %d %d %d ",c,r,colorMax);
    fwrite(displayImage,sizeof(unsigned char), r*c, fout);
    fclose(fin);
    fclose(fout);


    return(0);
}

void dots()
{
    printf(".");
}
void vprint(float v[])
{
    int i;
    printf("[");
    for(i=0; i < 3; i++)
    {
        printf("%f ",v[i]);
    }
    printf("]\n");
}
//DOT PRODUCT OF TWO VECTORS
float dotProduct(float v1[], float v2[])
{
    int i;
    float dotProd = 0;
    for(i = 0; i < 3; i++)
    {
        dotProd += v1[i] * v2[i];
    }
    return(dotProd);
}
//ADD TWO VECTORS
void vadd(float v1[], float v2[], float sum[])
{
    int i = 0;
    for(i = 0; i < 3; i++)
    {
        sum[i] = v1[i] + v2[i];
    }
}
//SCALER * VECTOR
void scale(float scale, float v[], float prod[])
{
    int i;
    for(i = 0; i < 3; i++)
    {
        prod[i] = v[i] * scale;
    }
}
//HELPER FUNCTION TO COMPUTER CROSS PRODUCTS
void crossProduct(float v1[], float v2[], float z[])
{
    z[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    z[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    z[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}
//HELPER FUNCTION TO ROTATE VECTORS
void rotate(float v[],float R[][3])
{
    int i,j;
    float temp[3] = {0};
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            temp[i] += R[j][i] * v[j];
        }
    }
    for(i = 0; i < 3; i++)
    {
        v[i] = temp[i];
    }
}

//HELPER FUNCTION TO FIND BOUNDS OF IMAGE
void bounds(float **vertices_arr,unsigned int v_count, float min[],float max[],float center[], float *E)
{
    int i;
    float minx = FLT_MAX, miny = FLT_MAX, minz = FLT_MAX;
    float maxx = 0, maxy = 0, maxz = 0;
    float diff[3];

    //FIND MINS AND MAXS FOR EACH AXIS
    for(i = 0; i < v_count; i++)
    {
        if(vertices_arr[i][0] < minx)
        {
            minx = vertices_arr[i][0];
        }
        if(vertices_arr[i][1] < miny)
        {
            miny = vertices_arr[i][1];
        }
        if(vertices_arr[i][2] < minz)
        {
            minz = vertices_arr[i][2];
        }

        if(vertices_arr[i][0] > maxx)
        {
            maxx = vertices_arr[i][0];
        }
        if(vertices_arr[i][1] > maxy)
        {
            maxy = vertices_arr[i][1];
        }
        if(vertices_arr[i][2] > maxz)
        {
            maxz = vertices_arr[i][2];
        }
    }
    //printf("x: %f %f\ny: %f %f\nz: %f %f\n",minx, maxx, miny, maxy, minz, maxz);
    //SET CENTER POINT
    center[0] = maxx - (maxx - minx) / 2;
    center[1] = maxy - (maxy - miny) / 2;
    center[2] = maxz - (maxz - minz) / 2;

    //SET MIN VALUES
    min[0] = minx;
    min[1] = miny;
    min[2] = minz;

    //SET MAX VALUES
    max[0] = maxx;
    max[1] = maxy;
    max[2] = maxz;

    //<MAX - MIN>
    subtract(max, min, diff);

    //SET E
    for(i = 0; i < 3; i++)
    {
        if(diff[i] > *E)
        {
            *E = diff[i];
        }
    }
}

//HELPER FUNCTION TO READ THE NUMBER OF FACES AND VERTICES OF THE PLY IMAGE
void readVerticesNFaces(FILE *fin,int **faces_arr,float **vertices_arr, unsigned int v_count, unsigned int f_count)
{
    int i;
    char buffer[1000];

    //READ VERTICES INTO AN ARRAY
    for(i = 0; i < v_count; i++)
    {
        fgets(buffer,sizeof(buffer),fin);
        //if(i == 0 || i == v_count -1)
        //    printf("%s\n",buffer);

        sscanf(buffer,"%f %f %f",&vertices_arr[i][0],&vertices_arr[i][1],&vertices_arr[i][2]);
    }
    //READ FACES INTO AN ARRAY
    for(i = 0; i < f_count; i++)
    {
        fgets(buffer,sizeof(buffer),fin);
        //if(i == 0 || i == f_count -1)
            //printf("%s\n",buffer);

        sscanf(buffer,"3 %d %d %d",&faces_arr[i][0],&faces_arr[i][1],&faces_arr[i][2]);
    }
}

//HELPER FUNCTION TO SUBTRACT TWO VECTORS
void subtract(float x[],float y[], float z[])
{
    int i;
    for(i = 0; i < 3; i++)
    {
        z[i] = x[i] - y[i];
    }
}
